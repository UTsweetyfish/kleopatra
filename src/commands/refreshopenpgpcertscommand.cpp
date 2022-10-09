/* -*- mode: c++; c-basic-offset:4 -*-
    commands/refreshopenpgpcertscommand.cpp

    This file is part of Kleopatra, the KDE keymanager
    SPDX-FileCopyrightText: 2008 Klarälvdalens Datakonsult AB

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <config-kleopatra.h>

#include "refreshopenpgpcertscommand.h"

#include <Libkleo/GnuPG>

#include <KLocalizedString>
#include <KMessageBox>

using namespace Kleo;
using namespace Kleo::Commands;

RefreshOpenPGPCertsCommand::RefreshOpenPGPCertsCommand(KeyListController *c)
    : GnuPGProcessCommand(c)
{
    setShowsOutputWindow(true);
}

RefreshOpenPGPCertsCommand::RefreshOpenPGPCertsCommand(QAbstractItemView *v, KeyListController *c)
    : GnuPGProcessCommand(v, c)
{
    setShowsOutputWindow(true);
}

RefreshOpenPGPCertsCommand::~RefreshOpenPGPCertsCommand() {}

bool RefreshOpenPGPCertsCommand::preStartHook(QWidget *parent) const
{
    if (!haveKeyserverConfigured())
        if (KMessageBox::warningContinueCancel(parent,
                                               xi18nc("@info",
                                                       "<para>No OpenPGP directory services have been configured.</para>"
                                                       "<para>If not all of the certificates carry the name of their preferred "
                                                       "certificate server (few do), a fallback server is needed to fetch from.</para>"
                                                       "<para>Since none is configured, <application>Kleopatra</application> will use "
                                                       "<resource>keys.gnupg.net</resource> as the fallback.</para>"
                                                       "<para>You can configure OpenPGP directory servers in Kleopatra's "
                                                       "configuration dialog.</para>"
                                                       "<para>Do you want to continue with <resource>keys.gnupg.net</resource> "
                                                       "as fallback server?</para>"),
                                               i18nc("@title:window", "OpenPGP Certificate Refresh"),
                                               KStandardGuiItem::cont(), KStandardGuiItem::cancel(),
                                               QStringLiteral("warn-refresh-openpgp-missing-keyserver"))
                != KMessageBox::Continue) {
            return false;
        }
    return KMessageBox::warningContinueCancel(parent,
            xi18nc("@info",
                   "<para>Refreshing OpenPGP certificates implies downloading all certificates anew, "
                   "to check if any of them have been revoked in the meantime.</para>"
                   "<para>This can put a severe strain on your own as well as other people's network "
                   "connections, and can take up to an hour or more to complete, depending on "
                   "your network connection, and the number of certificates to check.</para> "
                   "<para>Are you sure you want to continue?</para>"),
            i18nc("@title:window", "OpenPGP Certificate Refresh"),
            KStandardGuiItem::cont(), KStandardGuiItem::cancel(),
            QStringLiteral("warn-refresh-openpgp-expensive"))
           == KMessageBox::Continue;
}

QStringList RefreshOpenPGPCertsCommand::arguments() const
{
    QStringList result;
    result << gpgPath();
    if (!haveKeyserverConfigured()) {
        result << QStringLiteral("--keyserver") << QStringLiteral("keys.gnupg.net");
    }
    result << QStringLiteral("--refresh-keys");
    return result;
}

QString RefreshOpenPGPCertsCommand::errorCaption() const
{
    return i18nc("@title:window", "OpenPGP Certificate Refresh Error");
}

QString RefreshOpenPGPCertsCommand::successCaption() const
{
    return i18nc("@title:window", "OpenPGP Certificate Refresh Finished");
}

QString RefreshOpenPGPCertsCommand::crashExitMessage(const QStringList &args) const
{
    return xi18nc("@info",
                  "<para>The GPG process that tried to refresh OpenPGP certificates "
                  "ended prematurely because of an unexpected error.</para>"
                  "<para>Please check the output of <icode>%1</icode> for details.</para>", args.join(QLatin1Char(' ')));
}

QString RefreshOpenPGPCertsCommand::errorExitMessage(const QStringList &args) const
{
    return xi18nc("@info",
                  "<para>An error occurred while trying to refresh OpenPGP certificates.</para> "
                  "<para>The output from <command>%1</command> was: <bcode>%2</bcode></para>",
                  args[0], errorString());
}

QString RefreshOpenPGPCertsCommand::successMessage(const QStringList &) const
{
    return i18nc("@info", "OpenPGP certificates refreshed successfully.");
    // ### --check-trustdb
}

