/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef EWSMODIFYITEMTYPEJOB_H
#define EWSMODIFYITEMTYPEJOB_H

#include "ewsmodifyitemjob.h"

class EwsModifyItemTypeJob : public EwsModifyItemJob
{
    Q_OBJECT
public:
    EwsModifyItemTypeJob(EwsClient &client, const Akonadi::Item::List &items, const QSet<QByteArray> &parts, QObject *parent);
    ~EwsModifyItemTypeJob() override;
    void start() override;
};

#endif
