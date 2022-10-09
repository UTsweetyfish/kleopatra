/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/encryptemailtask.cpp

    This file is part of Kleopatra, the KDE keymanager
    SPDX-FileCopyrightText: 2007 Klarälvdalens Datakonsult AB

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <config-kleopatra.h>

#include "encryptemailtask.h"

#include <utils/input.h>
#include <utils/output.h>
#include <utils/kleo_assert.h>
#include <utils/auditlog.h>

#include <Libkleo/Stl_Util>
#include <QGpgME/Protocol>
#include <QGpgME/EncryptJob>

#include <gpgme++/encryptionresult.h>
#include <gpgme++/key.h>

#include <KLocalizedString>

#include <QPointer>
#include <QTextDocument> // for Qt::escape

using namespace Kleo;
using namespace Kleo::Crypto;
using namespace GpgME;

namespace
{

class EncryptEMailResult : public Task::Result
{
    const EncryptionResult m_result;
    const AuditLog m_auditLog;
public:
    EncryptEMailResult(const EncryptionResult &r, const AuditLog &auditLog)
        : Task::Result(), m_result(r), m_auditLog(auditLog) {}

    QString overview() const override;
    QString details() const override;
    int errorCode() const override;
    QString errorString() const override;
    VisualCode code() const override;
    AuditLog auditLog() const override;
};

QString makeResultString(const EncryptionResult &res)
{
    const Error err = res.error();

    if (err.isCanceled()) {
        return i18n("Encryption canceled.");
    }

    if (err) {
        return i18n("Encryption failed: %1", QString::fromLocal8Bit(err.asString()).toHtmlEscaped());
    }

    return i18n("Encryption succeeded.");
}

}

class EncryptEMailTask::Private
{
    friend class ::Kleo::Crypto::EncryptEMailTask;
    EncryptEMailTask *const q;
public:
    explicit Private(EncryptEMailTask *qq);

private:
    std::unique_ptr<QGpgME::EncryptJob> createJob(GpgME::Protocol proto);

private:
    void slotResult(const EncryptionResult &);

private:
    std::shared_ptr<Input> input;
    std::shared_ptr<Output> output;
    std::vector<Key> recipients;

    QPointer<QGpgME::EncryptJob> job;
};

EncryptEMailTask::Private::Private(EncryptEMailTask *qq)
    : q(qq),
      input(),
      output(),
      job(nullptr)
{

}

EncryptEMailTask::EncryptEMailTask(QObject *p)
    : Task(p), d(new Private(this))
{

}

EncryptEMailTask::~EncryptEMailTask() {}

void EncryptEMailTask::setInput(const std::shared_ptr<Input> &input)
{
    kleo_assert(!d->job);
    kleo_assert(input);
    d->input = input;
}

void EncryptEMailTask::setOutput(const std::shared_ptr<Output> &output)
{
    kleo_assert(!d->job);
    kleo_assert(output);
    d->output = output;
}

void EncryptEMailTask::setRecipients(const std::vector<Key> &recipients)
{
    kleo_assert(!d->job);
    kleo_assert(!recipients.empty());
    d->recipients = recipients;
}

Protocol EncryptEMailTask::protocol() const
{
    kleo_assert(!d->recipients.empty());
    return d->recipients.front().protocol();
}

QString EncryptEMailTask::label() const
{
    return d->input ? d->input->label() : QString();
}

unsigned long long EncryptEMailTask::inputSize() const
{
    return d->input ? d->input->size() : 0;
}

void EncryptEMailTask::doStart()
{
    kleo_assert(!d->job);
    kleo_assert(d->input);
    kleo_assert(d->output);
    kleo_assert(!d->recipients.empty());

    std::unique_ptr<QGpgME::EncryptJob> job = d->createJob(protocol());
    kleo_assert(job.get());

    job->start(d->recipients,
               d->input->ioDevice(), d->output->ioDevice(),
               /*alwaysTrust=*/true);

    d->job = job.release();
}

void EncryptEMailTask::cancel()
{
    if (d->job) {
        d->job->slotCancel();
    }
}

std::unique_ptr<QGpgME::EncryptJob> EncryptEMailTask::Private::createJob(GpgME::Protocol proto)
{
    const QGpgME::Protocol *const backend = (proto == GpgME::OpenPGP) ? QGpgME::openpgp() : QGpgME::smime();
    kleo_assert(backend);
    bool shouldArmor = (proto == OpenPGP || q->asciiArmor()) && !output->binaryOpt();
    std::unique_ptr<QGpgME::EncryptJob> encryptJob(backend->encryptJob(shouldArmor, /*textmode=*/false));
    kleo_assert(encryptJob.get());
    if (proto == CMS && !q->asciiArmor() && !output->binaryOpt()) {
        encryptJob->setOutputIsBase64Encoded(true);
    }
    connect(encryptJob.get(), SIGNAL(progress(QString,int,int)),
            q, SLOT(setProgress(QString,int,int)));
    connect(encryptJob.get(), SIGNAL(result(GpgME::EncryptionResult,QByteArray)),
            q, SLOT(slotResult(GpgME::EncryptionResult)));
    return encryptJob;
}

void EncryptEMailTask::Private::slotResult(const EncryptionResult &result)
{
    const auto *const job = qobject_cast<const QGpgME::Job *>(q->sender());
    if (result.error().code()) {
        output->cancel();
    } else {
        output->finalize();
    }
    q->emitResult(std::shared_ptr<Result>(new EncryptEMailResult(result, AuditLog::fromJob(job))));
}

QString EncryptEMailResult::overview() const
{
    return makeOverview(makeResultString(m_result));
}

QString EncryptEMailResult::details() const
{
    return QString();
}

int EncryptEMailResult::errorCode() const
{
    return m_result.error().encodedError();
}

QString EncryptEMailResult::errorString() const
{
    return hasError() ? makeResultString(m_result) : QString();
}

AuditLog EncryptEMailResult::auditLog() const
{
    return m_auditLog;
}

Task::Result::VisualCode EncryptEMailResult::code() const
{
    if (m_result.error().isCanceled()) {
        return Warning;
    }
    return m_result.error().code() ? NeutralError : NeutralSuccess;
}

#include "moc_encryptemailtask.cpp"

