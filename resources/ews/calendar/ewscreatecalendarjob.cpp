/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KLocalizedString>

#include "ewscreatecalendarjob.h"
#include <KCalendarCore/Event>
#include <AkonadiCore/AgentManager>
#include <AkonadiCore/Collection>
#include <AkonadiCore/Item>
#include "ewsmoveitemrequest.h"
#include "ewscreateitemrequest.h"
#include "ewspropertyfield.h"
#include "ewscalendarhandler.h"
#include "ewsmailbox.h"

#include "ewsresource_debug.h"

EwsCreateCalendarJob::EwsCreateCalendarJob(EwsClient &client,
                                           const Akonadi::Item &item,
                                           const Akonadi::Collection &collection,
                                           EwsTagStore *tagStore,
                                           EwsResource *parent)
    : EwsCreateItemJob(client, item, collection, tagStore, parent)
{
}

EwsCreateCalendarJob::~EwsCreateCalendarJob()
{
}

void EwsCreateCalendarJob::doStart()
{
    if (!mItem.hasPayload<KCalendarCore::Event::Ptr>()) {
        setErrorMsg(QStringLiteral("Expected MIME message payload"));
        emitResult();
    }
    auto req = new EwsCreateItemRequest(mClient, this);
    KCalendarCore::Event::Ptr event = mItem.payload<KCalendarCore::Event::Ptr>();
    if(event->recurrenceId().isValid())
    {
        qCWarning(EWSRES_LOG) << QStringLiteral("Creating recurring calendar items is not supported!");
        emitResult();
        return;
    }
    EwsItem item;
    bool meetingDisposition = false;
    item.setType(EwsItemTypeCalendarItem);
    item.setField(EwsItemFieldStart,event->dtStart());
    item.setField(EwsItemFieldEnd,event->dtEnd());
    item.setField(EwsItemFieldIsAllDayEvent, event->allDay());
    //item.setField(EwsItemFieldCalendarItemType, EwsCalendarItemType::EwsCalendarItemSingle);
    item.setField(EwsItemFieldSubject, event->summary());
    KCalendarCore::Attendee::List attendees = event->attendees();
    KCalendarCore::Attendee::List required;
    KCalendarCore::Attendee::List optional;
    for( KCalendarCore::Attendee &attendee : attendees)
    {
        int attendeeType = attendee.role();
        if(attendee.RSVP())
            meetingDisposition = true;
        switch (attendeeType)
        {
            case KCalendarCore::Attendee::Role::ReqParticipant:
            case KCalendarCore::Attendee::Role::Chair:
                required.append(attendee);
                break;
            case KCalendarCore::Attendee::Role::NonParticipant:
            case KCalendarCore::Attendee::Role::OptParticipant:
                optional.append(attendee);
                break;
        }
    }
    if(!required.isEmpty())
        item.setField(EwsItemFieldRequiredAttendees,QVariant::fromValue<KCalendarCore::Attendee::List>(required));
    if(!optional.isEmpty())
        item.setField(EwsItemFieldRequiredAttendees,QVariant::fromValue<KCalendarCore::Attendee::List>(optional));

    populateCommonProperties(item);
    req->setSavedFolderId(EwsId(mCollection.remoteId(), mCollection.remoteRevision()));
    if(meetingDisposition)
        req->setMeetingDisposition(EwsMeetingDispSendOnlyToAll);
    else
        req->setMeetingDisposition(EwsMeetingDispSendToNone);

    req->setItems(EwsItem::List() << item);
    req->setMessageDisposition(EwsDispSaveOnly);
    connect(req,
            &EwsCreateItemRequest::finished,
            this,
            &EwsCreateCalendarJob::calendarCreateFinished);
    addSubjob(req);
    req->start();
}

void EwsCreateCalendarJob::calendarCreateFinished(KJob *job)
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
        setErrorMsg(i18n("Failed to create Calendar item"));
    }

    emitResult();
}

bool EwsCreateCalendarJob::setSend(bool send)
{
    Q_UNUSED(send)

    qCWarning(EWSRES_LOG) << QStringLiteral("Sending calendar items is not supported!");
    return false;
}
