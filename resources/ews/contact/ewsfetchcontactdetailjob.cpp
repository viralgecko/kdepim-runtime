/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsfetchcontactdetailjob.h"
#include "ewsgetitemrequest.h"
#include "ewsitemshape.h"
#include "ewsmailbox.h"
#include "ewsresource_debug.h"
#include <KContacts/Addressee>
#include <KContacts/AddresseeList>
#include <KContacts/VCardConverter>

using namespace Akonadi;
using namespace KContacts;

EwsFetchContactDetailJob::EwsFetchContactDetailJob(EwsClient &client, QObject *parent, const Akonadi::Collection &collection)
    : EwsFetchItemDetailJob(client, parent, collection)
{
    EwsItemShape shape(EwsShapeIdOnly);
    shape << EwsPropertyField(QStringLiteral("item:Body"));
    shape << EwsPropertyField(QStringLiteral("item:MimeContent"));
    shape << EwsPropertyField(QStringLiteral("contacts:SpouseName"));
    shape << EwsPropertyField(QStringLiteral("contacts:WeddingAnniversary"));
    mRequest->setItemShape(shape);
}

EwsFetchContactDetailJob::~EwsFetchContactDetailJob()
{
}

void EwsFetchContactDetailJob::processItems(const EwsGetItemRequest::Response::List &responses)
{
    Item::List::iterator it = mChangedItems.begin();
    VCardConverter *Convert = new VCardConverter();
    Addressee contact;

    for (const EwsGetItemRequest::Response &resp : responses) {
        Item &item = *it;

        if (!resp.isSuccess()) {
            qCWarningNC(EWSRES_LOG) << QStringLiteral("Failed to fetch item %1").arg(item.remoteId());
            continue;
        }

        // const EwsItem &ewsItem = resp.item();

        // TODO: Implement


        const EwsItem &ewsItem = resp.item();
        QString mimeContent = ewsItem[EwsItemFieldMimeContent].toString();
        contact = Convert->parseVCard(mimeContent.toUtf8());
        contact.setSpousesName(ewsItem[EwsItemFieldSpouseName].toString());
        contact.setAnniversary(ewsItem[EwsItemFieldWeddingAnniversary].toDate());
        QStringList customs = contact.customs();
        KContacts::Impp::List imppList;
        QString skype = QStringLiteral("skype:");
        QString imAddress = contact.custom(QStringLiteral("MS"),QStringLiteral("IMADDRESS"));
        if(!imAddress.isEmpty())
        {
            contact.insertImpp(KContacts::Impp(QUrl(skype.append(imAddress))));
            contact.insertCustom(QStringLiteral("SKYPE"),QStringLiteral("USERNAME"),imAddress);
        }
        item.setPayload<Addressee>(contact);

        ++it;
    }

    emitResult();
}
