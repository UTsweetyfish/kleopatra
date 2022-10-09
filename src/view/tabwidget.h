/* -*- mode: c++; c-basic-offset:4 -*-
    view/tabwidget.h

    This file is part of Kleopatra, the KDE keymanager
    SPDX-FileCopyrightText: 2007 Klarälvdalens Datakonsult AB

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QWidget>

#include <memory>
#include <vector>

#include <utils/pimpl_ptr.h>


class QAbstractItemView;

class KConfigGroup;
class KActionCollection;
class KConfig;

namespace Kleo
{

class AbstractKeyListModel;
class AbstractKeyListSortFilterProxyModel;
class KeyFilter;
class KeyListModelInterface;

class TabWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TabWidget(QWidget *parent = nullptr, Qt::WindowFlags f = {});
    ~TabWidget();

    void setFlatModel(AbstractKeyListModel *model);
    AbstractKeyListModel *flatModel() const;
    void setHierarchicalModel(AbstractKeyListModel *model);
    AbstractKeyListModel *hierarchicalModel() const;

    QAbstractItemView *addView(const QString &title = QString(), const QString &keyFilterID = QString(), const QString &searchString = QString());
    QAbstractItemView *addView(const KConfigGroup &group);
    QAbstractItemView *addTemporaryView(const QString &title = QString(), AbstractKeyListSortFilterProxyModel *proxy = nullptr, const QString &tabToolTip = QString());

    void loadViews(const KConfig *cfg);
    void saveViews(KConfig *cfg) const;

    std::vector<QAbstractItemView *> views() const;
    QAbstractItemView *currentView() const;
    KeyListModelInterface *currentModel() const;

    unsigned int count() const;

    void createActions(KActionCollection *collection);
    void connectSearchBar(QObject *sb);

    void setMultiSelection(bool on);

public Q_SLOTS:
    void setKeyFilter(const std::shared_ptr<Kleo::KeyFilter> &filter);
    void setStringFilter(const QString &filter);

Q_SIGNALS:
    void viewAdded(QAbstractItemView *view);
    void viewAboutToBeRemoved(QAbstractItemView *view);

    void currentViewChanged(QAbstractItemView *view);
    void stringFilterChanged(const QString &filter);
    void keyFilterChanged(const std::shared_ptr<Kleo::KeyFilter> &filter);

    void enableChangeStringFilter(bool enable);
    void enableChangeKeyFilter(bool enable);

private:
    class Private;
    kdtools::pimpl_ptr<Private> d;

    Q_PRIVATE_SLOT(d, void currentIndexChanged(int))
    Q_PRIVATE_SLOT(d, void slotPageTitleChanged(const QString &))
    Q_PRIVATE_SLOT(d, void slotPageKeyFilterChanged(const std::shared_ptr<Kleo::KeyFilter> &))
    Q_PRIVATE_SLOT(d, void slotPageStringFilterChanged(const QString &))
    Q_PRIVATE_SLOT(d, void slotPageHierarchyChanged(bool))
    Q_PRIVATE_SLOT(d, void slotRenameCurrentTab())
    Q_PRIVATE_SLOT(d, void slotNewTab())
    Q_PRIVATE_SLOT(d, void slotDuplicateCurrentTab())
    Q_PRIVATE_SLOT(d, void slotCloseCurrentTab())
    Q_PRIVATE_SLOT(d, void slotMoveCurrentTabLeft())
    Q_PRIVATE_SLOT(d, void slotMoveCurrentTabRight())
    Q_PRIVATE_SLOT(d, void slotToggleHierarchicalView(bool))
    Q_PRIVATE_SLOT(d, void slotExpandAll())
    Q_PRIVATE_SLOT(d, void slotCollapseAll())
};

}

