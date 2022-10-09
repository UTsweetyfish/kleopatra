/*
    kwatchgnupgconfig.cpp

    This file is part of Kleopatra, the KDE keymanager
    SPDX-FileCopyrightText: 2001, 2002, 2004 Klarälvdalens Datakonsult AB

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <config-kleopatra.h>

#include "kwatchgnupgconfig.h"
#include "kwatchgnupg.h"

#include <Libkleo/FileNameRequester>

#include <KLocalizedString>
#include <KConfig>
#include <KPluralHandlingSpinBox>

#include <QVBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QGroupBox>
#include <KSharedConfig>
#include <KConfigGroup>
#include <QDialogButtonBox>

static const char *log_levels[] = { "none", "basic", "advanced", "expert", "guru" };

static int log_level_to_int(const QString &loglevel)
{
    if (loglevel == QLatin1String("none")) {
        return 0;
    } else if (loglevel == QLatin1String("basic")) {
        return 1;
    } else if (loglevel == QLatin1String("advanced")) {
        return 2;
    } else if (loglevel == QLatin1String("expert")) {
        return 3;
    } else if (loglevel == QLatin1String("guru")) {
        return 4;
    } else {
        // default
        return 1;
    }
}

KWatchGnuPGConfig::KWatchGnuPGConfig(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18nc("@title:window", "Configure KWatchGnuPG"));
    auto mainLayout = new QVBoxLayout(this);

    mButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    QPushButton *okButton = mButtonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(mButtonBox, &QDialogButtonBox::rejected, this, &KWatchGnuPGConfig::reject);

    auto top = new QWidget;
    mainLayout->addWidget(top);
    mainLayout->addWidget(mButtonBox);

    auto vlay = new QVBoxLayout(top);
    vlay->setContentsMargins(0, 0, 0, 0);

    auto group = new QGroupBox(i18n("WatchGnuPG"), top);
    vlay->addWidget(group);

    auto glay = new QGridLayout(group);
    glay->setColumnStretch(1, 1);

    int row = -1;

    ++row;
    mExeED = new Kleo::FileNameRequester(group);
    auto label = new QLabel(i18n("&Executable:"), group);
    label->setBuddy(mExeED);
    glay->addWidget(label, row, 0);
    glay->addWidget(mExeED, row, 1);

    connect(mExeED, &Kleo::FileNameRequester::fileNameChanged, this, &KWatchGnuPGConfig::slotChanged);

    ++row;
    mSocketED = new Kleo::FileNameRequester(group);
    label = new QLabel(i18n("&Socket:"), group);
    label->setBuddy(mSocketED);
    glay->addWidget(label, row, 0);
    glay->addWidget(mSocketED, row, 1);

    connect(mSocketED, &Kleo::FileNameRequester::fileNameChanged, this, &KWatchGnuPGConfig::slotChanged);

    ++row;
    mLogLevelCB = new QComboBox(group);
    mLogLevelCB->addItem(i18n("None"));
    mLogLevelCB->addItem(i18n("Basic"));
    mLogLevelCB->addItem(i18n("Advanced"));
    mLogLevelCB->addItem(i18n("Expert"));
    mLogLevelCB->addItem(i18n("Guru"));
    label = new QLabel(i18n("Default &log level:"), group);
    label->setBuddy(mLogLevelCB);
    glay->addWidget(label, row, 0);
    glay->addWidget(mLogLevelCB, row, 1);

    connect(mLogLevelCB, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &KWatchGnuPGConfig::slotChanged);

    /******************* Log Window group *******************/
    group = new QGroupBox(i18n("Log Window"), top);
    vlay->addWidget(group);

    glay = new QGridLayout(group);
    glay->setColumnStretch(1, 1);

    row = -1;

    ++row;
    mLoglenSB = new KPluralHandlingSpinBox(group);
    mLoglenSB->setRange(0, 1000000);
    mLoglenSB->setSingleStep(100);
    mLoglenSB->setSuffix(ki18ncp("history size spinbox suffix", " line", " lines"));
    mLoglenSB->setSpecialValueText(i18n("unlimited"));
    label = new QLabel(i18n("&History size:"), group);
    label->setBuddy(mLoglenSB);
    glay->addWidget(label, row, 0);
    glay->addWidget(mLoglenSB, row, 1);
    auto button = new QPushButton(i18n("Set &Unlimited"), group);
    glay->addWidget(button, row, 2);

    connect(mLoglenSB, static_cast<void (KPluralHandlingSpinBox::*)(int)>(&KPluralHandlingSpinBox::valueChanged), this, &KWatchGnuPGConfig::slotChanged);
    connect(button, &QPushButton::clicked, this, &KWatchGnuPGConfig::slotSetHistorySizeUnlimited);

    ++row;
    mWordWrapCB = new QCheckBox(i18n("Enable &word wrapping"), group);
    mWordWrapCB->hide(); // QTextEdit doesn't support word wrapping in LogText mode
    glay->addWidget(mWordWrapCB, row, 0, 1, 3);

    connect(mWordWrapCB, &QCheckBox::clicked, this, &KWatchGnuPGConfig::slotChanged);

    vlay->addStretch(1);

    connect(okButton, &QPushButton::clicked, this, &KWatchGnuPGConfig::slotSave);
}

KWatchGnuPGConfig::~KWatchGnuPGConfig() {}

void KWatchGnuPGConfig::slotSetHistorySizeUnlimited()
{
    mLoglenSB->setValue(0);
}

void KWatchGnuPGConfig::loadConfig()
{
    const KConfigGroup watchGnuPG(KSharedConfig::openConfig(), "WatchGnuPG");
    mExeED->setFileName(watchGnuPG.readEntry("Executable", WATCHGNUPGBINARY));
    mSocketED->setFileName(watchGnuPG.readEntry("Socket", WATCHGNUPGSOCKET));
    mLogLevelCB->setCurrentIndex(log_level_to_int(watchGnuPG.readEntry("LogLevel", "basic")));

    const KConfigGroup logWindow(KSharedConfig::openConfig(), "LogWindow");
    mLoglenSB->setValue(logWindow.readEntry("MaxLogLen", 10000));
    mWordWrapCB->setChecked(logWindow.readEntry("WordWrap", false));

    mButtonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

void KWatchGnuPGConfig::saveConfig()
{
    KConfigGroup watchGnuPG(KSharedConfig::openConfig(), "WatchGnuPG");
    watchGnuPG.writeEntry("Executable", mExeED->fileName());
    watchGnuPG.writeEntry("Socket", mSocketED->fileName());
    watchGnuPG.writeEntry("LogLevel", log_levels[mLogLevelCB->currentIndex()]);

    KConfigGroup logWindow(KSharedConfig::openConfig(), "LogWindow");
    logWindow.writeEntry("MaxLogLen", mLoglenSB->value());
    logWindow.writeEntry("WordWrap", mWordWrapCB->isChecked());

    KSharedConfig::openConfig()->sync();

    mButtonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

void KWatchGnuPGConfig::slotChanged()
{
    mButtonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void KWatchGnuPGConfig::slotSave()
{
    saveConfig();
    Q_EMIT reconfigure();
    accept();
}

