/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsitemtypehandler.h"

#include <KMime/Message>

#include "ewscreateitemtypejob.h"
#include "ewsfetchitemtypedetailjob.h"
#include "ewsmodifyitemtypejob.h"
#include "ewsresource_debug.h"

using namespace Akonadi;

EwsFetchItemTypeHandler::EwsFetchItemTypeHandler()
{
}

EwsFetchItemTypeHandler::~EwsFetchItemTypeHandler()
{
}

EwsItemHandler *EwsFetchItemTypeHandler::factory()
{
    return new EwsFetchItemTypeHandler();
}

EwsFetchItemDetailJob *EwsFetchItemTypeHandler::fetchItemDetailJob(EwsClient &client, QObject *parent, const Akonadi::Collection &collection)
{
    return new EwsFetchItemTypeDetailsJob(client, parent, collection);
}

void EwsFetchItemTypeHandler::setSeenFlag(Item &item, bool value)
{
    Q_UNUSED(item)
    Q_UNUSED(value)
}

QString EwsFetchItemTypeHandler::mimeType()
{
    return KMime::Message::mimeType();
}

bool EwsFetchItemTypeHandler::setItemPayload(Akonadi::Item &item, const EwsItem &ewsItem)
{
    QByteArray mimeContent = ewsItem[EwsItemFieldMimeContent].toByteArray();
    if (mimeContent.isEmpty()) {
        qCWarning(EWSRES_LOG) << QStringLiteral("MIME content is empty!");
        return false;
    }

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

EwsModifyItemJob *EwsFetchItemTypeHandler::modifyItemJob(EwsClient &client, const QVector<Akonadi::Item> &items, const QSet<QByteArray> &parts, QObject *parent)
{
    return new EwsModifyItemTypeJob(client, items, parts, parent);
}

EwsCreateItemJob *EwsFetchItemTypeHandler::createItemJob(EwsClient &client,
                                                      const Akonadi::Item &item,
                                                      const Akonadi::Collection &collection,
                                                      EwsTagStore *tagStore,
                                                      EwsResource *parent)
{
    return new EwsCreateItemTypeJob(client, item, collection, tagStore, parent);
}

EWS_DECLARE_ITEM_HANDLER(EwsFetchItemTypeHandler, EwsItemTypeItem)
