/*
    SPDX-FileCopyrightText: 2021 Andreas Merl <merl.andreas@outlook.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsitemclassattribute.h"
#include <QByteArray>
#include <QString>


ewsItemClassAttribute::ewsItemClassAttribute(QString folderClass)
    : mItemClass(folderClass)
{
}

void ewsItemClassAttribute::setItemClass(QString folderClass)
{
    mItemClass = folderClass;
}

QString ewsItemClassAttribute::ItemClass()
{
    return mItemClass;
}

QByteArray ewsItemClassAttribute::type() const
{
    static const QByteArray sType("ItemClass");
    return sType;
}

Akonadi::Attribute *ewsItemClassAttribute::clone() const
{
    return new ewsItemClassAttribute(mItemClass);
}

void ewsItemClassAttribute::deserialize(const QByteArray& data)
{
    mItemClass = QString::fromUtf8(data.toStdString().data());
}

QByteArray ewsItemClassAttribute::serialized() const
{
    return QByteArray(mItemClass.toUtf8());
}


