/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef EWSFETCHITEMTYPEDETAILJOB_H
#define EWSFETCHITEMTYPEDETAILJOB_H

#include "ewsfetchitemdetailjob.h"

class EwsFetchItemTypeDetailsJob : public EwsFetchItemDetailJob
{
    Q_OBJECT
public:
    EwsFetchItemTypeDetailsJob(EwsClient &client, QObject *parent, const Akonadi::Collection &collection);
    ~EwsFetchItemTypeDetailsJob() override;

protected:
    void processItems(const QList<EwsGetItemRequest::Response> &responses) override;
};

#endif
