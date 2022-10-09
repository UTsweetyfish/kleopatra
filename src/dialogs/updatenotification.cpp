/*  dialogs/updatenotification.cpp

    This file is part of Kleopatra, the KDE keymanager
    SPDX-FileCopyrightText: 2017 Bundesamt für Sicherheit in der Informationstechnik
    SPDX-FileContributor: Intevation GmbH

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "updatenotification.h"

#include <Libkleo/Compat>
#include <Libkleo/GnuPG>

#include "kleopatra_debug.h"

#include <QIcon>
#include <QGridLayout>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QDesktopServices>
#include <QProcess>
#include <QProgressDialog>
#include <QPushButton>
#include <QLabel>
#include <QUrl>
#include <QDateTime>

#include <KIconLoader>
#include <KLocalizedString>
#include <KConfigGroup>
#include <KSharedConfig>
#include <KMessageBox>

#include <QGpgME/CryptoConfig>
#include <QGpgME/Protocol>

#include <gpgme++/gpgmefw.h>
#include <gpgme++/swdbresult.h>
#include <gpgme++/error.h>

using namespace Kleo;

namespace
{
static void gpgconf_set_update_check(bool value)
{
    auto conf = QGpgME::cryptoConfig();
    auto entry = getCryptoConfigEntry(conf, "dirmngr", "allow-version-check");
    if (!entry) {
        qCDebug(KLEOPATRA_LOG) << "allow-version-check entry not found";
        return;
    }
    if (entry->boolValue() != value) {
        entry->setBoolValue(value);
        conf->sync(true);
    }
}
} // namespace

void UpdateNotification::forceUpdateCheck(QWidget *parent)
{
    auto proc = new QProcess;

    proc->setProgram(gnupgInstallPath() + QStringLiteral("/gpg-connect-agent.exe"));
    proc->setArguments(QStringList() << QStringLiteral("--dirmngr")
                       << QStringLiteral("loadswdb --force")
                       << QStringLiteral("/bye"));

    auto progress = new QProgressDialog(i18n("Searching for updates..."),
                                        i18n("Cancel"), 0, 0, parent);
    progress->setMinimumDuration(0);
    progress->show();

    connect(progress, &QProgressDialog::canceled, [ proc] () {
            proc->kill();
            qCDebug(KLEOPATRA_LOG) << "Update force canceled. Output:"
                                   << QString::fromLocal8Bit(proc->readAllStandardOutput())
                                   << "stderr:"
                                   << QString::fromLocal8Bit(proc->readAllStandardError());
    });

    connect(proc, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            [parent, progress, proc](int exitCode, QProcess::ExitStatus exitStatus) {
            qCDebug(KLEOPATRA_LOG) << "Update force exited with status:" << exitStatus
                                   << "code:" << exitCode;
            delete progress;
            proc->deleteLater();
            UpdateNotification::checkUpdate(parent, exitStatus == QProcess::NormalExit);
    });

    qCDebug(KLEOPATRA_LOG) << "Starting:" << proc->program() << "args" << proc->arguments();

    proc->start();
}

void UpdateNotification::checkUpdate(QWidget *parent, bool force)
{
#ifdef Q_OS_WIN
    KConfigGroup updatecfg(KSharedConfig::openConfig(), "UpdateNotification");

    if (updatecfg.readEntry("NeverShow", false) && !force) {
        return;
    }

    // Gpg defaults to no update check. For Gpg4win we want this
    // enabled if the user does not explicitly disable update
    // checks neverShow would be true in that case or
    // we would have set AllowVersionCheck once and the user
    // explicitly removed that.
    if (force || updatecfg.readEntry("AllowVersionCheckSetOnce", false)) {
        gpgconf_set_update_check (true);
        updatecfg.writeEntry("AllowVersionCheckSetOnce", true);
    }

    const auto current = gpg4winVersion();
    GpgME::Error err;
    const auto lastshown = updatecfg.readEntry("LastShown", QDateTime());

    if (!force && lastshown.isValid() &&
        lastshown.addSecs(20 * 60 * 60) > QDateTime::currentDateTime()) {
        qDebug() << QDateTime::currentDateTime().addSecs(20 * 60 * 60);
        return;
    }

    const auto results = GpgME::SwdbResult::query("gpg4win",
                                                  current.toUtf8().constData(),
                                                  &err);
    if (err) {
        qCDebug(KLEOPATRA_LOG) << "update check failed: " << err.asString();
        return;
    }

    if (results.size() != 1) {
        /* Should not happen */
        qCDebug(KLEOPATRA_LOG) << "more then one result";
        return;
    }

    const auto result = results[0];

    if (result.update()) {
        const QString newVersion = QStringLiteral("%1.%2.%3").arg(result.version().major)
                                                             .arg(result.version().minor)
                                                             .arg(result.version().patch);
        qCDebug(KLEOPATRA_LOG) << "Have update to version:" << newVersion;
        UpdateNotification notifier(parent, newVersion);
        notifier.exec();
        updatecfg.writeEntry("LastShown", QDateTime::currentDateTime());
        updatecfg.sync();
    } else {
        qCDebug(KLEOPATRA_LOG) << "No update for:" << current;
        if (force) {
            KMessageBox::information(parent,
                                     i18nc("@info",
                                           "No update found in the available version database."),
                                     i18nc("@title", "Up to date"));
        }
    }
#else
    Q_UNUSED(parent)
    Q_UNUSED(force)
#endif
}


UpdateNotification::UpdateNotification(QWidget *parent, const QString &version) :
    QDialog(parent)
{
    resize(400, 200);
    auto lay = new QGridLayout(this);
    auto logo = new QLabel;
    logo->setMaximumWidth(110);

    setAttribute(Qt::WA_QuitOnClose, false);

    KIconLoader *const il = KIconLoader::global();
    const QString iconPath = il->iconPath(QStringLiteral("gpg4win"),
                                          KIconLoader::User);
    logo->setPixmap(QIcon(iconPath).pixmap(100, 100));

    auto label = new QLabel;
    const QString boldVersion = QStringLiteral("<b>%1</b>").arg(version);
    label->setText (i18nc("%1 is the version number", "Version %1 is available.", boldVersion) +
        QStringLiteral("<br><br>") +
        i18nc("Link to NEWS style changelog",
              "See the <a href=\"https://www.gpg4win.org/change-history.html\">new features</a>."));
    label->setOpenExternalLinks(true);
    label->setTextInteractionFlags(Qt::TextBrowserInteraction);
    label->setWordWrap(true);
    setWindowTitle(i18nc("@title:window", "Update Available"));
    setWindowIcon(QIcon(QLatin1String("gpg4win")));

    lay->addWidget(logo, 0, 0);
    lay->addWidget(label, 0, 1);
    const auto chk = new QCheckBox (i18n("Show this notification for future updates."));
    lay->addWidget(chk, 1, 0, 1, -1);

    KConfigGroup updatecfg(KSharedConfig::openConfig(), "UpdateNotification");
    chk->setChecked(!updatecfg.readEntry("NeverShow", false));

    const auto bb = new QDialogButtonBox();
    const auto b = bb->addButton(i18n("&Get update"), QDialogButtonBox::AcceptRole);
    b->setDefault(true);
    b->setIcon(QIcon::fromTheme(QStringLiteral("arrow-down")));
    bb->addButton(QDialogButtonBox::Cancel);
    lay->addWidget(bb, 2, 0, 1, -1);
    connect (bb, &QDialogButtonBox::accepted, this, [this, chk]() {
            QDesktopServices::openUrl(QUrl(QStringLiteral("https://www.gpg4win.org/download.html")));
            KConfigGroup updatecfg(KSharedConfig::openConfig(), "UpdateNotification");
            updatecfg.writeEntry("NeverShow", !chk->isChecked());
            gpgconf_set_update_check (chk->isChecked());
            QDialog::accept();
        });
    connect (bb, &QDialogButtonBox::rejected, this, [this, chk]() {
            KConfigGroup updatecfg(KSharedConfig::openConfig(), "UpdateNotification");
            updatecfg.writeEntry("NeverShow", !chk->isChecked());
            gpgconf_set_update_check (chk->isChecked());
            QDialog::reject();
        });
}
