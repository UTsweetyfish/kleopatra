/* -*- mode: c++; c-basic-offset:4 -*-
    utils/filedialog.cpp

    This file is part of Kleopatra, the KDE keymanager
    SPDX-FileCopyrightText: 2009 Klarälvdalens Datakonsult AB

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <config-kleopatra.h>

#include "filedialog.h"

#include <QFileDialog>
#include <QDir>
#include <QMap>

using namespace Kleo;

namespace
{
using Map = QMap<QString, QString>;
Q_GLOBAL_STATIC(Map, dir_id_2_dir_map)
}

static QString dir(const QString &id)
{
    const QString dir = (*dir_id_2_dir_map())[id];
    if (dir.isEmpty()) {
        return QDir::homePath();
    } else {
        return dir;
    }
}

static void update(const QString &fname, const QString &id)
{
    if (!fname.isEmpty()) {
        (*dir_id_2_dir_map())[ id ] = QFileInfo(fname).absolutePath();
    }
}

QString FileDialog::getOpenFileName(QWidget *parent, const QString &caption, const QString &dirID, const QString &filter)
{
    const QString fname = QFileDialog::getOpenFileName(parent, caption, dir(dirID), filter);
    update(fname, dirID);
    return fname;
}

QStringList FileDialog::getOpenFileNames(QWidget *parent, const QString &caption, const QString &dirID, const QString &filter)
{
    const QStringList files = QFileDialog::getOpenFileNames(parent, caption, dir(dirID), filter);
    if (!files.empty()) {
        update(files.front(), dirID);
    }
    return files;
}

QString FileDialog::getSaveFileName(QWidget *parent, const QString &caption, const QString &dirID, const QString &filter)
{
    const QString fname = QFileDialog::getSaveFileName(parent, caption, dir(dirID), filter);
    update(fname, dirID);
    return fname;
}

QString FileDialog::getSaveFileNameEx(QWidget *parent, const QString &caption, const QString &dirID, const QString &proposedFileName, const QString &filter)
{
    if (proposedFileName.isEmpty()) {
        return getSaveFileName(parent, caption, dirID, filter);
    }
    const QString fname = QFileDialog::getSaveFileName(parent, caption, QDir(dir(dirID)).filePath(proposedFileName), filter);
    update(fname, dirID);
    return fname;
}
