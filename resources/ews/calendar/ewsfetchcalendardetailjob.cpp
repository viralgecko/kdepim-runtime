/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsfetchcalendardetailjob.h"

#include <QTimeZone>

#include <KCalendarCore/Event>
#include <KCalendarCore/ICalFormat>
#include <KCalendarCore/MemoryCalendar>

#include "ewsgetitemrequest.h"
#include "ewsitemshape.h"
#include "ewsmailbox.h"
#include "ewsoccurrence.h"
#include "ewsresource_debug.h"
#include "ewsfolderclassattribute.h"

using namespace Akonadi;

EwsFetchCalendarDetailJob::EwsFetchCalendarDetailJob(EwsClient &client, QObject *parent, const Collection &collection)
    : EwsFetchItemDetailJob(client, parent, collection)
{
    EwsItemShape shape(EwsShapeIdOnly);
    /*    shape << EwsPropertyField(QStringLiteral("calendar:UID"));
        shape << EwsPropertyField(QStringLiteral("item:Subject"));
        shape << EwsPropertyField(QStringLiteral("item:Body"));
        shape << EwsPropertyField(QStringLiteral("calendar:Organizer"));
        shape << EwsPropertyField(QStringLiteral("calendar:RequiredAttendees"));
        shape << EwsPropertyField(QStringLiteral("calendar:OptionalAttendees"));
        shape << EwsPropertyField(QStringLiteral("calendar:Resources"));
        shape << EwsPropertyField(QStringLiteral("calendar:Start"));
        shape << EwsPropertyField(QStringLiteral("calendar:End"));
        shape << EwsPropertyField(QStringLiteral("calendar:IsAllDayEvent"));
        shape << EwsPropertyField(QStringLiteral("calendar:LegacyFreeBusyStatus"));
        shape << EwsPropertyField(QStringLiteral("calendar:AppointmentSequenceNumber"));
        shape << EwsPropertyField(QStringLiteral("calendar:IsRecurring"));
        shape << EwsPropertyField(QStringLiteral("calendar:Recurrence"));
        shape << EwsPropertyField(QStringLiteral("calendar:FirstOccurrence"));
        shape << EwsPropertyField(QStringLiteral("calendar:LastOccurrence"));
        shape << EwsPropertyField(QStringLiteral("calendar:ModifiedOccurrences"));
        shape << EwsPropertyField(QStringLiteral("calendar:DeletedOccurrences"));
        shape << EwsPropertyField(QStringLiteral("calendar:StartTimeZone"));
        shape << EwsPropertyField(QStringLiteral("calendar:EndTimeZone"));
        shape << EwsPropertyField(QStringLiteral("calendar:MyResponseType"));
        shape << EwsPropertyField(QStringLiteral("item:HasAttachments"));
        shape << EwsPropertyField(QStringLiteral("item:Attachments"));*/

    //    shape << EwsPropertyField(QStringLiteral("item:Attachments"));
    shape << EwsPropertyField(QStringLiteral("calendar:ModifiedOccurrences"));
    shape << EwsPropertyField(QStringLiteral("calendar:DeletedOccurrences"));
    shape << EwsPropertyField(QStringLiteral("calendar:RequiredAttendees"));
    shape << EwsPropertyField(QStringLiteral("calendar:OptionalAttendees"));
    shape << EwsPropertyField(QStringLiteral("calendar:LegacyFreeBusyStatus"));
    shape << EwsPropertyField(QStringLiteral("item:Body"));
    //    shape << EwsPropertyField(QStringLiteral("item:Culture"));
    shape << EwsPropertyField(QStringLiteral("item:MimeContent"));
    shape << EwsPropertyField(QStringLiteral("item:Subject"));
    //    shape << EwsPropertyField(QStringLiteral("calendar:TimeZone"));
    mRequest->setItemShape(shape);
}

EwsFetchCalendarDetailJob::~EwsFetchCalendarDetailJob()
{
}

void EwsFetchCalendarDetailJob::processItems(const EwsGetItemRequest::Response::List &responses)
{
    Item::List::iterator it = mChangedItems.begin();
    KCalendarCore::ICalFormat format;

    EwsId::List addItems;

    Q_FOREACH (const EwsGetItemRequest::Response &resp, responses) {
        Item &item = *it;

        if (!resp.isSuccess()) {
            qCWarningNC(EWSRES_LOG) << QStringLiteral("Failed to fetch item %1").arg(item.remoteId());
            continue;
        }

        const EwsItem &ewsItem = resp.item();
        QString mimeContent = ewsItem[EwsItemFieldMimeContent].toString();
        KCalendarCore::Calendar::Ptr memcal(new KCalendarCore::MemoryCalendar(QTimeZone::utc()));
        format.fromString(memcal, mimeContent);
        qCDebugNC(EWSRES_LOG) << QStringLiteral("Found %1 events").arg(memcal->events().count());
        KCalendarCore::Incidence::Ptr incidence;
        if (memcal->events().count() > 1) {
            Q_FOREACH (const KCalendarCore::Event::Ptr &event, memcal->events()) {
                qCDebugNC(EWSRES_LOG) << QString::number(event->recurrence()->recurrenceType(), 16) << event->recurrenceId() << event->recurrenceId().isValid();
                if (!event->recurrenceId().isValid()) {
                    incidence = event;
                }
            }
            const EwsOccurrence::List excList = ewsItem[EwsItemFieldModifiedOccurrences].value<EwsOccurrence::List>();
            for (const EwsOccurrence &exc : excList) {
                addItems.append(exc.itemId());
            }
        } else if (memcal->events().count() == 1) {
            incidence = memcal->events()[0];
        }
        // KCalendarCore::Incidence::Ptr incidence(format.fromString(mimeContent));

        if (incidence) {
            QDateTime dt(incidence->dtStart());
            if (dt.isValid()) {
                incidence->setDtStart(dt);
            }
            if (incidence->type() == KCalendarCore::Incidence::TypeEvent) {
                auto *event = reinterpret_cast<KCalendarCore::Event *>(incidence.data());
                dt = event->dtEnd();
                if (dt.isValid()) {
                    event->setDtEnd(dt);
                }
            }
            dt = incidence->recurrenceId();
            if (dt.isValid()) {
                incidence->setRecurrenceId(dt);
            }

            if(mCollection.hasAttribute("FolderClass"))
            {
                QByteArray folderClass = mCollection.attribute(QByteArray("FolderClass"))->serialized();
                if(QString::fromUtf8(folderClass)==QString::fromUtf8("IPF.Appointment.Birthday"))
                {
                    QStringList categories = incidence->categories();
                    categories.append(QString::fromUtf8("Birthday"));
                    incidence->setCategories(categories);
                    incidence->setCustomProperty("KABC", "BIRTHDAY", QStringLiteral("YES"));
                }
            }
            int freeBusy = ewsItem[EwsItemFieldLegacyFreeBusyStatus].toInt();
            switch(freeBusy)
            {
                case EwsLegacyFreeBusyStatus::EwsLfbNoData:
                    incidence->setStatus(KCalendarCore::Incidence::Status::StatusNone);
                    break;
                case EwsLegacyFreeBusyStatus::EwsLfbStatusBusy:
                    incidence->setStatus(KCalendarCore::Incidence::Status::StatusConfirmed);
                    break;
                case EwsLegacyFreeBusyStatus::EwsLfbStatusFree:
                    incidence->setStatus(KCalendarCore::Incidence::Status::StatusNone);
                    break;
                case EwsLegacyFreeBusyStatus::EwsLfbStatusTentative:
                    incidence->setStatus(KCalendarCore::Incidence::Status::StatusTentative);
                    break;
                case EwsLegacyFreeBusyStatus::EwsLfbOutOfOffice:
                    incidence->setStatus(KCalendarCore::Incidence::Status::StatusX);
                    incidence->setCustomStatus(QStringLiteral("Out of Office"));
                    break;
                case EwsLegacyFreeBusyStatus::EwsLfbStatusWorkingElsewhere:
                    incidence->setStatus(KCalendarCore::Incidence::Status::StatusX);
                    incidence->setCustomStatus(QStringLiteral("Working Elsewhere"));
                    break;
                default:
                    incidence->setStatus(KCalendarCore::Incidence::Status::StatusConfirmed);
                    break;
            }

            incidence->clearAttendees();
            KCalendarCore::Attendee::List requiredList = ewsItem[EwsItemFieldRequiredAttendees].value<KCalendarCore::Attendee::List>();
            KCalendarCore::Attendee::List optionalList = ewsItem[EwsItemFieldOptionalAttendees].value<KCalendarCore::Attendee::List>();
            KCalendarCore::Attendee::List attendees;
            for( KCalendarCore::Attendee &attendee : requiredList)
            {
                if(attendee.role() != KCalendarCore::Attendee::Role::Chair)
                {
                    attendee.setRole(KCalendarCore::Attendee::Role::ReqParticipant);
                }
                attendees.append(attendee);
            }
            for( KCalendarCore::Attendee &attendee : optionalList)
            {
                if(attendee.role() != KCalendarCore::Attendee::Role::Chair)
                {
                    attendee.setRole(KCalendarCore::Attendee::Role::OptParticipant);
                }
                attendees.append(attendee);
            }
            incidence->setAttendees(attendees);

            item.setPayload<KCalendarCore::Incidence::Ptr>(incidence);
        }

        ++it;
    }

    if (addItems.isEmpty()) {
        emitResult();
    } else {
        auto req = new EwsGetItemRequest(mClient, this);
        EwsItemShape shape(EwsShapeIdOnly);
        //        shape << EwsPropertyField(QStringLiteral("item:Attachments"));
        shape << EwsPropertyField(QStringLiteral("item:Body"));
        shape << EwsPropertyField(QStringLiteral("item:MimeContent"));
        shape << EwsPropertyField(QStringLiteral("calendar:RequiredAttendees"));
        shape << EwsPropertyField(QStringLiteral("calendar:OptionalAttendees"));
        shape << EwsPropertyField(QStringLiteral("calendar:LegacyFreeBusyStatus"));
        //        shape << EwsPropertyField(QStringLiteral("calendar:TimeZone"));
        //        shape << EwsPropertyField(QStringLiteral("item:Culture"));
        req->setItemShape(shape);

        req->setItemIds(addItems);
        connect(req, &KJob::result, this, &EwsFetchCalendarDetailJob::exceptionItemsFetched);
        req->start();
    }
}

void EwsFetchCalendarDetailJob::exceptionItemsFetched(KJob *job)
{
    if (job->error()) {
        setError(job->error());
        setErrorText(job->errorText());
        emitResult();
        return;
    }

    auto req = qobject_cast<EwsGetItemRequest *>(job);

    if (!req) {
        setError(1);
        setErrorText(QStringLiteral("Job is not an instance of EwsGetItemRequest"));
        emitResult();
        return;
    }

    KCalendarCore::ICalFormat format;
    Q_FOREACH (const EwsGetItemRequest::Response &resp, req->responses()) {
        if (!resp.isSuccess()) {
            qCWarningNC(EWSRES_LOG) << QStringLiteral("Failed to fetch item.");
            continue;
        }
        const EwsItem &ewsItem = resp.item();

        Item item(KCalendarCore::Event::eventMimeType());
        item.setParentCollection(mCollection);
        EwsId id = ewsItem[EwsItemFieldItemId].value<EwsId>();
        item.setRemoteId(id.id());
        item.setRemoteRevision(id.changeKey());

        QString mimeContent = ewsItem[EwsItemFieldMimeContent].toString();
        KCalendarCore::Calendar::Ptr memcal(new KCalendarCore::MemoryCalendar(QTimeZone::utc()));
        format.fromString(memcal, mimeContent);
        KCalendarCore::Incidence::Ptr incidence(memcal->events().last());
        incidence->clearRecurrence();

        QDateTime dt(incidence->dtStart());
        if (dt.isValid()) {
            incidence->setDtStart(dt);
        }
        if (incidence->type() == KCalendarCore::Incidence::TypeEvent) {
            auto *event = reinterpret_cast<KCalendarCore::Event *>(incidence.data());
            dt = event->dtEnd();
            if (dt.isValid()) {
                event->setDtEnd(dt);
            }
        }
        dt = incidence->recurrenceId();
        if (dt.isValid()) {
            incidence->setRecurrenceId(dt);
        }
        int freeBusy = ewsItem[EwsItemFieldLegacyFreeBusyStatus].toInt();
        switch(freeBusy)
        {
            case EwsLegacyFreeBusyStatus::EwsLfbNoData:
                incidence->setStatus(KCalendarCore::Incidence::Status::StatusNone);
                break;
            case EwsLegacyFreeBusyStatus::EwsLfbStatusBusy:
                incidence->setStatus(KCalendarCore::Incidence::Status::StatusConfirmed);
                break;
            case EwsLegacyFreeBusyStatus::EwsLfbStatusFree:
                incidence->setStatus(KCalendarCore::Incidence::Status::StatusNone);
                break;
            case EwsLegacyFreeBusyStatus::EwsLfbStatusTentative:
                incidence->setStatus(KCalendarCore::Incidence::Status::StatusTentative);
                break;
            case EwsLegacyFreeBusyStatus::EwsLfbOutOfOffice:
                    incidence->setStatus(KCalendarCore::Incidence::Status::StatusX);
                    incidence->setCustomStatus(QStringLiteral("Out of Office"));
                    break;
            case EwsLegacyFreeBusyStatus::EwsLfbStatusWorkingElsewhere:
                incidence->setStatus(KCalendarCore::Incidence::Status::StatusX);
                incidence->setCustomStatus(QStringLiteral("Working Elsewhere"));
                break;
            default:
                incidence->setStatus(KCalendarCore::Incidence::Status::StatusConfirmed);
                break;
        }
        incidence->clearAttendees();
        KCalendarCore::Attendee::List requiredList = ewsItem[EwsItemFieldRequiredAttendees].value<KCalendarCore::Attendee::List>();
        KCalendarCore::Attendee::List optionalList = ewsItem[EwsItemFieldOptionalAttendees].value<KCalendarCore::Attendee::List>();
        KCalendarCore::Attendee::List attendees;
        for( KCalendarCore::Attendee &attendee : requiredList)
        {
            if(attendee.role() != KCalendarCore::Attendee::Role::Chair)
            {
                attendee.setRole(KCalendarCore::Attendee::Role::ReqParticipant);
            }
            attendees.append(attendee);
        }
        for( KCalendarCore::Attendee &attendee : optionalList)
        {
            if(attendee.role() != KCalendarCore::Attendee::Role::Chair)
            {
                attendee.setRole(KCalendarCore::Attendee::Role::OptParticipant);
            }
            attendees.append(attendee);
        }
        incidence->setAttendees(attendees);

        item.setPayload<KCalendarCore::Incidence::Ptr>(incidence);

        mChangedItems.append(item);
    }

    emitResult();
}
