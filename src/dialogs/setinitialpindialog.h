/* -*- mode: c++; c-basic-offset:4 -*-
    dialogs/setinitialpindialog.h

    This file is part of Kleopatra, the KDE keymanager
    SPDX-FileCopyrightText: 2009 Klarälvdalens Datakonsult AB

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QDialog>

#include <utils/pimpl_ptr.h>

namespace GpgME
{
class Error;
}

namespace Kleo
{
namespace Dialogs
{

class SetInitialPinDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SetInitialPinDialog(QWidget *parent = nullptr);
    ~SetInitialPinDialog();

    void setNksPinPresent(bool);
    void setSigGPinPresent(bool);

    bool isComplete() const;

public Q_SLOTS:
    void setNksPinSettingResult(const GpgME::Error &error);
    void setSigGPinSettingResult(const GpgME::Error &error);

Q_SIGNALS:
    void nksPinRequested();
    void sigGPinRequested();

private:
    class Private;
    kdtools::pimpl_ptr<Private> d;
    Q_PRIVATE_SLOT(d, void slotNksButtonClicked())
    Q_PRIVATE_SLOT(d, void slotSigGButtonClicked())
};

}
}

