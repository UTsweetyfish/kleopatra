/* -*- mode: c++; c-basic-offset:4 -*-
    commands/revokecertificationcommand.cpp

    This file is part of Kleopatra, the KDE keymanager
    SPDX-FileCopyrightText: 2020 g10 Code GmbH
    SPDX-FileContributor: Ingo Klöcker <dev@ingo-kloecker.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <config-kleopatra.h>

#include "revokecertificationcommand.h"

#include "command_p.h"

#include "exportopenpgpcertstoservercommand.h"
#include "dialogs/revokecertificationdialog.h"

#include <Libkleo/Formatting>
#include <Libkleo/KeyCache>

#include <QGpgME/Protocol>
#include <QGpgME/QuickJob>

#include <gpgme++/engineinfo.h>

#include <KLocalizedString>

#include "kleopatra_debug.h"

#include <gpgme++/gpgmepp_version.h>
#if GPGMEPP_VERSION >= 0x10E01 // 1.14.1
# define GPGME_HAS_REVSIG
#endif

using namespace Kleo;
using namespace Kleo::Commands;
using namespace GpgME;
using namespace QGpgME;

class RevokeCertificationCommand::Private : public Command::Private
{
    friend class ::Kleo::Commands::RevokeCertificationCommand;
    RevokeCertificationCommand *q_func() const
    {
        return static_cast<RevokeCertificationCommand *>(q);
    }
public:
    explicit Private(RevokeCertificationCommand *qq, KeyListController *c);
    ~Private();

    void init();

private:
    void slotDialogAccepted();
    void slotDialogRejected();
    void slotResult(const Error &err);

private:
    void ensureDialogCreated();
    void createJob();

private:
    Key certificationKey;
    Key certificationTarget;
    std::vector<UserID> uids;
    QPointer<RevokeCertificationDialog> dialog;
    QPointer<QGpgME::QuickJob> job;
};

RevokeCertificationCommand::Private *RevokeCertificationCommand::d_func()
{
    return static_cast<Private *>(d.get());
}
const RevokeCertificationCommand::Private *RevokeCertificationCommand::d_func() const
{
    return static_cast<const Private *>(d.get());
}

#define d d_func()
#define q q_func()

RevokeCertificationCommand::Private::Private(RevokeCertificationCommand *qq, KeyListController *c)
    : Command::Private(qq, c)
{
}

RevokeCertificationCommand::Private::~Private()
{
}

void RevokeCertificationCommand::Private::init()
{
    const std::vector<Key> keys_ = keys();
    if (keys_.size() != 1) {
        qCWarning(KLEOPATRA_LOG) << "RevokeCertificationCommand::Private::init: Expected exactly one key, but got" << keys_.size();
        return;
    }
    if (keys_.front().protocol() != GpgME::OpenPGP) {
        qCWarning(KLEOPATRA_LOG) << "RevokeCertificationCommand::Private::init: Expected OpenPGP key, but got" << keys_.front().protocolAsString();
        return;
    }
    certificationTarget = keys_.front();
}

void RevokeCertificationCommand::Private::slotDialogAccepted()
{
    createJob();

#ifdef GPGME_HAS_REVSIG
    job->startRevokeSignature(certificationTarget, dialog->selectedCertificationKey(), dialog->selectedUserIDs());
#endif
}

void RevokeCertificationCommand::Private::slotDialogRejected()
{
    canceled();
}

void RevokeCertificationCommand::Private::slotResult(const Error &err)
{
    if (err.isCanceled()) {
        // do nothing
    } else if (err) {
        error(i18n("<p>An error occurred while trying to revoke the certification of<br/><br/>"
                   "<b>%1</b>:</p><p>\t%2</p>",
                   Formatting::formatForComboBox(certificationTarget),
                   QString::fromUtf8(err.asString())),
              i18n("Revocation Error"));
    } else {
        information(i18n("Revocation successful."),
                    i18n("Revocation Succeeded"));
        if (dialog && dialog->sendToServer()) {
            auto const cmd = new ExportOpenPGPCertsToServerCommand(certificationTarget);
            cmd->start();
        }
    }

    finished();
}

void RevokeCertificationCommand::Private::ensureDialogCreated()
{
    if (dialog) {
        return;
    }

    dialog = new RevokeCertificationDialog;
    applyWindowID(dialog);
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    connect(dialog, SIGNAL(accepted()), q, SLOT(slotDialogAccepted()));
    connect(dialog, SIGNAL(rejected()), q, SLOT(slotDialogRejected()));
}

void RevokeCertificationCommand::Private::createJob()
{
    Q_ASSERT(!job);

    Q_ASSERT(certificationTarget.protocol() == OpenPGP);
    const auto backend = QGpgME::openpgp();
    if (!backend) {
        return;
    }

    QuickJob *const j = backend->quickJob();
    if (!j) {
        return;
    }

    connect(j, &Job::progress,
            q, &Command::progress);
    connect(j, SIGNAL(result(GpgME::Error)),
            q, SLOT(slotResult(GpgME::Error)));

    job = j;
}

RevokeCertificationCommand::RevokeCertificationCommand(QAbstractItemView *v, KeyListController *c)
    : Command(v, new Private(this, c))
{
    d->init();
}

RevokeCertificationCommand::RevokeCertificationCommand(const GpgME::UserID &uid)
    : Command(uid.parent(), new Private(this, nullptr))
{
    std::vector<UserID>(1, uid).swap(d->uids);
    d->init();
}

RevokeCertificationCommand::RevokeCertificationCommand(const GpgME::UserID::Signature &signature)
    : Command(signature.parent().parent(), new Private(this, nullptr))
{
    std::vector<UserID>(1, signature.parent()).swap(d->uids);
    d->certificationKey = KeyCache::instance()->findByKeyIDOrFingerprint(signature.signerKeyID());
    d->init();
}

RevokeCertificationCommand::~RevokeCertificationCommand()
{
    qCDebug(KLEOPATRA_LOG) << "~RevokeCertificationCommand()";
}

// static
bool RevokeCertificationCommand::isSupported()
{
#ifdef GPGME_HAS_REVSIG
    return engineInfo(GpgEngine).engineVersion() >= "2.2.24";
#else
    return false;
#endif
}

void RevokeCertificationCommand::doStart()
{
    if (d->certificationTarget.isNull()) {
        d->finished();
        return;
    }

    for (const UserID &uid : std::as_const(d->uids))
        if (qstricmp(uid.parent().primaryFingerprint(), d->certificationTarget.primaryFingerprint()) != 0) {
            qCWarning(KLEOPATRA_LOG) << "User-ID <-> Key mismatch!";
            d->finished();
            return;
        }

    d->ensureDialogCreated();
    Q_ASSERT(d->dialog);

    d->dialog->setCertificateToRevoke(d->certificationTarget);
    if (!d->uids.empty()) {
        d->dialog->setSelectedUserIDs(d->uids);
    }
    if (!d->certificationKey.isNull()) {
        d->dialog->setSelectedCertificationKey(d->certificationKey);
    }
    d->dialog->show();
}

void RevokeCertificationCommand::doCancel()
{
    qCDebug(KLEOPATRA_LOG) << "RevokeCertificationCommand::doCancel()";
    if (d->job) {
        d->job->slotCancel();
    }
}

#undef d
#undef q

#include "moc_revokecertificationcommand.cpp"
