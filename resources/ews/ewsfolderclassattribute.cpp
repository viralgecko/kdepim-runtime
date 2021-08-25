/*
    SPDX-FileCopyrightText: 2021 Andreas Merl <merl.andreas@outlook.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsfolderclassattribute.h"
#include <QByteArray>
#include <QString>


ewsFolderClassAttribute::ewsFolderClassAttribute(QString folderClass)
    : mFolderClass(folderClass)
{
}

void ewsFolderClassAttribute::setFolderClass(QString folderClass)
{
    mFolderClass = folderClass;
}

QString ewsFolderClassAttribute::FolderClass()
{
    return mFolderClass;
}

QByteArray ewsFolderClassAttribute::type() const
{
    static const QByteArray sType("FolderClass");
    return sType;
}

Akonadi::Attribute *ewsFolderClassAttribute::clone() const
{
    return new ewsFolderClassAttribute(mFolderClass);
}

void ewsFolderClassAttribute::deserialize(const QByteArray& data)
{
    mFolderClass = QString::fromUtf8(data.toStdString().data());
}

QByteArray ewsFolderClassAttribute::serialized() const
{
    return QByteArray(mFolderClass.toUtf8());
}


