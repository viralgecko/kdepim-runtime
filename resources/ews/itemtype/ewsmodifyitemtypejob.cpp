/*
    SPDX-FileCopyrightText: 2015-2016 Krzysztof Nowicki <krissn@op.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ewsmodifyitemtypejob.h"

#include "ewsresource_debug.h"

EwsModifyItemTypeJob::EwsModifyItemTypeJob(EwsClient &client, const Akonadi::Item::List &items, const QSet<QByteArray> &parts, QObject *parent)
    : EwsModifyItemJob(client, items, parts, parent)
{
}

EwsModifyItemTypeJob::~EwsModifyItemTypeJob()
{
}

void EwsModifyItemTypeJob::start()
{
    qCWarning(EWSRES_LOG) << QStringLiteral("Abch person item modification not implemented!");
    emitResult();
}
