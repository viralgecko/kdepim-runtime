/*
    SPDX-FileCopyrightText: 2021 Andreas Merl <merl.andreas@outlook.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <attribute.h>

class ewsItemClassAttribute : public Akonadi::Attribute
{
public:
    explicit ewsItemClassAttribute(QString folderClass);
    void setItemClass(QString folderClass);
    QString ItemClass();
    QByteArray type() const override;
    Attribute *clone() const override;
    QByteArray serialized() const override;
    void deserialize(const QByteArray &data) override;

private:
    QString mItemClass;

};

