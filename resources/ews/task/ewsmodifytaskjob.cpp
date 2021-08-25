/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KCalendarCore/Todo>
#include <klocalizedstring.h>

#include "ewsmodifytaskjob.h"
#include "ewstaskhandler.h"
#include "ewsupdateitemrequest.h"

#include "ewsresource_debug.h"
using namespace Akonadi;

EwsModifyTaskJob::EwsModifyTaskJob(EwsClient &client, const Akonadi::Item::List &items, const QSet<QByteArray> &parts, QObject *parent)
    : EwsModifyItemJob(client, items, parts, parent)
{
}

EwsModifyTaskJob::~EwsModifyTaskJob()
{
}

void EwsModifyTaskJob::start()
{
    bool doSubmit = false;
    auto req = new EwsUpdateItemRequest(mClient, this);
    EwsId itemId;

    for (const Item &item : qAsConst(mItems)) {
        itemId = EwsId(item.remoteId(), item.remoteRevision());

        EwsUpdateItemRequest::ItemChange ic(itemId, EwsItemTypeTask);
        QHash<EwsPropertyField, QVariant> propertyHash = EwsTaskHandler::writeProperties(item);

        for (auto it = propertyHash.cbegin(), end = propertyHash.cend(); it != end; ++it) {
            EwsUpdateItemRequest::Update *upd;
            if (it.value().isNull()) {
                upd = new EwsUpdateItemRequest::DeleteUpdate(it.key());
            } else {
                upd = new EwsUpdateItemRequest::SetUpdate(it.key(), it.value());
            }
            ic.addUpdate(upd);
        }

        req->addItemChange(ic);
        doSubmit = true;
    }

    if (doSubmit) {
        connect(req, &KJob::result, this, &EwsModifyTaskJob::updateItemFinished);
        req->start();
    } else {
        delete req;
        emitResult();
    }
}

void EwsModifyTaskJob::updateItemFinished(KJob *job)
{
    if (job->error()) {
        setErrorText(job->errorString());
        emitResult();
        return;
    }

    auto req = qobject_cast<EwsUpdateItemRequest *>(job);
    if (!req) {
        setErrorText(QStringLiteral("Invalid EwsUpdateItemRequest job object"));
        emitResult();
        return;
    }

    Q_ASSERT(req->responses().size() == mItems.size());

    Item::List::iterator it = mItems.begin();
    Q_FOREACH (const EwsUpdateItemRequest::Response &resp, req->responses()) {
        if (!resp.isSuccess()) {
            setErrorText(QStringLiteral("Item update failed: ") + resp.responseMessage());
            emitResult();
            return;
        }

        it->setRemoteRevision(resp.itemId().changeKey());
    }

    emitResult();
}

