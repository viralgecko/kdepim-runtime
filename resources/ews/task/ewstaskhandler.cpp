/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewstaskhandler.h"

#include <KCalendarCore/Todo>

#include "ewscreatetaskjob.h"
#include "ewsfetchtaskdetailjob.h"
#include "ewsmodifytaskjob.h"

using namespace Akonadi;

EwsTaskHandler::EwsTaskHandler()
{
}

EwsTaskHandler::~EwsTaskHandler()
{
}

EwsItemHandler *EwsTaskHandler::factory()
{
    return new EwsTaskHandler();
}

EwsFetchItemDetailJob *EwsTaskHandler::fetchItemDetailJob(EwsClient &client, QObject *parent, const Akonadi::Collection &collection)
{
    return new EwsFetchTaskDetailJob(client, parent, collection);
}

void EwsTaskHandler::setSeenFlag(Item &item, bool value)
{
    Q_UNUSED(item)
    Q_UNUSED(value)
}

QString EwsTaskHandler::mimeType()
{
    return KCalendarCore::Todo::todoMimeType();
}

bool EwsTaskHandler::setItemPayload(Akonadi::Item &item, const EwsItem &ewsItem)
{
    KCalendarCore::Todo::Ptr todo = KCalendarCore::Todo::Ptr(new KCalendarCore::Todo());
    QString mimeContent = ewsItem[EwsItemFieldMimeContent].toString();
    QDateTime dtDue = ewsItem[EwsItemFieldDueDate].toDateTime();
    QDateTime dtStart = ewsItem[EwsItemFieldStartDate].toDateTime();
    QDateTime dtCompleted = ewsItem[EwsItemFieldCompleteDate].toDateTime();
    if(!dtDue.isNull())
    {
        todo->setDtDue(dtDue);
    }
    if(!dtStart.isNull())
    {
        todo->setDtStart(dtStart);
    }
    if(dtStart.isValid() && dtDue.isValid())
    {
        KCalendarCore::Duration duration = KCalendarCore::Duration(dtStart,dtDue,KCalendarCore::Duration::Type::Days);
        todo->setDuration(duration);
    }
    if(!dtCompleted.isNull())
    {
        todo->setCompleted(dtCompleted);
    }
    else
    {
        todo->setCompleted(ewsItem[EwsItemFieldIsComplete].toBool());
    }
    todo->setSummary(ewsItem[EwsItemFieldSubject].toString());
    todo->setPercentComplete(ewsItem[EwsItemFieldPercentComplete].toUInt());
    todo->setOrganizer(ewsItem[EwsItemFieldOwner].toString());
    item.setPayload<KCalendarCore::Todo::Ptr>(todo);
    return true;
}

EwsModifyItemJob *EwsTaskHandler::modifyItemJob(EwsClient &client, const QVector<Akonadi::Item> &items, const QSet<QByteArray> &parts, QObject *parent)
{
    return new EwsModifyTaskJob(client, items, parts, parent);
}

EwsCreateItemJob *
EwsTaskHandler::createItemJob(EwsClient &client, const Akonadi::Item &item, const Akonadi::Collection &collection, EwsTagStore *tagStore, EwsResource *parent)
{
    return new EwsCreateTaskJob(client, item, collection, tagStore, parent);
}

QHash<EwsPropertyField, QVariant> EwsTaskHandler::writeProperties(const Akonadi::Item &item)
{
    KCalendarCore::Todo::Ptr task = item.payload<KCalendarCore::Todo::Ptr>();
    QHash<EwsPropertyField, QVariant> propertyHash;

    if(task->completed().isValid())
    {
        propertyHash.insert(EwsPropertyField(QStringLiteral("task:CompleteDate")),task->completed().toString(Qt::ISODate));
    }
    else
    {
        propertyHash.insert(EwsPropertyField(QStringLiteral("task:CompleteDate")),QVariant());
    }
    if(task->dtStart().isValid())
    {
        propertyHash.insert(EwsPropertyField(QStringLiteral("task:StartDate")),task->dtStart().toString(Qt::ISODate));
    }
    else
    {
        propertyHash.insert(EwsPropertyField(QStringLiteral("task:StartDate")),QVariant());
    }
    if(task->dtDue().isValid())
    {
        propertyHash.insert(EwsPropertyField(QStringLiteral("task:DueDate")),task->dtDue().toString(Qt::ISODate));
    }
    else
    {
        propertyHash.insert(EwsPropertyField(QStringLiteral("task:DueDate")),QVariant());
    }
    propertyHash.insert(EwsPropertyField(QStringLiteral("item:Subject")),task->summary());
    propertyHash.insert(EwsPropertyField(QStringLiteral("task:PercentComplete")),QString::number(task->percentComplete()));
    return propertyHash;
}

EWS_DECLARE_ITEM_HANDLER(EwsTaskHandler, EwsItemTypeTask)
