/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewscontacthandler.h"

#include <AkonadiCore/Item>
#include <KContacts/Addressee>
#include <KContacts/ContactGroup>
#include <KContacts/VCardConverter>

#include "ewscreatecontactjob.h"
#include "ewsfetchcontactdetailjob.h"
#include "ewsmodifycontactjob.h"
#include "ewsresource_debug.h"

using namespace Akonadi;

EwsContactHandler::EwsContactHandler()
{
}

EwsContactHandler::~EwsContactHandler()
{
}

EwsItemHandler *EwsContactHandler::factory()
{
    return new EwsContactHandler();
}

EwsFetchItemDetailJob *EwsContactHandler::fetchItemDetailJob(EwsClient &client, QObject *parent, const Akonadi::Collection &collection)
{
    return new EwsFetchContactDetailJob(client, parent, collection);
}

void EwsContactHandler::setSeenFlag(Item &item, bool value)
{
    Q_UNUSED(item)
    Q_UNUSED(value)
}

QString EwsContactHandler::mimeType()
{
    return KContacts::Addressee::mimeType();
}

bool EwsContactHandler::setItemPayload(Akonadi::Item &item, const EwsItem &ewsItem)
{
    QByteArray mimeContent = ewsItem[EwsItemFieldMimeContent].toByteArray();
    KContacts::VCardConverter *Convert = new KContacts::VCardConverter();
    if (mimeContent.isEmpty()) {
        qCWarning(EWSRES_LOG) << QStringLiteral("MIME content is empty!");
        return false;
    }
    item.setPayload<KContacts::Addressee>(Convert->parseVCard(mimeContent));


    return true;
}

EwsModifyItemJob *EwsContactHandler::modifyItemJob(EwsClient &client, const QVector<Akonadi::Item> &items, const QSet<QByteArray> &parts, QObject *parent)
{
    return new EwsModifyContactJob(client, items, parts, parent);
}

EwsCreateItemJob *EwsContactHandler::createItemJob(EwsClient &client,
                                                   const Akonadi::Item &item,
                                                   const Akonadi::Collection &collection,
                                                   EwsTagStore *tagStore,
                                                   EwsResource *parent)
{
    return new EwsCreateContactJob(client, item, collection, tagStore, parent);
}

EWS_DECLARE_ITEM_HANDLER(EwsContactHandler, EwsItemTypeContact)
