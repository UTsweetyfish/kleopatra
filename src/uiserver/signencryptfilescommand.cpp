/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/signencryptfilescommand.cpp

    This file is part of Kleopatra, the KDE keymanager
    SPDX-FileCopyrightText: 2007 Klarälvdalens Datakonsult AB

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <config-kleopatra.h>

#include "signencryptfilescommand.h"

#include <crypto/signencryptfilescontroller.h>

#include <Libkleo/KleoException>

#include <KLocalizedString>

using namespace Kleo;
using namespace Kleo::Crypto;

class SignEncryptFilesCommand::Private : public QObject
{
    Q_OBJECT
private:
    friend class ::Kleo::SignEncryptFilesCommand;
    SignEncryptFilesCommand *const q;
public:
    explicit Private(SignEncryptFilesCommand *qq)
        : q(qq),
          controller()
    {

    }

private:
    void checkForErrors() const;

private Q_SLOTS:
    void slotDone();
    void slotError(int, const QString &);

private:
    std::shared_ptr<SignEncryptFilesController> controller;
};

SignEncryptFilesCommand::SignEncryptFilesCommand()
    : AssuanCommandMixin<SignEncryptFilesCommand>(), d(new Private(this))
{

}

SignEncryptFilesCommand::~SignEncryptFilesCommand() {}

void SignEncryptFilesCommand::Private::checkForErrors() const
{

    if (!q->numFiles())
        throw Exception(makeError(GPG_ERR_ASS_NO_INPUT),
                        i18n("At least one FILE must be present"));

    if (!q->senders().empty())
        throw Exception(makeError(GPG_ERR_CONFLICT),
                        i18n("%1 is a filemanager mode command, "
                             "connection seems to be in email mode (%2 present)",
                             QString::fromLatin1(q->name()), QStringLiteral("SENDER")));
    if (!q->recipients().empty())
        throw Exception(makeError(GPG_ERR_CONFLICT),
                        i18n("%1 is a filemanager mode command, "
                             "connection seems to be in email mode (%2 present)",
                             QString::fromLatin1(q->name()), QStringLiteral("RECIPIENT")));

    if (!q->inputs().empty())
        throw Exception(makeError(GPG_ERR_CONFLICT),
                        i18n("%1 is a filemanager mode command, "
                             "connection seems to be in email mode (%2 present)",
                             QString::fromLatin1(q->name()), QStringLiteral("INPUT")));
    if (!q->outputs().empty())
        throw Exception(makeError(GPG_ERR_CONFLICT),
                        i18n("%1 is a filemanager mode command, "
                             "connection seems to be in email mode (%2 present)",
                             QString::fromLatin1(q->name()), QStringLiteral("OUTPUT")));
    if (!q->messages().empty())
        throw Exception(makeError(GPG_ERR_CONFLICT),
                        i18n("%1 is a filemanager mode command, "
                             "connection seems to be in email mode (%2 present)",
                             QString::fromLatin1(q->name()), QStringLiteral("MESSAGE")));
}

int SignEncryptFilesCommand::doStart()
{

    d->checkForErrors();

    d->controller.reset(new SignEncryptFilesController(shared_from_this()));

    d->controller->setProtocol(checkProtocol(FileManager));

    unsigned int op = operation();
    if (hasOption("archive")) {
        op |= SignEncryptFilesController::ArchiveForced;
    } else {
        op |= SignEncryptFilesController::ArchiveAllowed;
    }
    d->controller->setOperationMode(op);
    d->controller->setFiles(fileNames());

    QObject::connect(d->controller.get(), SIGNAL(done()), d.get(), SLOT(slotDone()), Qt::QueuedConnection);
    QObject::connect(d->controller.get(), SIGNAL(error(int,QString)), d.get(), SLOT(slotError(int,QString)), Qt::QueuedConnection);

    d->controller->start();

    return 0;
}

void SignEncryptFilesCommand::Private::slotDone()
{
    q->done();
}

void SignEncryptFilesCommand::Private::slotError(int err, const QString &details)
{
    q->done(err, details);
}

void SignEncryptFilesCommand::doCanceled()
{
    if (d->controller) {
        d->controller->cancel();
    }
}

#include "signencryptfilescommand.moc"

