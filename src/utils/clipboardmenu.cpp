/*
  SPDX-FileCopyrightText: 2014-2021 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: GPL-2.0-only
*/

#include "clipboardmenu.h"

#include "kdtoolsglobal.h"
#include "mainwindow.h"

#include <commands/importcertificatefromclipboardcommand.h>
#include <commands/encryptclipboardcommand.h>
#include <commands/signclipboardcommand.h>
#include <commands/decryptverifyclipboardcommand.h>

#include <KLocalizedString>
#include <KActionMenu>

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QSignalBlocker>

using namespace Kleo;

using namespace Kleo::Commands;

ClipboardMenu::ClipboardMenu(QObject *parent)
    : QObject(parent),
      mWindow(nullptr)
{
    mClipboardMenu = new KActionMenu(i18n("Clipboard"), this);
    mImportClipboardAction = new QAction(i18n("Certificate Import"), this);
    mEncryptClipboardAction = new QAction(i18n("Encrypt..."), this);
    mSmimeSignClipboardAction = new QAction(i18n("S/MIME-Sign..."), this);
    mOpenPGPSignClipboardAction = new QAction(i18n("OpenPGP-Sign..."), this);
    mDecryptVerifyClipboardAction = new QAction(i18n("Decrypt/Verify..."), this);

    KDAB_SET_OBJECT_NAME(mClipboardMenu);
    KDAB_SET_OBJECT_NAME(mImportClipboardAction);
    KDAB_SET_OBJECT_NAME(mEncryptClipboardAction);
    KDAB_SET_OBJECT_NAME(mSmimeSignClipboardAction);
    KDAB_SET_OBJECT_NAME(mOpenPGPSignClipboardAction);
    KDAB_SET_OBJECT_NAME(mDecryptVerifyClipboardAction);

    connect(mImportClipboardAction, &QAction::triggered, this, &ClipboardMenu::slotImportClipboard);
    connect(mEncryptClipboardAction, &QAction::triggered, this, &ClipboardMenu::slotEncryptClipboard);
    connect(mSmimeSignClipboardAction, &QAction::triggered, this, &ClipboardMenu::slotSMIMESignClipboard);
    connect(mOpenPGPSignClipboardAction, &QAction::triggered, this, &ClipboardMenu::slotOpenPGPSignClipboard);
    connect(mDecryptVerifyClipboardAction, &QAction::triggered, this, &ClipboardMenu::slotDecryptVerifyClipboard);
    mClipboardMenu->addAction(mImportClipboardAction);
    mClipboardMenu->addAction(mEncryptClipboardAction);
    mClipboardMenu->addAction(mSmimeSignClipboardAction);
    mClipboardMenu->addAction(mOpenPGPSignClipboardAction);
    mClipboardMenu->addAction(mDecryptVerifyClipboardAction);
    connect(QApplication::clipboard(), &QClipboard::changed, this, &ClipboardMenu::slotEnableDisableActions);
    slotEnableDisableActions();
}

ClipboardMenu::~ClipboardMenu()
{

}

void ClipboardMenu::setMainWindow(MainWindow *window)
{
    mWindow = window;
}

KActionMenu *ClipboardMenu::clipboardMenu() const
{
    return mClipboardMenu;
}

void ClipboardMenu::startCommand(Command *cmd)
{
    Q_ASSERT(cmd);
    cmd->setParent(mWindow);
    cmd->start();
}

void ClipboardMenu::slotImportClipboard()
{
    startCommand(new ImportCertificateFromClipboardCommand(nullptr));
}

void ClipboardMenu::slotEncryptClipboard()
{
    startCommand(new EncryptClipboardCommand(nullptr));
}

void ClipboardMenu::slotOpenPGPSignClipboard()
{
    startCommand(new SignClipboardCommand(GpgME::OpenPGP, nullptr));
}

void ClipboardMenu::slotSMIMESignClipboard()
{
    startCommand(new SignClipboardCommand(GpgME::CMS, nullptr));
}

void ClipboardMenu::slotDecryptVerifyClipboard()
{
    startCommand(new DecryptVerifyClipboardCommand(nullptr));
}

void ClipboardMenu::slotEnableDisableActions()
{
    const QSignalBlocker blocker(QApplication::clipboard());
    mImportClipboardAction->setEnabled(ImportCertificateFromClipboardCommand::canImportCurrentClipboard());
    mEncryptClipboardAction->setEnabled(EncryptClipboardCommand::canEncryptCurrentClipboard());
    mOpenPGPSignClipboardAction->setEnabled(SignClipboardCommand::canSignCurrentClipboard());
    mSmimeSignClipboardAction->setEnabled(SignClipboardCommand::canSignCurrentClipboard());
    mDecryptVerifyClipboardAction->setEnabled(DecryptVerifyClipboardCommand::canDecryptVerifyCurrentClipboard());
}
