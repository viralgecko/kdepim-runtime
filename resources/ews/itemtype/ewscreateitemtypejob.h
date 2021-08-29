/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "ewscreateitemjob.h"

class EwsCreateItemTypeJob : public EwsCreateItemJob
{
    Q_OBJECT
public:
    EwsCreateItemTypeJob(EwsClient &client, const Akonadi::Item &item, const Akonadi::Collection &collection, EwsTagStore *tagStore, EwsResource *parent);
    ~EwsCreateItemTypeJob() override;
    bool setSend(bool send = true) override;

protected:
    void doStart() override;
};
