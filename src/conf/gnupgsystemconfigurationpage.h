/* -*- mode: c++; c-basic-offset:4 -*-
    conf/gnupgsystemconfigurationpage.h

    This file is part of Kleopatra, the KDE keymanager
    SPDX-FileCopyrightText: 2008 Klarälvdalens Datakonsult AB

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once

#include <KCModule>
namespace Kleo
{
class CryptoConfigModule;
}

namespace Kleo
{
namespace Config
{

class GnuPGSystemConfigurationPage : public KCModule
{
    Q_OBJECT
public:
    explicit GnuPGSystemConfigurationPage(QWidget *parent = nullptr, const QVariantList &args = QVariantList());
    ~GnuPGSystemConfigurationPage() override;

    void load() override;
    void save() override;
    void defaults() override;

private:
    Kleo::CryptoConfigModule *mWidget;
};

}
}

