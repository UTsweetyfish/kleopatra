/* -*- mode: c++; c-basic-offset:4 -*-
    commands/selftestcommand.cpp

    This file is part of Kleopatra, the KDE keymanager
    SPDX-FileCopyrightText: 2008 Klarälvdalens Datakonsult AB

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <config-kleopatra.h>

#include "selftestcommand.h"

#include "command_p.h"

#include <dialogs/selftestdialog.h>

#include "kleopatra_debug.h"

#ifdef Q_OS_WIN
# include <selftest/registrycheck.h>
#endif
#include <selftest/enginecheck.h>
#include <selftest/gpgconfcheck.h>
#ifdef HAVE_KLEOPATRACLIENT_LIBRARY
# include <selftest/uiservercheck.h>
#endif
#include <selftest/gpgagentcheck.h>
#include <selftest/libkleopatrarccheck.h>

#include <Libkleo/Stl_Util>

#include <KConfigGroup>
#include <KSharedConfig>

#include <vector>

#include <QGpgME/CryptoConfig>
#include <QGpgME/Protocol>

using namespace Kleo;
using namespace Kleo::Commands;
using namespace Kleo::Dialogs;

static const char *const components[] = {
    nullptr, // gpgconf
    "gpg",
    "gpg-agent",
    "scdaemon",
    "gpgsm",
    "dirmngr",
};
static const unsigned int numComponents = sizeof components / sizeof * components;

class SelfTestCommand::Private : Command::Private
{
    friend class ::Kleo::Commands::SelfTestCommand;
    SelfTestCommand *q_func() const
    {
        return static_cast<SelfTestCommand *>(q);
    }
public:
    explicit Private(SelfTestCommand *qq, KeyListController *c);
    ~Private();

private:
    void init();

    void ensureDialogCreated()
    {
        if (dialog) {
            return;
        }
        dialog = new SelfTestDialog;
        applyWindowID(dialog);
        dialog->setAttribute(Qt::WA_DeleteOnClose);

        connect(dialog, SIGNAL(updateRequested()),
                q_func(), SLOT(slotUpdateRequested()));
        connect(dialog, SIGNAL(accepted()),
                q_func(), SLOT(slotDialogAccepted()));
        connect(dialog, SIGNAL(rejected()),
                q_func(), SLOT(slotDialogRejected()));

        dialog->setRunAtStartUp(runAtStartUp());
        dialog->setAutomaticMode(automatic);
    }

    void ensureDialogShown()
    {
        ensureDialogCreated();
        if (dialog->isVisible()) {
            dialog->raise();
        } else {
            dialog->show();
        }
    }

    bool runAtStartUp() const
    {
        const KConfigGroup config(KSharedConfig::openConfig(), "Self-Test");
        return config.readEntry("run-at-startup", true);
    }

    void setRunAtStartUp(bool on)
    {
        KConfigGroup config(KSharedConfig::openConfig(), "Self-Test");
        config.writeEntry("run-at-startup", on);
    }

    void runTests()
    {
        std::vector< std::shared_ptr<Kleo::SelfTest> > tests;

#if defined(Q_OS_WIN)
        qCDebug(KLEOPATRA_LOG) << "Checking Windows Registry...";
        tests.push_back(makeGpgProgramRegistryCheckSelfTest());
#if defined(HAVE_KLEOPATRACLIENT_LIBRARY)
        qCDebug(KLEOPATRA_LOG) << "Checking Ui Server connectivity...";
        tests.push_back(makeUiServerConnectivitySelfTest());
#endif
#endif
        qCDebug(KLEOPATRA_LOG) << "Checking gpg installation...";
        tests.push_back(makeGpgEngineCheckSelfTest());
        qCDebug(KLEOPATRA_LOG) << "Checking gpgsm installation...";
        tests.push_back(makeGpgSmEngineCheckSelfTest());
        qCDebug(KLEOPATRA_LOG) << "Checking gpgconf installation...";
        tests.push_back(makeGpgConfEngineCheckSelfTest());
        for (unsigned int i = 0; i < numComponents; ++i) {
            qCDebug(KLEOPATRA_LOG) << "Checking configuration of:" << components[i];
            tests.push_back(makeGpgConfCheckConfigurationSelfTest(components[i]));
        }
#ifndef Q_OS_WIN
        tests.push_back(makeGpgAgentConnectivitySelfTest());
#endif
        tests.push_back(makeLibKleopatraRcSelfTest());

        if (!dialog && std::none_of(tests.cbegin(), tests.cend(),
                                    [](const std::shared_ptr<SelfTest> &test) {
                                        return test->failed();
                                    })) {
            finished();
            return;
        }

        ensureDialogCreated();

        dialog->clear();
        dialog->addSelfTests(tests);

        ensureDialogShown();
    }

private:
    void slotDialogAccepted()
    {
        setRunAtStartUp(dialog->runAtStartUp());
        finished();
    }
    void slotDialogRejected()
    {
        if (automatic) {
            canceled = true;
            Command::Private::canceled();
        } else {
            slotDialogAccepted();
        }
    }
    void slotUpdateRequested()
    {
        const auto conf = QGpgME::cryptoConfig();
        if (conf) {
            conf->clear();
        }
        runTests();
    }

private:
    QPointer<SelfTestDialog> dialog;
    bool canceled;
    bool automatic;
};

SelfTestCommand::Private *SelfTestCommand::d_func()
{
    return static_cast<Private *>(d.get());
}
const SelfTestCommand::Private *SelfTestCommand::d_func() const
{
    return static_cast<const Private *>(d.get());
}

#define d d_func()
#define q q_func()

SelfTestCommand::Private::Private(SelfTestCommand *qq, KeyListController *c)
    : Command::Private(qq, c),
      dialog(),
      canceled(false),
      automatic(false)
{

}

SelfTestCommand::Private::~Private()
{

}

SelfTestCommand::SelfTestCommand(KeyListController *c)
    : Command(new Private(this, c))
{
    d->init();
}

SelfTestCommand::SelfTestCommand(QAbstractItemView *v, KeyListController *c)
    : Command(v, new Private(this, c))
{
    d->init();
}

void SelfTestCommand::Private::init()
{

}

SelfTestCommand::~SelfTestCommand() {}

void SelfTestCommand::setAutomaticMode(bool on)
{
    d->automatic = on;
    if (d->dialog) {
        d->dialog->setAutomaticMode(on);
    }
}

bool SelfTestCommand::isCanceled() const
{
    return d->canceled;
}

void SelfTestCommand::doStart()
{

    if (d->automatic) {
        if (!d->runAtStartUp()) {
            d->finished();
            return;
        }
    } else {
        d->ensureDialogCreated();
    }

    d->runTests();

}

void SelfTestCommand::doCancel()
{
    d->canceled = true;
    if (d->dialog) {
        d->dialog->close();
    }
    d->dialog = nullptr;
}

#undef d
#undef q

#include "moc_selftestcommand.cpp"
