/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/gui/resultitemwidget.h

    This file is part of Kleopatra, the KDE keymanager
    SPDX-FileCopyrightText: 2008 Klarälvdalens Datakonsult AB

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QWidget>

#include <crypto/task.h>

#include <utils/pimpl_ptr.h>

#include <memory>

class QString;

namespace Kleo
{
namespace Crypto
{

class Task;

namespace Gui
{

class ResultItemWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ResultItemWidget(const std::shared_ptr<const Task::Result> &result, QWidget *parent = nullptr, Qt::WindowFlags flags = {});
    ~ResultItemWidget();

    bool hasErrorResult() const;

    void showCloseButton(bool show);

public Q_SLOTS:
    void showAuditLog();

Q_SIGNALS:
    void linkActivated(const QString &link);
    void closeButtonClicked();

private:
    class Private;
    kdtools::pimpl_ptr<Private> d;
    Q_PRIVATE_SLOT(d, void slotLinkActivated(QString))
};
}
}
}

