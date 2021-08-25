/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KCalendarCore/Todo>

#include "ewsfetchtaskdetailjob.h"

#include "ewsgetitemrequest.h"
#include "ewsitemshape.h"
#include "ewsmailbox.h"
#include "ewsresource_debug.h"

using namespace Akonadi;
using namespace KCalendarCore;

EwsFetchTaskDetailJob::EwsFetchTaskDetailJob(EwsClient &client, QObject *parent, const Akonadi::Collection &collection)
    : EwsFetchItemDetailJob(client, parent, collection)
{
    EwsItemShape shape(EwsShapeIdOnly);
    shape << EwsPropertyField(QStringLiteral("item:Subject"));
    shape << EwsPropertyField(QStringLiteral("item:Body"));
    shape << EwsPropertyField(QStringLiteral("item:MimeContent"));
    shape << EwsPropertyField(QStringLiteral("task:ActualWork"));
    shape << EwsPropertyField(QStringLiteral("task:AssignedTime"));
    shape << EwsPropertyField(QStringLiteral("task:BillingInformation"));
    //shape << EwsPropertyField(QStringLiteral("task:ChangeCount"));
    shape << EwsPropertyField(QStringLiteral("task:Companies"));
    shape << EwsPropertyField(QStringLiteral("task:CompleteDate"));
    shape << EwsPropertyField(QStringLiteral("task:Contacts"));
    shape << EwsPropertyField(QStringLiteral("task:DelegationState"));
    shape << EwsPropertyField(QStringLiteral("task:Delegator"));
    //shape << EwsPropertyField(QStringLiteral("task:DueDate"));
    shape << EwsPropertyField(QStringLiteral("task:IsAssignmentEditable"));
    //shape << EwsPropertyField(QStringLiteral("task:IsComplete"));
    //shape << EwsPropertyField(QStringLiteral("task:IsRecurring"));
    shape << EwsPropertyField(QStringLiteral("task:IsTeamTask"));
    shape << EwsPropertyField(QStringLiteral("task:Mileage"));
    //shape << EwsPropertyField(QStringLiteral("task:Owner"));
    //shape << EwsPropertyField(QStringLiteral("task:PercentComplete"));
    shape << EwsPropertyField(QStringLiteral("task:Recurrence"));
    shape << EwsPropertyField(QStringLiteral("task:StartDate"));
    //shape << EwsPropertyField(QStringLiteral("task:Status"));
    //shape << EwsPropertyField(QStringLiteral("task:StatusDescription"));
    shape << EwsPropertyField(QStringLiteral("task:TotalWork"));
    mRequest->setItemShape(shape);
}

EwsFetchTaskDetailJob::~EwsFetchTaskDetailJob()
{
}

void EwsFetchTaskDetailJob::processItems(const QList<EwsGetItemRequest::Response> &responses)
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
        KCalendarCore::Todo::Ptr todo;
        QString mimeContent = ewsItem[EwsItemFieldMimeContent].toString();
        QString subject = ewsItem[EwsItemFieldSubject].value<QString>();
        QDateTime dtDue;
        QDateTime dtStart;
        //int percentComplete = ewsItem[EwsItemFieldPercentComplete].toInt();
        //QString status = ewsItem[EwsItemFieldStatus].toString();
        if(!ewsItem[EwsItemFieldDueDate].isNull())
        {
            dtDue = ewsItem[EwsItemFieldDueDate].toDateTime();
            qCWarningNC(EWSRES_LOG) << QStringLiteral("Due: %1").arg(dtDue.toString());
            todo->setDtDue(dtDue);
        }
        if(!ewsItem[EwsItemFieldStartDate].isNull())
        {
            dtStart = ewsItem[EwsItemFieldStartDate].toDateTime();
            qCWarningNC(EWSRES_LOG) << QStringLiteral("Start: %1").arg(dtStart.toString());
            todo->setDtStart(dtStart);
        }

        //todo->setPercentComplete(percentComplete);

        if(dtStart.isValid() && dtDue.isValid())
        {
            Duration duration = Duration(dtStart,dtDue,Duration::Type::Days);
            todo->setDuration(duration);
        }
        qCWarningNC(EWSRES_LOG) << QStringLiteral("Subject: %1").arg(subject);
        //todo->setSummary(subject, false);

        item.setPayload<KCalendarCore::Todo::Ptr>(todo);


        ++it;
    }

    emitResult();
}
