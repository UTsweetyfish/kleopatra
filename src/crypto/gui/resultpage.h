/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/gui/resultpage.h

    This file is part of Kleopatra, the KDE keymanager
    SPDX-FileCopyrightText: 2008 Klarälvdalens Datakonsult AB

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <crypto/gui/wizardpage.h>
#include <crypto/task.h>

#include <utils/pimpl_ptr.h>

#include <memory>

namespace Kleo
{
namespace Crypto
{

class TaskCollection;

namespace Gui
{

class ResultPage : public WizardPage
{
    Q_OBJECT

public:
    explicit ResultPage(QWidget *parent = nullptr, Qt::WindowFlags flags = {});
    ~ResultPage() override;

    void setTaskCollection(const std::shared_ptr<TaskCollection> &coll);

    bool isComplete() const override;

    bool keepOpenWhenDone() const;
    void setKeepOpenWhenDone(bool keep);

private:
    class Private;
    kdtools::pimpl_ptr<Private> d;
    Q_PRIVATE_SLOT(d, void progress(QString, int, int))
    Q_PRIVATE_SLOT(d, void result(std::shared_ptr<const Kleo::Crypto::Task::Result>))
    Q_PRIVATE_SLOT(d, void started(std::shared_ptr<Kleo::Crypto::Task>))
    Q_PRIVATE_SLOT(d, void keepOpenWhenDone(bool))
    Q_PRIVATE_SLOT(d, void allDone())
};

}
}
}

