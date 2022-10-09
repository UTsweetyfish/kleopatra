/*
    main.cpp

    This file is part of Kleopatra, the KDE keymanager
    SPDX-FileCopyrightText: 2001, 2002, 2004 Klarälvdalens Datakonsult AB

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <config-kleopatra.h>

#include "aboutdata.h"
#include "kwatchgnupgmainwin.h"
#include <kcoreaddons_version.h>
#if KCOREADDONS_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <Kdelibs4ConfigMigrator>
#endif
#include "utils/kuniqueservice.h"

#include <KLocalizedString>
#include <KCrash>
#include "kwatchgnupg_debug.h"
#include <QCommandLineParser>
#include <QApplication>

int main(int argc, char **argv)
{
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
    QApplication app(argc, argv);
    KCrash::initialize();
#if KCOREADDONS_VERSION < QT_VERSION_CHECK(6, 0, 0)
    Kdelibs4ConfigMigrator migrate(QStringLiteral("kwatchgnupg"));
    migrate.setConfigFiles(QStringList() << QStringLiteral("kwatchgnupgrc"));
    migrate.setUiFiles(QStringList() << QStringLiteral("kwatchgnupgui.rc"));
    migrate.migrate();
#endif

    KLocalizedString::setApplicationDomain("kwatchgnupg");
    AboutData aboutData;

    KAboutData::setApplicationData(aboutData);
    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    KUniqueService service;

    auto mMainWin = new KWatchGnuPGMainWindow();
    mMainWin->show();
    return app.exec();
}
