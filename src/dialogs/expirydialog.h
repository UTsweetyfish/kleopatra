/* -*- mode: c++; c-basic-offset:4 -*-
    dialogs/expirydialog.h

    This file is part of Kleopatra, the KDE keymanager
    SPDX-FileCopyrightText: 2008 Klarälvdalens Datakonsult AB
    SPDX-FileCopyrightText: 2021 g10 Code GmbH
    SPDX-FileContributor: Ingo Klöcker <dev@ingo-kloecker.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QDialog>

#include <memory.h>

class QDate;

namespace Kleo
{
namespace Dialogs
{

class ExpiryDialog : public QDialog
{
    Q_OBJECT
public:
    enum class Mode
    {
        UpdateCertificateWithSubkeys,
        UpdateCertificateWithoutSubkeys,
        UpdateIndividualSubkey,
    };

    explicit ExpiryDialog(Mode mode, QWidget *parent = nullptr);
    ~ExpiryDialog() override;

    void setDateOfExpiry(const QDate &date);
    QDate dateOfExpiry() const;

    void setUpdateExpirationOfAllSubkeys(bool update);
    bool updateExpirationOfAllSubkeys() const;

private:
    class Private;
    std::unique_ptr<Private> d;
};

}
}

