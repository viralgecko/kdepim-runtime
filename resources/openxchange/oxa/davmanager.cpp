/*
    This file is part of oxaccess.

    SPDX-FileCopyrightText: 2009 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "davmanager.h"

#include <kio/davjob.h>

#include <QDomDocument>

using namespace OXA;

DavManager *DavManager::mSelf = nullptr;

DavManager::DavManager()
{
}

DavManager::~DavManager()
{
}

DavManager *DavManager::self()
{
    if (!mSelf) {
        mSelf = new DavManager();
    }

    return mSelf;
}

void DavManager::setBaseUrl(const QUrl &url)
{
    mBaseUrl = url;
}

QUrl DavManager::baseUrl() const
{
    return mBaseUrl;
}

KIO::DavJob *DavManager::createFindJob(const QString &path, const QDomDocument &document) const
{
    QUrl url(mBaseUrl);
    url.setPath(path);

    return KIO::davPropFind(url, document, QStringLiteral("0"), KIO::HideProgressInfo);
}

KIO::DavJob *DavManager::createPatchJob(const QString &path, const QDomDocument &document) const
{
    QUrl url(mBaseUrl);
    url.setPath(path);

    return KIO::davPropPatch(url, document, KIO::HideProgressInfo);
}
