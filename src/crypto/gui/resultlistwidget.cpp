/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/gui/resultlistwidget.cpp

    This file is part of Kleopatra, the KDE keymanager
    SPDX-FileCopyrightText: 2008 Klarälvdalens Datakonsult AB

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <config-kleopatra.h>

#include "resultlistwidget.h"

#include "emailoperationspreferences.h"

#include <crypto/gui/resultitemwidget.h>

#include <utils/scrollarea.h>

#include <Libkleo/Stl_Util>

#include <KLocalizedString>
#include <QPushButton>
#include <KStandardGuiItem>

#include <QLabel>
#include <QVBoxLayout>

#include <KGuiItem>

using namespace Kleo;
using namespace Kleo::Crypto;
using namespace Kleo::Crypto::Gui;

class ResultListWidget::Private
{
    ResultListWidget *const q;
public:
    explicit Private(ResultListWidget *qq);

    void result(const std::shared_ptr<const Task::Result> &result);
    void started(const std::shared_ptr<Task> &task);
    void allTasksDone();

    void addResultWidget(ResultItemWidget *widget);
    void setupSingle();
    void setupMulti();
    void resizeIfStandalone();

    std::vector< std::shared_ptr<TaskCollection> > m_collections;
    bool m_standaloneMode = false;
    int m_lastErrorItemIndex = 0;
    ScrollArea *m_scrollArea = nullptr;
    QPushButton *m_closeButton = nullptr;
    QVBoxLayout *m_layout = nullptr;
    QLabel *m_progressLabel = nullptr;
};

ResultListWidget::Private::Private(ResultListWidget *qq)
    : q(qq),
      m_collections()
{
    m_layout = new QVBoxLayout(q);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    m_progressLabel = new QLabel;
    m_progressLabel->setWordWrap(true);
    m_layout->addWidget(m_progressLabel);
    m_progressLabel->setVisible(false);

    m_closeButton = new QPushButton;
    KGuiItem::assign(m_closeButton, KStandardGuiItem::close());
    q->connect(m_closeButton, &QPushButton::clicked, q, &ResultListWidget::close);
    m_layout->addWidget(m_closeButton);
    m_closeButton->setVisible(false);
}

ResultListWidget::ResultListWidget(QWidget *parent, Qt::WindowFlags f) : QWidget(parent, f), d(new Private(this))
{
}

ResultListWidget::~ResultListWidget()
{
    if (!d->m_standaloneMode) {
        return;
    }
    EMailOperationsPreferences prefs;
    prefs.setDecryptVerifyPopupGeometry(geometry());
    prefs.save();
}

void ResultListWidget::Private::setupSingle()
{
    m_layout->addStretch();
}

void ResultListWidget::Private::resizeIfStandalone()
{
    if (m_standaloneMode) {
        q->resize(q->size().expandedTo(q->sizeHint()));
    }
}

void ResultListWidget::Private::setupMulti()
{
    if (m_scrollArea) {
        return;    // already been here...
    }

    m_scrollArea = new ScrollArea;
    m_scrollArea->setFocusPolicy(Qt::NoFocus);
    Q_ASSERT(qobject_cast<QBoxLayout *>(m_scrollArea->widget()->layout()));
    static_cast<QBoxLayout *>(m_scrollArea->widget()->layout())->setContentsMargins(0, 0, 0, 0);
    static_cast<QBoxLayout *>(m_scrollArea->widget()->layout())->setSpacing(2);
    static_cast<QBoxLayout *>(m_scrollArea->widget()->layout())->addStretch();
    m_layout->insertWidget(0, m_scrollArea);
}

void ResultListWidget::Private::addResultWidget(ResultItemWidget *widget)
{
    Q_ASSERT(widget);
    Q_ASSERT(std::any_of(m_collections.cbegin(), m_collections.cend(),
                       [](const std::shared_ptr<TaskCollection> &t) { return !t->isEmpty(); }));

    Q_ASSERT(m_scrollArea);
    Q_ASSERT(m_scrollArea->widget());
    Q_ASSERT(qobject_cast<QBoxLayout *>(m_scrollArea->widget()->layout()));
    QBoxLayout &blay = *static_cast<QBoxLayout *>(m_scrollArea->widget()->layout());
    blay.insertWidget(widget->hasErrorResult() ? m_lastErrorItemIndex++ : (blay.count() - 1), widget);

    widget->show();
    resizeIfStandalone();
}

void ResultListWidget::Private::allTasksDone()
{
    if (!q->isComplete()) {
        return;
    }
    m_progressLabel->setVisible(false);
    resizeIfStandalone();
    Q_EMIT q->completeChanged();
}

void ResultListWidget::Private::result(const std::shared_ptr<const Task::Result> &result)
{
    Q_ASSERT(result);
    Q_ASSERT(std::any_of(m_collections.cbegin(), m_collections.cend(),
                       [](const std::shared_ptr<TaskCollection> &t) { return !t->isEmpty(); }));
    auto wid = new ResultItemWidget(result);
    q->connect(wid, &ResultItemWidget::linkActivated, q, &ResultListWidget::linkActivated);
    q->connect(wid, &ResultItemWidget::closeButtonClicked, q, &ResultListWidget::close);
    addResultWidget(wid);
}

bool ResultListWidget::isComplete() const
{
    return std::all_of(d->m_collections.cbegin(), d->m_collections.cend(),
                       std::mem_fn(&TaskCollection::allTasksCompleted));
}

unsigned int ResultListWidget::totalNumberOfTasks() const
{
    return kdtools::accumulate_transform(d->m_collections.cbegin(),
                                         d->m_collections.cend(),
                                         std::mem_fn(&TaskCollection::size), 0U);
}

unsigned int ResultListWidget::numberOfCompletedTasks() const
{
    return kdtools::accumulate_transform(d->m_collections.cbegin(), d->m_collections.cend(),
                                         std::mem_fn(&TaskCollection::numberOfCompletedTasks), 0U);
}

void ResultListWidget::setTaskCollection(const std::shared_ptr<TaskCollection> &coll)
{
    //clear(); ### PENDING(marc) implement
    addTaskCollection(coll);
}

void ResultListWidget::addTaskCollection(const std::shared_ptr<TaskCollection> &coll)
{
    Q_ASSERT(coll); Q_ASSERT(!coll->isEmpty());
    d->m_collections.push_back(coll);
    connect(coll.get(), SIGNAL(result(std::shared_ptr<const Kleo::Crypto::Task::Result>)),
            this, SLOT(result(std::shared_ptr<const Kleo::Crypto::Task::Result>)));
    connect(coll.get(), SIGNAL(started(std::shared_ptr<Kleo::Crypto::Task>)),
            this, SLOT(started(std::shared_ptr<Kleo::Crypto::Task>)));
    connect(coll.get(), SIGNAL(done()), this, SLOT(allTasksDone()));
    d->setupMulti();
    setStandaloneMode(d->m_standaloneMode);
}

void ResultListWidget::Private::started(const std::shared_ptr<Task> &task)
{
    Q_ASSERT(task);
    Q_ASSERT(m_progressLabel);
    m_progressLabel->setText(i18nc("number, operation description", "Operation %1: %2", q->numberOfCompletedTasks() + 1, task->label()));
    resizeIfStandalone();
}

void ResultListWidget::setStandaloneMode(bool standalone)
{
    d->m_standaloneMode = standalone;
    if (totalNumberOfTasks() == 0) {
        return;
    }
    d->m_closeButton->setVisible(standalone);
    d->m_progressLabel->setVisible(standalone);
}

#include "moc_resultlistwidget.cpp"
