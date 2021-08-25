/*
    SPDX-FileCopyrightText: 2021 Andreas Merl <merl.andreas@outlook.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef EWSFOLDERCLASSATTRIBUTE_H
#define EWSFOLDERCLASSATTRIBUTE_H

#include <attribute.h>

class ewsFolderClassAttribute : public Akonadi::Attribute
{
public:
    explicit ewsFolderClassAttribute(QString folderClass);
    void setFolderClass(QString folderClass);
    QString FolderClass();
    QByteArray type() const override;
    Attribute *clone() const override;
    QByteArray serialized() const override;
    void deserialize(const QByteArray &data) override;

private:
    QString mFolderClass;

};

#endif
