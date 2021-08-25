/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsabchpersonhandler.h"

#include <KMime/Message>

#include "ewscreateabchpersonjob.h"
#include "ewsfetchabchpersondetailjob.h"
#include "ewsmodifyabchpersonjob.h"
#include "ewsresource_debug.h"

using namespace Akonadi;

EwsAbchPersonHandler::EwsAbchPersonHandler()
{
}

EwsAbchPersonHandler::~EwsAbchPersonHandler()
{
}

EwsItemHandler *EwsAbchPersonHandler::factory()
{
    return new EwsAbchPersonHandler();
}

EwsFetchItemDetailJob *EwsAbchPersonHandler::fetchItemDetailJob(EwsClient &client, QObject *parent, const Akonadi::Collection &collection)
{
    return new EwsFetchAbchContactDetailsJob(client, parent, collection);
}

void EwsAbchPersonHandler::setSeenFlag(Item &item, bool value)
{
    Q_UNUSED(item)
    Q_UNUSED(value)
}

QString EwsAbchPersonHandler::mimeType()
{
    return KMime::Message::mimeType();
}

bool EwsAbchPersonHandler::setItemPayload(Akonadi::Item &item, const EwsItem &ewsItem)
{
    KMime::Message::Ptr msg(new KMime::Message);
    QByteArray mimeContent = ewsItem[EwsItemFieldMimeContent].toByteArray();

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
    item.setPayload<KMime::Message::Ptr>(msg);

    return true;
}

EwsModifyItemJob *EwsAbchPersonHandler::modifyItemJob(EwsClient &client, const QVector<Akonadi::Item> &items, const QSet<QByteArray> &parts, QObject *parent)
{
    return new EwsModifyAbchPersonJob(client, items, parts, parent);
}

EwsCreateItemJob *EwsAbchPersonHandler::createItemJob(EwsClient &client,
                                                      const Akonadi::Item &item,
                                                      const Akonadi::Collection &collection,
                                                      EwsTagStore *tagStore,
                                                      EwsResource *parent)
{
    return new EwsCreateAbchPersonJob(client, item, collection, tagStore, parent);
}

EWS_DECLARE_ITEM_HANDLER(EwsAbchPersonHandler, EwsItemTypeAbchPerson)
