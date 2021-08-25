/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include <KLocalizedString>

#include "ewscreatetaskjob.h"
#include <KCalendarCore/Todo>
#include <AkonadiCore/AgentManager>
#include <AkonadiCore/Collection>
#include <AkonadiCore/Item>
#include "ewsmoveitemrequest.h"
#include "ewscreateitemrequest.h"
#include "ewspropertyfield.h"
#include "ewstaskhandler.h"
#include "ewsresource_debug.h"

EwsCreateTaskJob::EwsCreateTaskJob(EwsClient &client,
                                   const Akonadi::Item &item,
                                   const Akonadi::Collection &collection,
                                   EwsTagStore *tagStore,
                                   EwsResource *parent)
    : EwsCreateItemJob(client, item, collection, tagStore, parent)
{
}

EwsCreateTaskJob::~EwsCreateTaskJob()
{
}

void EwsCreateTaskJob::doStart()
{
    if (!mItem.hasPayload<KCalendarCore::Todo::Ptr>()) {
        setErrorMsg(QStringLiteral("Expected MIME message payload"));
        emitResult();
    }
    auto req = new EwsCreateItemRequest(mClient, this);
    KCalendarCore::Todo::Ptr Task = mItem.payload<KCalendarCore::Todo::Ptr>();
    EwsItem item;
    item.setType(EwsItemTypeTask);
    item.setField(EwsItemFieldSubject,Task->summary());
    if(Task->dtDue().isValid())
    {
        item.setField(EwsItemFieldDueDate,Task->dtDue());
    }
    if(Task->dtStart().isValid())
    {
        item.setField(EwsItemFieldStartDate,Task->dtStart());
    }
    if(Task->completed().isValid())
    {
        item.setField(EwsItemFieldCompleteDate,Task->completed());
    }
    item.setField(EwsItemFieldPercentComplete,Task->percentComplete());
    item.setField(EwsItemFieldOwner,Task->organizer().fullName());
    item.setField(EwsItemFieldDelegationState, Task->customStatus());
    item.setField(EwsItemFieldStatus, Task->status());
    item.setField(EwsItemFieldContacts, Task->contacts());
    req->setSavedFolderId(EwsId(mCollection.remoteId(), mCollection.remoteRevision()));
    populateCommonProperties(item);

    req->setItems(EwsItem::List() << item);
    req->setMessageDisposition(EwsDispSaveOnly);
    connect(req,
            &EwsCreateItemRequest::finished,
            this,
            &EwsCreateTaskJob::taskCreateFinished);
    addSubjob(req);
    req->start();
}

void EwsCreateTaskJob::taskCreateFinished(KJob *job)
{
        auto req = qobject_cast<EwsCreateItemRequest *>(job);
    if (job->error()) {
        setErrorMsg(job->errorString());
        emitResult();
        return;
    }

    if (!req) {
        setErrorMsg(QStringLiteral("Invalid EwsCreateItemRequest job object"));
        emitResult();
        return;
    }

    if (req->responses().count() != 1) {
        setErrorMsg(QStringLiteral("Invalid number of responses received from server."));
        emitResult();
        return;
    }

    EwsCreateItemRequest::Response resp = req->responses().first();
    if (resp.isSuccess()) {
        EwsId id = resp.itemId();
        mItem.setRemoteId(id.id());
        mItem.setRemoteRevision(id.changeKey());
        mItem.setParentCollection(mCollection);
    } else {
        qCWarning(EWSRES_LOG) << QStringLiteral("Error Message: %1").arg(resp.responseMessage());
        setErrorMsg(i18n("Failed to create Task item"));
    }

    emitResult();
}

bool EwsCreateTaskJob::setSend(bool send)
{
    Q_UNUSED(send)

    qCWarning(EWSRES_LOG) << QStringLiteral("Sending task items is not supported!");
    return false;
}
