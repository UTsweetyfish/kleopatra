/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/taskcollection.h

    This file is part of Kleopatra, the KDE keymanager
    SPDX-FileCopyrightText: 2008 Klarälvdalens Datakonsult AB

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>

#include <crypto/task.h>

#include <utils/pimpl_ptr.h>

#include <memory>

#include <vector>

namespace Kleo
{
namespace Crypto
{

class TaskCollection : public QObject
{
    Q_OBJECT
public:
    explicit TaskCollection(QObject *parent = nullptr);
    ~TaskCollection();

    std::vector<std::shared_ptr<Task> > tasks() const;
    std::shared_ptr<Task> taskById(int id) const;

    void setTasks(const std::vector<std::shared_ptr<Task> > &tasks);

    bool isEmpty() const;
    size_t size() const;

    int numberOfCompletedTasks() const;
    bool allTasksCompleted() const;
    bool errorOccurred() const;
    bool allTasksHaveErrors() const;

Q_SIGNALS:
    void progress(const QString &msg, int processed, int total);
    void result(const std::shared_ptr<const Kleo::Crypto::Task::Result> &result);
    void started(const std::shared_ptr<Kleo::Crypto::Task> &task);
    void done();

private:
    class Private;
    kdtools::pimpl_ptr<Private> d;
    Q_PRIVATE_SLOT(d, void taskProgress(QString, int, int))
    Q_PRIVATE_SLOT(d, void taskResult(std::shared_ptr<const Kleo::Crypto::Task::Result>))
    Q_PRIVATE_SLOT(d, void taskStarted())
};
}
}

