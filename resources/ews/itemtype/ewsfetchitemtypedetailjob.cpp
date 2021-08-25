/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsfetchitemtypedetailjob.h"

#include <KMime/Message>

#include "ewsgetitemrequest.h"
#include "ewsitemshape.h"
#include "ewsmailbox.h"
#include "ewsresource_debug.h"

using namespace Akonadi;

EwsFetchItemTypeDetailsJob::EwsFetchItemTypeDetailsJob(EwsClient &client, QObject *parent, const Akonadi::Collection &collection)
    : EwsFetchItemDetailJob(client, parent, collection)
{
    EwsItemShape shape(EwsShapeIdOnly);
    shape << EwsPropertyField(QStringLiteral("item:Body"));
    shape << EwsPropertyField(QStringLiteral("item:MimeContent"));
    shape << EwsPropertyField(QStringLiteral("item:ItemClass"));
    shape << EwsPropertyField(QStringLiteral("item:Subject"));
    shape << EwsPropertyField(QStringLiteral("item:Importance"));
    shape << EwsPropertyField(QStringLiteral("item:Categories"));
    shape << EwsPropertyField(QStringLiteral("item:InReplyTo"));
    shape << EwsPropertyField(QStringLiteral("item:Size"));
    shape << EwsPropertyField(QStringLiteral("item:InternetMessageHeaders"));
    mRequest->setItemShape(shape);
}

EwsFetchItemTypeDetailsJob::~EwsFetchItemTypeDetailsJob()
{
}

void EwsFetchItemTypeDetailsJob::processItems(const QList<EwsGetItemRequest::Response> &responses)
{
    Item::List::iterator it = mChangedItems.begin();

    for (const EwsGetItemRequest::Response &resp : responses) {
        Item &item = *it;

        if (!resp.isSuccess()) {
            qCWarningNC(EWSRES_LOG) << QStringLiteral("Failed to fetch item %1").arg(item.remoteId());
            continue;
        }

        // const EwsItem &ewsItem = resp.item();

        // TODO: Implement

        const EwsItem &ewsItem = resp.item();
        KMime::Message::Ptr msg(new KMime::Message);

        QVariant v = ewsItem[EwsItemFieldSubject];
        if (Q_LIKELY(v.isValid())) {
            msg->subject()->fromUnicodeString(v.toString(), "utf-8");
        }

        v = ewsItem[EwsItemFieldInReplyTo];
        if (v.isValid()) {
            msg->inReplyTo()->from7BitString(v.toString().toLatin1());
        }

        v = ewsItem[EwsItemFieldDateTimeReceived];
        if (v.isValid()) {
            msg->date()->setDateTime(v.toDateTime());
        }

        QByteArray mimeContent = ewsItem[EwsItemFieldMimeContent].toByteArray();
        mimeContent.replace("\r\n", "\n");
        msg->setContent(mimeContent);
        QVariantList Body = ewsItem[EwsItemFieldBody].toList();
        if(!Body.isEmpty())
        {
            if (Body[0].toString().isEmpty()) {
                msg->setBody("\n");
            }
        }
        msg->assemble();
        item.setPayload(KMime::Message::Ptr(msg));




        ++it;
    }

    emitResult();
}
