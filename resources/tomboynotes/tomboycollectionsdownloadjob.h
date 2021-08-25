/*
    SPDX-FileCopyrightText: 2016 Stefan Stäglich <sstaeglich@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TOMBOYCOLLECTIONSDOWNLOADJOB_H
#define TOMBOYCOLLECTIONSDOWNLOADJOB_H

#include "tomboyjobbase.h"
#include <AkonadiCore/Collection>

class TomboyCollectionsDownloadJob : public TomboyJobBase
{
    Q_OBJECT
public:
    // ctor
    explicit TomboyCollectionsDownloadJob(const QString &collectionName, KIO::AccessManager *manager, int refreshInterval, QObject *parent = nullptr);
    // returns the parsed results wrapped in Akonadi::Collection::List, see bellow
    Akonadi::Collection::List collections() const;

    // automatically called by KJob
    void start() override;

private:
    // This will be called once the request is finished.
    void onRequestFinished();
    Akonadi::Collection::List mResultCollections;
    const QString mCollectionName;
    const int mRefreshInterval;
};

#endif // TOMBOYCOLLECTIONSDOWNLOADJOB_H
