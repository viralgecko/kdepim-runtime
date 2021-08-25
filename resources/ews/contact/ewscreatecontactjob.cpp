/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KLocalizedString>

#include <KContacts/Addressee>
#include <KContacts/VCardConverter>
#include <AkonadiCore/AgentManager>
#include <AkonadiCore/Collection>
#include <AkonadiCore/Item>
#include "ewsmoveitemrequest.h"
#include "ewscreateitemrequest.h"
#include "ewscreatecontactjob.h"
#include "ewspropertyfield.h"
#include "ewscontacthandler.h"

#include "ewsresource_debug.h"
using namespace KContacts;

EwsCreateContactJob::EwsCreateContactJob(EwsClient &client,
                                         const Akonadi::Item &item,
                                         const Akonadi::Collection &collection,
                                         EwsTagStore *tagStore,
                                         EwsResource *parent)
    : EwsCreateItemJob(client, item, collection, tagStore, parent)
{
}

EwsCreateContactJob::~EwsCreateContactJob()
{
}

void EwsCreateContactJob::doStart()
{
    qCWarning(EWSRES_LOG) << QStringLiteral("Contact item creation not implemented!");
    if (!mItem.hasPayload<Addressee>()) {
        setErrorMsg(QStringLiteral("Expected MIME message payload"));
        emitResult();
    }
    auto req = new EwsCreateItemRequest(mClient, this);
    Addressee Contact = mItem.payload<Addressee>();
    EwsItem item;
    QStringList customes = Contact.customs();
    QStringList imAddresses;
    item.setType(EwsItemTypeContact);
    item.setField(EwsItemFieldEmailAddresses, QVariant::fromValue(Contact.emailList()));
    item.setField(EwsItemFieldImAddresses, QVariant::fromValue(Contact.imppList()));
    item.setField(EwsItemFieldGivenName, Contact.givenName());
    item.setField(EwsItemFieldSurname, Contact.familyName());
    item.setField(EwsItemFieldNickname, Contact.nickName());
    if(Contact.birthday().isValid())
        item.setField(EwsItemFieldBirthday,Contact.birthday());
    item.setField(EwsItemFieldMiddleName,Contact.additionalName());
    item.setField(EwsItemFieldSpouseName,Contact.spousesName());
    item.setField(EwsItemFieldNotes,Contact.note());
    if(Contact.anniversary().isValid())
        item.setField(EwsItemFieldWeddingAnniversary,Contact.anniversary());
    item.setField(EwsItemFieldPhoto,Contact.photo().rawData());
    req->setSavedFolderId(EwsId(mCollection.remoteId(), mCollection.remoteRevision()));
    populateCommonProperties(item);

    req->setItems(EwsItem::List() << item);
    req->setMessageDisposition(EwsDispSaveOnly);
    connect(req,
            &EwsCreateItemRequest::finished,
            this,
            &EwsCreateContactJob::contactCreateFinished);
    addSubjob(req);
    req->start();
}

void EwsCreateContactJob::contactCreateFinished(KJob *job)
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
        setErrorMsg(i18n("Failed to create contact item"));
    }

    emitResult();
}

bool EwsCreateContactJob::setSend(bool send)
{
    Q_UNUSED(send)

    qCWarning(EWSRES_LOG) << QStringLiteral("Sending contact items is not supported!");
    return false;
}
