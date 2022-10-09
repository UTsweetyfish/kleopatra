/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/gui/resultlistwidget.h

    This file is part of Kleopatra, the KDE keymanager
    SPDX-FileCopyrightText: 2008 Klarälvdalens Datakonsult AB

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QWidget>

#include <crypto/taskcollection.h>

#include <utils/pimpl_ptr.h>

#include <memory>

class QString;

namespace Kleo
{
namespace Crypto
{

class TaskCollection;

namespace Gui
{

class ResultListWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ResultListWidget(QWidget *parent = nullptr, Qt::WindowFlags flags = {});
    ~ResultListWidget();

    void setTaskCollection(const std::shared_ptr<TaskCollection> &coll);
    void addTaskCollection(const std::shared_ptr<TaskCollection> &coll);

    void setStandaloneMode(bool standalone);

    bool isComplete() const;

    unsigned int totalNumberOfTasks() const;
    unsigned int numberOfCompletedTasks() const;

Q_SIGNALS:
    void linkActivated(const QString &link);
    void completeChanged();

private:
    class Private;
    kdtools::pimpl_ptr<Private> d;
    Q_PRIVATE_SLOT(d, void result(std::shared_ptr<const Kleo::Crypto::Task::Result>))
    Q_PRIVATE_SLOT(d, void started(std::shared_ptr<Kleo::Crypto::Task>))
    Q_PRIVATE_SLOT(d, void allTasksDone())
};
}
}
}

