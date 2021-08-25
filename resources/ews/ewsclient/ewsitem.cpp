/*
    SPDX-FileCopyrightText: 2015-2017 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsitem.h"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <KContacts/Email>
#include <KContacts/Address>
#include <KCalendarCore/Todo>
#include <KContacts/PhoneNumber>

#include "ewsattachment.h"
#include "ewsattendee.h"
#include "ewsclient_debug.h"
#include "ewsitembase_p.h"
#include "ewsmailbox.h"
#include "ewsoccurrence.h"
#include "ewsrecurrence.h"
#include "ewsxml.h"

#define D_PTR EwsItemPrivate *d = reinterpret_cast<EwsItemPrivate *>(this->d.data());
#define D_CPTR const EwsItemPrivate *d = reinterpret_cast<const EwsItemPrivate *>(this->d.data());

class EwsItemPrivate : public EwsItemBasePrivate
{
public:
    typedef EwsXml<EwsItemFields> Reader;

    EwsItemPrivate();
    EwsItemPrivate(const EwsItemBasePrivate &other);
    EwsItemBasePrivate *clone() const override
    {
        return new EwsItemPrivate(*this);
    }

    static bool bodyReader(QXmlStreamReader &reader, QVariant &val);
    static bool messageHeadersReader(QXmlStreamReader &reader, QVariant &val);
    static bool mailboxReader(QXmlStreamReader &reader, QVariant &val);
    static bool recipientsReader(QXmlStreamReader &reader, QVariant &val);
    static bool timezoneReader(QXmlStreamReader &reader, QVariant &val);
    static bool attendeesReader(QXmlStreamReader &reader, QVariant &val);
    static bool occurrenceReader(QXmlStreamReader &reader, QVariant &val);
    static bool occurrencesReader(QXmlStreamReader &reader, QVariant &val);
    static bool recurrenceReader(QXmlStreamReader &reader, QVariant &val);
    static bool stringListReader(QXmlStreamReader &reader, QVariant &val);
    static bool stringListWriter(QXmlStreamWriter &writer, const QVariant &val);
    static bool attachmentsReader(QXmlStreamReader &reader, QVariant &val);
    static bool emailAddressesReader(QXmlStreamReader &reader, QVariant &val);
    static bool emailAddressesWriter(QXmlStreamWriter &writer, const QVariant &val);
    static bool imAddressesReader(QXmlStreamReader &reader, QVariant &val);
    static bool imAddressesWriter(QXmlStreamWriter &writer, const QVariant &val);
    static bool taskStatusReader(QXmlStreamReader &reader, QVariant &val);
    static bool taskStatusWriter(QXmlStreamWriter &writer, const QVariant &val);
    static bool phoneNumberWriter(QXmlStreamWriter &writer, const QVariant &val);
    static bool phoneNumberReader(QXmlStreamReader &reader, QVariant &val);

    bool operator==(const EwsItemPrivate &other) const;

    EwsItemType mType;
    static const Reader mStaticEwsXml;
    Reader mEwsXml;
};
// Todo
// figure out:
// task:Status
//
// task:Recurrence is quiet different from a CalendarItem:Recurrence
static const QVector<EwsItemPrivate::Reader::Item> ewsItemItems = {
    // Item fields
    {EwsItemFieldMimeContent, QStringLiteral("MimeContent"), &ewsXmlBase64Reader, &ewsXmlBase64Writer},
    {EwsItemFieldItemId, QStringLiteral("ItemId"), &ewsXmlIdReader, &ewsXmlIdWriter},
    {EwsItemFieldParentFolderId, QStringLiteral("ParentFolderId"), &ewsXmlIdReader, &ewsXmlIdWriter},
    {EwsItemFieldItemClass, QStringLiteral("ItemClass"), &ewsXmlTextReader, &ewsXmlTextWriter},
    {EwsItemFieldSubject, QStringLiteral("Subject"), &ewsXmlTextReader, &ewsXmlTextWriter},
    {EwsItemFieldSensitivity, QStringLiteral("Sensitivity"), &ewsXmlSensitivityReader},
    {EwsItemFieldBody, QStringLiteral("Body"), &EwsItemPrivate::bodyReader},
    {EwsItemFieldAttachments, QStringLiteral("Attachments"), &EwsItemPrivate::attachmentsReader},
    {EwsItemFieldDateTimeReceived, QStringLiteral("DateTimeReceived"), &ewsXmlDateTimeReader},
    {EwsItemFieldSize, QStringLiteral("Size"), &ewsXmlUIntReader},
    {EwsItemFieldCategories, QStringLiteral("Categories"), &EwsItemPrivate::stringListReader, &EwsItemPrivate::stringListWriter},
    {EwsItemFieldImportance, QStringLiteral("Importance"), &ewsXmlImportanceReader},
    {EwsItemFieldInReplyTo, QStringLiteral("InReplyTo"), &ewsXmlTextReader},
    {EwsItemFieldIsDraft, QStringLiteral("IsDraft"), &ewsXmlBoolReader, &ewsXmlBoolWriter},
    {EwsItemFieldIsFromMe, QStringLiteral("IsFromMe"), &ewsXmlBoolReader},
    {EwsItemFieldInternetMessageHeaders, QStringLiteral("InternetMessageHeaders"), &EwsItemPrivate::messageHeadersReader},
    {EwsItemFieldExtendedProperties,
     QStringLiteral("ExtendedProperty"),
     &EwsItemBasePrivate::extendedPropertyReader,
     &EwsItemBasePrivate::extendedPropertyWriter},
    {EwsItemFieldHasAttachments, QStringLiteral("HasAttachments"), &ewsXmlBoolReader},
    // Message fields
    {EwsItemFieldToRecipients, QStringLiteral("ToRecipients"), &EwsItemPrivate::recipientsReader},
    {EwsItemFieldCcRecipients, QStringLiteral("CcRecipients"), &EwsItemPrivate::recipientsReader},
    {EwsItemFieldBccRecipients, QStringLiteral("BccRecipients"), &EwsItemPrivate::recipientsReader},
    {EwsItemFieldFrom, QStringLiteral("From"), &EwsItemPrivate::mailboxReader},
    {EwsItemFieldInternetMessageId, QStringLiteral("InternetMessageId"), &ewsXmlTextReader},
    {EwsItemFieldIsRead, QStringLiteral("IsRead"), &ewsXmlBoolReader, &ewsXmlBoolWriter},
    {EwsItemFieldReferences, QStringLiteral("References"), &ewsXmlTextReader},
    {EwsItemFieldReplyTo, QStringLiteral("ReplyTo"), &EwsItemPrivate::mailboxReader},
    // Task fields
    {EwsItemFieldActualWork, QStringLiteral("ActualWork"), &ewsXmlIntReader, &ewsXmlIntWriter},
    {EwsItemFieldAssignedTime, QStringLiteral("AssignedTime"), &ewsXmlDateTimeReader, &ewsXmlDateTimeWriter},
    {EwsItemFieldBillingInformation, QStringLiteral("BillingInformation"), &ewsXmlTextReader, &ewsXmlTextWriter},
    {EwsItemFieldChangeCount, QStringLiteral("ChangeCount"), &ewsXmlUIntReader, &ewsXmlUIntWriter},
    {EwsItemFieldCompanies, QStringLiteral("Companies"), &ewsXmlTextReader, &ewsXmlTextWriter},
    {EwsItemFieldCompleteDate, QStringLiteral("CompleteDate"), &ewsXmlDateTimeReader, &ewsXmlDateTimeWriter},
    {EwsItemFieldContacts, QStringLiteral("Contacts"), &EwsItemPrivate::stringListReader, &EwsItemPrivate::stringListWriter},
    {EwsItemFieldDelegationState, QStringLiteral("DelegationState"), &ewsXmlTextReader},
    {EwsItemFieldDelegator, QStringLiteral("Delegator"), &ewsXmlTextReader, &ewsXmlTextWriter},
    {EwsItemFieldDueDate, QStringLiteral("DueDate"), &ewsXmlDateTimeReader, &ewsXmlDateTimeWriter},
    {EwsItemFieldIsAssignmentEditable, QStringLiteral("IsAssignmentEditable"), &ewsXmlUIntReader, &ewsXmlUIntWriter},
    {EwsItemFieldIsComplete, QStringLiteral("IsComplete"), &ewsXmlBoolReader},
    {EwsItemFieldIsRecurring, QStringLiteral("IsRecurring"), &ewsXmlBoolReader},
    {EwsItemFieldIsTeamTask, QStringLiteral("IsTeamTask"), &ewsXmlBoolReader},
    {EwsItemFieldMileage, QStringLiteral("Mileage"), &ewsXmlTextReader, &ewsXmlTextWriter},
    {EwsItemFieldOwner, QStringLiteral("Owner"), &ewsXmlTextReader, &ewsXmlTextWriter},
    {EwsItemFieldPercentComplete, QStringLiteral("PercentComplete"), &ewsXmlUIntReader, &ewsXmlUIntWriter},
    //{EwsItemFieldRecurrence, QStringLiteral("Recurrence"), },
    {EwsItemFieldStartDate, QStringLiteral("StartDate"), &ewsXmlDateTimeReader, &ewsXmlDateTimeWriter},
    {EwsItemFieldStatus, QStringLiteral("Status"), &EwsItemPrivate::taskStatusReader, &EwsItemPrivate::taskStatusWriter},
    {EwsItemFieldStatusDescription, QStringLiteral("StatusDescription"), &ewsXmlTextReader, &ewsXmlTextWriter},
    {EwsItemFieldTotalWork, QStringLiteral("TotalWork"), &ewsXmlIntReader, &ewsXmlIntWriter},
    // CalendarItem fields
    {EwsItemFieldCalendarItemType, QStringLiteral("CalendarItemType"), &ewsXmlCalendarItemTypeReader},
    {EwsItemFieldUID, QStringLiteral("UID"), &ewsXmlTextReader, &ewsXmlTextWriter},
    {EwsItemFieldCulture, QStringLiteral("Culture"), &ewsXmlTextReader, &ewsXmlTextWriter},
    {EwsItemFieldStartTimeZone, QStringLiteral("StartTimeZone"), &EwsItemPrivate::timezoneReader},
    {EwsItemFieldOrganizer, QStringLiteral("Organizer"), &EwsItemPrivate::mailboxReader},
    {EwsItemFieldRequiredAttendees, QStringLiteral("RequiredAttendees"), &EwsItemPrivate::attendeesReader},
    {EwsItemFieldOptionalAttendees, QStringLiteral("OptionalAttendees"), &EwsItemPrivate::attendeesReader},
    {EwsItemFieldResources, QStringLiteral("Resources"), &EwsItemPrivate::attendeesReader},
    {EwsItemFieldStart, QStringLiteral("Start"), &ewsXmlDateTimeReader, &ewsXmlDateTimeWriter},
    {EwsItemFieldEnd, QStringLiteral("End"), &ewsXmlDateTimeReader, &ewsXmlDateTimeWriter},
    {EwsItemFieldRecurrenceId, QStringLiteral("RecurrenceId"), &ewsXmlDateTimeReader, &ewsXmlDateTimeWriter},
    {EwsItemFieldIsAllDayEvent, QStringLiteral("IsAllDayEvent"), &ewsXmlBoolReader, &ewsXmlBoolWriter},
    {EwsItemFieldLegacyFreeBusyStatus, QStringLiteral("LegacyFreeBusyStatus"), &ewsXmlLegacyFreeBusyStatusReader},
    {EwsItemFieldMyResponseType, QStringLiteral("MyResponseType"), &ewsXmlResponseTypeReader},
    {EwsItemFieldAppointmentSequenceNumber, QStringLiteral("AppointmentSequenceNumber"), &ewsXmlUIntReader},
    {EwsItemFieldRecurrence, QStringLiteral("Recurrence"), &EwsItemPrivate::recurrenceReader},
    {EwsItemFieldFirstOccurrence, QStringLiteral("FirstOccurrence"), &EwsItemPrivate::occurrenceReader},
    {EwsItemFieldLastOccurrence, QStringLiteral("LastOccurrence"), &EwsItemPrivate::occurrenceReader},
    {EwsItemFieldModifiedOccurrences, QStringLiteral("ModifiedOccurrences"), &EwsItemPrivate::occurrencesReader},
    {EwsItemFieldDeletedOccurrences, QStringLiteral("DeletedOccurrences"), &EwsItemPrivate::occurrencesReader},
    {EwsItemFieldTimeZone, QStringLiteral("TimeZone"), &ewsXmlTextReader},
    {EwsItemFieldExchangePersonIdGuid, QStringLiteral("ExchangePersonIdGuid"), &ewsXmlTextReader},
    {EwsItemFieldDoNotForwardMeeting, QStringLiteral("DoNotForwardMeeting"), &ewsXmlBoolReader},
    // Contacts fields
    {EwsItemFieldFileAs, QStringLiteral("FileAs"), &ewsXmlTextReader, &ewsXmlTextWriter},
    {EwsItemFieldFileAsMapping, QStringLiteral("FileAsMapping"), &ewsXmlTextReader, &ewsXmlTextWriter},
    {EwsItemFieldDisplayName, QStringLiteral("DisplayName"), &ewsXmlTextReader, &ewsXmlTextWriter},
    {EwsItemFieldGivenName, QStringLiteral("GivenName"), &ewsXmlTextReader, &ewsXmlTextWriter},
    {EwsItemFieldInitials, QStringLiteral("Initials"), &ewsXmlTextReader, &ewsXmlTextWriter},
    {EwsItemFieldMiddleName, QStringLiteral("MiddleName"), &ewsXmlTextReader, &ewsXmlTextWriter},
    {EwsItemFieldNickname, QStringLiteral("Nickname"), &ewsXmlTextReader, &ewsXmlTextWriter},
    //{EwsItemFieldCompleteName, QStringLiteral("CompleteName"),},
    {EwsItemFieldCompanyName, QStringLiteral("CompanyName"), &ewsXmlTextReader, &ewsXmlTextWriter},
    {EwsItemFieldEmailAddresses, QStringLiteral("EmailAddresses"), &EwsItemPrivate::emailAddressesReader, &EwsItemPrivate::emailAddressesWriter},
    //{EwsItemFieldPhysicalAddresses, QStringLiteral("PhysicalAddresses"),},
    {EwsItemFieldPhoneNumbers, QStringLiteral("PhoneNumbers"), &EwsItemPrivate::phoneNumberReader, &EwsItemPrivate::phoneNumberWriter},
    {EwsItemFieldAssistantName, QStringLiteral("AssistantName"), &ewsXmlTextReader, &ewsXmlTextWriter},
    {EwsItemFieldBirthday, QStringLiteral("Birthday"), &ewsXmlDateTimeReader, &ewsXmlDateTimeWriter},
    {EwsItemFieldBusinessHomePage, QStringLiteral("BusinessHomePage"), &ewsXmlTextReader, &ewsXmlTextWriter},
    {EwsItemFieldChildren, QStringLiteral("Children"), &EwsItemPrivate::stringListReader, &EwsItemPrivate::stringListWriter},
    {EwsItemFieldContactSource, QStringLiteral("ContactSource"), &ewsXmlTextReader, &ewsXmlTextWriter},
    {EwsItemFieldDepartment, QStringLiteral("Department"), &ewsXmlTextReader, &ewsXmlTextWriter},
    {EwsItemFieldGeneration, QStringLiteral("Generation"), &ewsXmlTextReader, &ewsXmlTextWriter},
    {EwsItemFieldImAddresses, QStringLiteral("ImAddresses"), &EwsItemPrivate::imAddressesReader, &EwsItemPrivate::imAddressesWriter},
    {EwsItemFieldJobTitle, QStringLiteral("JobTitle"), &ewsXmlTextReader, &ewsXmlTextWriter},
    {EwsItemFieldManager, QStringLiteral("Manager"), &ewsXmlTextReader, &ewsXmlTextWriter},
    {EwsItemFieldOfficeLocation, QStringLiteral("OfficeLocation"), &ewsXmlTextReader, &ewsXmlTextWriter},
    {EwsItemFieldPostalAddressIndex, QStringLiteral("PostalAddressIndex"), &ewsXmlTextReader, &ewsXmlTextWriter},
    {EwsItemFieldProfession, QStringLiteral("Profession"), &ewsXmlTextReader, &ewsXmlTextWriter},
    {EwsItemFieldSpouseName, QStringLiteral("SpouseName"), &ewsXmlTextReader, &ewsXmlTextWriter},
    {EwsItemFieldSurname, QStringLiteral("Surname"), &ewsXmlTextReader, &ewsXmlTextWriter},
    {EwsItemFieldWeddingAnniversary, QStringLiteral("WeddingAnniversary"), &ewsXmlDateTimeReader, &ewsXmlDateTimeWriter},
    {EwsItemFieldHasPicture, QStringLiteral("HasPicture"), &ewsXmlBoolReader},
    {EwsItemFieldPhoneticFullName, QStringLiteral("PhoneticFullName"), &ewsXmlTextReader, &ewsXmlTextWriter},
    {EwsItemFieldPhoneticFirstName, QStringLiteral("PhoneticFirstName"), &ewsXmlTextReader, &ewsXmlTextWriter},
    {EwsItemFieldPhoneticLastName, QStringLiteral("PhoneticLastName"), &ewsXmlTextReader, &ewsXmlTextWriter},
    {EwsItemFieldAlias, QStringLiteral("Alias"), &ewsXmlTextReader, &ewsXmlTextWriter},
    {EwsItemFieldNotes, QStringLiteral("Notes"), &ewsXmlTextReader, &ewsXmlTextWriter},
    {EwsItemFieldPhoto, QStringLiteral("Photo"), &ewsXmlBase64Reader, &ewsXmlBase64Writer},
    {EwsItemFieldUserSMIMECertificate, QStringLiteral("SMIMECertificate"), &ewsXmlBase64Reader, &ewsXmlBase64Writer},
    {EwsItemFieldMSExchangeCertificate, QStringLiteral("MSExchangeCertificate"), &ewsXmlBase64Reader, &ewsXmlBase64Writer},
};

const EwsItemPrivate::Reader EwsItemPrivate::mStaticEwsXml(ewsItemItems);

EwsItemPrivate::EwsItemPrivate()
    : EwsItemBasePrivate()
    , mType(EwsItemTypeUnknown)
    , mEwsXml(mStaticEwsXml)
{
}

bool EwsItemPrivate::bodyReader(QXmlStreamReader &reader, QVariant &val)
{
    QVariantList vl;
    QStringRef bodyType = reader.attributes().value(QStringLiteral("BodyType"));
    if (bodyType.isNull()) {
        qCWarningNC(EWSCLI_LOG) << QStringLiteral("Failed to read %1 element - missing %2 attribute").arg(QStringLiteral("Body"), QStringLiteral("BodyType"));
        return false;
    }
    bool isHtml;
    if (bodyType == QLatin1String("HTML")) {
        isHtml = true;
    } else if (bodyType == QLatin1String("Text")) {
        isHtml = false;
    } else {
        qCWarningNC(EWSCLI_LOG) << QStringLiteral("Failed to read Body element- unknown body type");
        return false;
    }
    vl.append(reader.readElementText());
    if (reader.error() != QXmlStreamReader::NoError) {
        qCWarningNC(EWSCLI_LOG) << QStringLiteral("Failed to read %1 element - invalid content.").arg(QStringLiteral("Body"));
        return false;
    }
    vl.append(isHtml);
    val = vl;
    return true;
}

bool EwsItemPrivate::messageHeadersReader(QXmlStreamReader &reader, QVariant &val)
{
    EwsItem::HeaderMap map = val.value<EwsItem::HeaderMap>();

    while (reader.readNextStartElement()) {
        if (reader.namespaceUri() != ewsTypeNsUri) {
            qCWarningNC(EWSCLI_LOG) << QStringLiteral("Unexpected namespace in InternetMessageHeaders element:") << reader.namespaceUri();
            return false;
        }

        if (reader.name() == QLatin1String("InternetMessageHeader")) {
            QStringRef nameRef = reader.attributes().value(QStringLiteral("HeaderName"));
            if (nameRef.isNull()) {
                qCWarningNC(EWSCLI_LOG) << QStringLiteral("Missing HeaderName attribute in InternetMessageHeader element.");
                return false;
            }
            QString name = nameRef.toString();
            QString value = reader.readElementText();
            map.insert(name, value);
            if (reader.error() != QXmlStreamReader::NoError) {
                qCWarning(EWSCLI_LOG) << QStringLiteral("Failed to read EWS request - invalid %1 element.").arg(QStringLiteral("InternetMessageHeader"));
                return false;
            }
        }
    }

    val = QVariant::fromValue<EwsItem::HeaderMap>(map);

    return true;
}

bool EwsItemPrivate::recipientsReader(QXmlStreamReader &reader, QVariant &val)
{
    EwsMailbox::List mboxList;

    while (reader.readNextStartElement()) {
        if (reader.namespaceUri() != ewsTypeNsUri) {
            qCWarningNC(EWSCLI_LOG) << QStringLiteral("Unexpected namespace in %1 element:").arg(reader.name().toString()) << reader.namespaceUri();
            return false;
        }
        EwsMailbox mbox(reader);
        if (!mbox.isValid()) {
            return false;
        }
        mboxList.append(mbox);
    }

    val = QVariant::fromValue<EwsMailbox::List>(mboxList);

    return true;
}

bool EwsItemPrivate::mailboxReader(QXmlStreamReader &reader, QVariant &val)
{
    if (!reader.readNextStartElement()) {
        qCWarningNC(EWSCLI_LOG) << QStringLiteral("Expected mailbox in %1 element:").arg(reader.name().toString());
        return false;
    }

    if (reader.namespaceUri() != ewsTypeNsUri) {
        qCWarningNC(EWSCLI_LOG) << QStringLiteral("Unexpected namespace in %1 element:").arg(reader.name().toString()) << reader.namespaceUri();
        reader.skipCurrentElement();
        return false;
    }

    EwsMailbox mbox(reader);
    if (!mbox.isValid()) {
        reader.skipCurrentElement();
        return false;
    }

    val = QVariant::fromValue<EwsMailbox>(mbox);

    // Leave the element
    reader.skipCurrentElement();

    return true;
}

bool EwsItemPrivate::timezoneReader(QXmlStreamReader &reader, QVariant &val)
{
    // TODO: This only reads the timezone identifier.
    QStringRef idRef = reader.attributes().value(QStringLiteral("Id"));
    if (idRef.isNull()) {
        qCWarningNC(EWSCLI_LOG) << QStringLiteral("Error reading %1 element - missing %2 attribute").arg(reader.name().toString(), QStringLiteral("Id"));
        reader.skipCurrentElement();
        return false;
    } else {
        reader.skipCurrentElement();
        val = idRef.toString();
        return true;
    }
}

bool EwsItemPrivate::attendeesReader(QXmlStreamReader &reader, QVariant &val)
{
    EwsAttendee::List attList;

    while (reader.readNextStartElement()) {
        if (reader.namespaceUri() != ewsTypeNsUri) {
            qCWarningNC(EWSCLI_LOG) << QStringLiteral("Unexpected namespace in %1 element:").arg(reader.name().toString()) << reader.namespaceUri();
            return false;
        }
        EwsAttendee att(reader);
        if (!att.isValid()) {
            return false;
        }
        attList.append(att);
    }

    val = QVariant::fromValue<EwsAttendee::List>(attList);

    return true;
}

bool EwsItemPrivate::occurrenceReader(QXmlStreamReader &reader, QVariant &val)
{
    EwsOccurrence occ(reader);
    if (!occ.isValid()) {
        return false;
    }
    val = QVariant::fromValue<EwsOccurrence>(occ);
    return true;
}

bool EwsItemPrivate::occurrencesReader(QXmlStreamReader &reader, QVariant &val)
{
    EwsOccurrence::List occList;

    while (reader.readNextStartElement()) {
        if (reader.namespaceUri() != ewsTypeNsUri) {
            qCWarningNC(EWSCLI_LOG) << QStringLiteral("Unexpected namespace in %1 element:").arg(reader.name().toString()) << reader.namespaceUri();
            return false;
        }
        EwsOccurrence occ(reader);
        if (!occ.isValid()) {
            return false;
        }
        occList.append(occ);
    }

    val = QVariant::fromValue<EwsOccurrence::List>(occList);

    return true;
}

bool EwsItemPrivate::stringListReader(QXmlStreamReader &reader, QVariant &val)
{
    QStringList list;

    while (reader.readNextStartElement()) {
        if (reader.namespaceUri() != ewsTypeNsUri) {
            qCWarningNC(EWSCLI_LOG) << QStringLiteral("Unexpected namespace in %1 element:").arg(reader.name().toString()) << reader.namespaceUri();
            return false;
        }

        if (reader.name() == QLatin1String("String")) {
            list.append(reader.readElementText());
            if (reader.error() != QXmlStreamReader::NoError) {
                qCWarning(EWSCLI_LOG) << QStringLiteral("Failed to read EWS request - invalid %1 element.").arg(QStringLiteral("Categories/Value"));
                return false;
            }
        }
    }

    val = list;

    return true;
}

bool EwsItemPrivate::stringListWriter(QXmlStreamWriter &writer, const QVariant &val)
{
    const QStringList list = val.toStringList();

    for (const QString &cat : list) {
        writer.writeTextElement(ewsTypeNsUri, QStringLiteral("String"), cat);
    }

    return true;
}

bool EwsItemPrivate::recurrenceReader(QXmlStreamReader &reader, QVariant &val)
{
    EwsRecurrence recurrence(reader);
    val = QVariant::fromValue<EwsRecurrence>(recurrence);
    return true;
}

bool EwsItemPrivate::attachmentsReader(QXmlStreamReader &reader, QVariant &val)
{
    EwsAttachment::List attList;

    while (reader.readNextStartElement()) {
        if (reader.namespaceUri() != ewsTypeNsUri) {
            qCWarningNC(EWSCLI_LOG) << QStringLiteral("Unexpected namespace in %1 element:").arg(reader.name().toString()) << reader.namespaceUri();
            return false;
        }
        EwsAttachment att(reader);
        if (!att.isValid()) {
            return false;
        }
        attList.append(att);
    }

    val = QVariant::fromValue<EwsAttachment::List>(attList);

    return true;
}

bool EwsItemPrivate::emailAddressesReader(QXmlStreamReader &reader, QVariant &val)
{
    KContacts::Email::List emailList;
    while (reader.readNextStartElement()) {
        if (reader.namespaceUri() != ewsTypeNsUri) {
            qCWarningNC(EWSCLI_LOG) << QStringLiteral("Unexpected namespace in %1 element:").arg(reader.name().toString()) << reader.namespaceUri();
            return false;
        }

        if (reader.name() == QLatin1String("Entry")) {
            emailList.append(KContacts::Email(reader.readElementText()));
            if (reader.error() != QXmlStreamReader::NoError) {
                qCWarning(EWSCLI_LOG) << QStringLiteral("Failed to read EWS request - invalid %1 element.").arg(QStringLiteral("EmailAddresses/Value"));
                return false;
            }
        }
    }
    val = QVariant::fromValue(emailList);
    return true;
}

bool EwsItemPrivate::emailAddressesWriter(QXmlStreamWriter &writer, const QVariant &val)
{
    const KContacts::Email::List list = val.value<KContacts::Email::List>();
    int i = 1;

    for (const KContacts::Email &email : list) {
        if(i > 3)
        {
            qCWarning(EWSCLI_LOG) << QStringLiteral("Failed to write EWS request - to many Email Addresses (only 3 supported).");
            return false;
        }
        writer.writeStartElement(ewsTypeNsUri, QStringLiteral("Entry"));
        writer.writeAttribute(QString::fromUtf8("Key"),QString::fromUtf8("EmailAddress").append(QString::number(i)));
        writer.writeCharacters(email.mail());
        writer.writeEndElement();
        i++;
    }
    return true;
}

bool EwsItemPrivate::imAddressesReader(QXmlStreamReader &reader, QVariant &val)
{
    QStringList imaddressList;
    while (reader.readNextStartElement()) {
        if (reader.namespaceUri() != ewsTypeNsUri) {
            qCWarningNC(EWSCLI_LOG) << QStringLiteral("Unexpected namespace in %1 element:").arg(reader.name().toString()) << reader.namespaceUri();
            return false;
        }

        if (reader.name() == QLatin1String("Entry")) {
            imaddressList.append(reader.readElementText());
            if (reader.error() != QXmlStreamReader::NoError) {
                qCWarning(EWSCLI_LOG) << QStringLiteral("Failed to read EWS request - invalid %1 element.").arg(QStringLiteral("EmailAddresses/Value"));
                return false;
            }
        }
    }
    val = QVariant::fromValue(imaddressList);
    return true;
}

bool EwsItemPrivate::imAddressesWriter(QXmlStreamWriter &writer, const QVariant &val)
{
    const QStringList list = val.toStringList();
    int i = 1;

    for (const QString &imAddress : list) {
        if(i > 3)
        {
            qCWarning(EWSCLI_LOG) << QStringLiteral("Failed to write EWS request - to many Email Addresses (only 3 supported).");
            return false;
        }
        writer.writeStartElement(ewsTypeNsUri, QStringLiteral("Entry"));
        writer.writeAttribute(QString::fromUtf8("Key"),QString::fromUtf8("ImAddress").append(QString::number(i)));
        writer.writeCharacters(imAddress);
        writer.writeEndElement();
        i++;
    }
    return true;
}

bool EwsItemPrivate::taskStatusReader(QXmlStreamReader &reader, QVariant &val)
{
    QString elmName = reader.name().toString();
    QString status = reader.readElementText();
    if (reader.error() != QXmlStreamReader::NoError) {
        qCWarningNC(EWSCLI_LOG) << QStringLiteral("Failed to read %1 element - invalid content.").arg(elmName);
        reader.skipCurrentElement();
        return false;
    }
    if(status == QString::fromUtf8("NotStarted"))
        val = KCalendarCore::Todo::Status::StatusNone;
    if(status == QString::fromUtf8("InProgress"))
        val = KCalendarCore::Todo::Status::StatusInProcess;
    if(status == QString::fromUtf8("WaitingOnOthers"))
        val = KCalendarCore::Todo::Status::StatusTentative;
    if(status == QString::fromUtf8("Deferred"))
        val = KCalendarCore::Todo::Status::StatusX;
    if(status == QString::fromUtf8("Completed"))
        val = KCalendarCore::Todo::Status::StatusCompleted;
    return true;
}

bool EwsItemPrivate::taskStatusWriter(QXmlStreamWriter &writer, const QVariant &val)
{
    switch(val.toInt())
    {
        case KCalendarCore::Todo::Status::StatusNone:
            writer.writeCharacters(QString::fromUtf8("NotStarted"));
            return true;
        case KCalendarCore::Todo::Status::StatusCompleted:
            writer.writeCharacters(QString::fromUtf8("Completed"));
            return true;
        case KCalendarCore::Todo::Status::StatusInProcess:
            writer.writeCharacters(QString::fromUtf8("InProgress"));
            return true;
        case KCalendarCore::Todo::Status::StatusTentative:
            writer.writeCharacters(QString::fromUtf8("WaitingOnOthers"));
            return true;
        case KCalendarCore::Todo::Status::StatusX:
            writer.writeCharacters(QString::fromUtf8("Deferred"));
            return true;
        default:
            return false;
    }
}

bool EwsItemPrivate::phoneNumberReader(QXmlStreamReader &reader, QVariant &val)
{
    //todo: add phone number type support
    KContacts::PhoneNumber::List numberList;
    KContacts::PhoneNumber();
    while (reader.readNextStartElement()) {
        if (reader.namespaceUri() != ewsTypeNsUri) {
            qCWarningNC(EWSCLI_LOG) << QStringLiteral("Unexpected namespace in %1 element:").arg(reader.name().toString()) << reader.namespaceUri();
            return false;
        }

        if (reader.name() == QLatin1String("Entry")) {
            numberList.append(KContacts::PhoneNumber(reader.readElementText()));
            if (reader.error() != QXmlStreamReader::NoError) {
                qCWarning(EWSCLI_LOG) << QStringLiteral("Failed to read EWS request - invalid %1 element.").arg(QStringLiteral("EmailAddresses/Value"));
                return false;
            }
        }
    }
    val = QVariant::fromValue(numberList);
    return true;
}

bool EwsItemPrivate::phoneNumberWriter(QXmlStreamWriter &writer, const QVariant &val)
{
    const KContacts::PhoneNumber::List list = val.value<KContacts::PhoneNumber::List>();
    int i = 1;

    for (const KContacts::PhoneNumber &phoneNumber : list) {
        writer.writeStartElement(ewsTypeNsUri, QStringLiteral("Entry"));
        switch (phoneNumber.type())
        {
            case KContacts::PhoneNumber::TypeFlag::Home:
                writer.writeAttribute(QString::fromUtf8("Key"),QString::fromUtf8("HomePhone"));
                break;
            case KContacts::PhoneNumber::TypeFlag::Work:
                writer.writeAttribute(QString::fromUtf8("Key"),QString::fromUtf8("BusinessPhone"));
                break;
            case KContacts::PhoneNumber::TypeFlag::Fax:
                writer.writeAttribute(QString::fromUtf8("Key"),QString::fromUtf8("BusinessFax"));
                break;
            case KContacts::PhoneNumber::TypeFlag::Car:
                writer.writeAttribute(QString::fromUtf8("Key"),QString::fromUtf8("CarPhone"));
                break;
            case KContacts::PhoneNumber::TypeFlag::Cell:
                writer.writeAttribute(QString::fromUtf8("Key"),QString::fromUtf8("MobilePhone"));
                break;
            case KContacts::PhoneNumber::TypeFlag::Pager:
                writer.writeAttribute(QString::fromUtf8("Key"),QString::fromUtf8("Pager"));
                break;
            case KContacts::PhoneNumber::TypeFlag::Pref:
                writer.writeAttribute(QString::fromUtf8("Key"),QString::fromUtf8("PrimaryPhone"));
                break;
            case KContacts::PhoneNumber::TypeFlag::Undefined:
                writer.writeAttribute(QString::fromUtf8("Key"),QString::fromUtf8("OtherTelephone"));
                break;
        }
        writer.writeCharacters(phoneNumber.number());
        writer.writeEndElement();
        i++;
    }
    return true;
}

bool EwsItemPrivate::operator==(const EwsItemPrivate &other) const
{
    if (!EwsItemBasePrivate::operator==(other)) {
        return false;
    }
    return mType == other.mType;
}

EwsItem::EwsItem()
    : EwsItemBase(QSharedDataPointer<EwsItemBasePrivate>(new EwsItemPrivate()))
{
}

EwsItem::EwsItem(QXmlStreamReader &reader)
    : EwsItemBase(QSharedDataPointer<EwsItemBasePrivate>(new EwsItemPrivate()))
{
    qRegisterMetaType<EwsItem::HeaderMap>();
    qRegisterMetaType<EwsMailbox>();
    qRegisterMetaType<EwsMailbox::List>();
    qRegisterMetaType<EwsAttendee>();
    qRegisterMetaType<EwsAttendee::List>();
    qRegisterMetaType<EwsRecurrence>();
    qRegisterMetaType<EwsOccurrence>();
    qRegisterMetaType<EwsOccurrence::List>();

    auto *d = reinterpret_cast<EwsItemPrivate *>(this->d.data());

    // Check what item type are we
    const QStringRef elmName = reader.name();
    if (elmName == QLatin1String("Item")) {
        d->mType = EwsItemTypeItem;
        const QStringRef subtype = reader.attributes().value(QStringLiteral("xsi:type"));
        if (!subtype.isEmpty()) {
            auto tokens = subtype.split(QLatin1Char(':'));
            const QStringRef type = tokens.size() == 1 ? tokens[0] : tokens[1];
            if (type == QLatin1String("AbchPersonItemType")) {
                d->mType = EwsItemTypeAbchPerson;
            }
        }
    } else if (elmName == QLatin1String("Message")) {
        d->mType = EwsItemTypeMessage;
    } else if (elmName == QLatin1String("CalendarItem")) {
        d->mType = EwsItemTypeCalendarItem;
    } else if (elmName == QLatin1String("Contact")) {
        d->mType = EwsItemTypeContact;
    } else if (elmName == QLatin1String("DistributionList")) {
        d->mType = EwsItemTypeDistributionList;
    } else if (elmName == QLatin1String("MeetingMessage")) {
        d->mType = EwsItemTypeMeetingMessage;
    } else if (elmName == QLatin1String("MeetingRequest")) {
        d->mType = EwsItemTypeMeetingRequest;
    } else if (elmName == QLatin1String("MeetingResponse")) {
        d->mType = EwsItemTypeMeetingResponse;
    } else if (elmName == QLatin1String("MeetingCancellation")) {
        d->mType = EwsItemTypeMeetingCancellation;
    } else if (elmName == QLatin1String("PostItem")) {
        d->mType = EwsItemTypePostItem;
    } else if (elmName == QLatin1String("Task")) {
        d->mType = EwsItemTypeTask;
    }

    while (reader.readNextStartElement()) {
        if (reader.namespaceUri() != ewsTypeNsUri) {
            qCWarningNC(EWSCLI_LOG) << "Unexpected namespace in Item element:" << reader.namespaceUri();
            return;
        }

        if (!readBaseItemElement(reader)) {
            qCWarningNC(EWSCLI_LOG) << QStringLiteral("Invalid Item child: %1").arg(reader.qualifiedName().toString());
            return;
        }
    }
    d->mValid = true;
}

EwsItem::EwsItem(const EwsItem &other)
    : EwsItemBase(other.d)
{
}

EwsItem::EwsItem(EwsItem &&other)
    : EwsItemBase(other.d)
{
}

EwsItem::~EwsItem()
{
}

EwsItem &EwsItem::operator=(const EwsItem &other)
{
    d = other.d;
    return *this;
}

EwsItem &EwsItem::operator=(EwsItem &&other)
{
    d = std::move(other.d);
    return *this;
}

EwsItemType EwsItem::type() const
{
    const auto *d = reinterpret_cast<const EwsItemPrivate *>(this->d.data());
    return d->mType;
}

void EwsItem::setType(EwsItemType type)
{
    D_PTR
    d->mType = type;
    d->mValid = true;
}

EwsItemType EwsItem::internalType() const
{
    D_CPTR

    EwsItemType type = d->mType;
    switch (type) {
    case EwsItemTypeMeetingMessage:
    case EwsItemTypeMeetingRequest:
    case EwsItemTypeMeetingResponse:
    case EwsItemTypeMeetingCancellation:
        type = EwsItemTypeMessage;
        break;
    case EwsItemTypeDistributionList:
        type = EwsItemTypeContact;
        break;
    default:
        break;
    }
    return type;
}

bool EwsItem::readBaseItemElement(QXmlStreamReader &reader)
{
    D_PTR

    if (!d->mEwsXml.readItem(reader, QStringLiteral("Item"), ewsTypeNsUri)) {
        return false;
    }

    d->mFields = d->mEwsXml.values();

    // The body item is special as it hold two values in one. Need to separate them into their
    // proper places.
    if (d->mFields.contains(EwsItemFieldBody)) {
        QVariantList vl = d->mFields[EwsItemFieldBody].value<QVariantList>();
        QVariantList::const_iterator it = vl.cbegin();
        d->mFields[EwsItemFieldBody] = *it++;
        d->mFields[EwsItemFieldBodyIsHtml] = *it;
    }
    return true;
}

bool EwsItem::write(QXmlStreamWriter &writer) const
{
    D_CPTR

    writer.writeStartElement(ewsTypeNsUri, ewsItemTypeNames[d->mType]);

    bool status = d->mEwsXml.writeItems(writer, ewsItemTypeNames[d->mType], ewsTypeNsUri, d->mFields);

    writer.writeEndElement();

    return status;
}

bool EwsItem::operator==(const EwsItem &other) const
{
    return *d == *other.d;
}
