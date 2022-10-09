/* commands/certificatetopivcardcommand.cpp

    This file is part of Kleopatra, the KDE keymanager
    SPDX-FileCopyrightText: 2020 g10 Code GmbH
    SPDX-FileContributor: Ingo Klöcker <dev@ingo-kloecker.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <config-kleopatra.h>

#include "certificatetopivcardcommand.h"

#include "cardcommand_p.h"

#include "commands/authenticatepivcardapplicationcommand.h"

#include "smartcard/pivcard.h"
#include "smartcard/readerstatus.h"

#include "utils/writecertassuantransaction.h"

#include <Libkleo/Dn>
#include <Libkleo/Formatting>
#include <Libkleo/KeyCache>

#include <KLocalizedString>

#include <qgpgme/dataprovider.h>

#include <gpgme++/context.h>

#include <gpg-error.h>
#if GPG_ERROR_VERSION_NUMBER >= 0x12400 // 1.36
# define GPG_ERROR_HAS_NO_AUTH
#endif

#include "kleopatra_debug.h"

using namespace Kleo;
using namespace Kleo::Commands;
using namespace Kleo::SmartCard;
using namespace GpgME;

class CertificateToPIVCardCommand::Private : public CardCommand::Private
{
    friend class ::Kleo::Commands::CertificateToPIVCardCommand;
    CertificateToPIVCardCommand *q_func() const
    {
        return static_cast<CertificateToPIVCardCommand *>(q);
    }
public:
    explicit Private(CertificateToPIVCardCommand *qq, const std::string &slot, const std::string &serialno);
    ~Private();

private:
    void start();
    void startCertificateToPIVCard();

    void authenticate();
    void authenticationFinished();
    void authenticationCanceled();

private:
    std::string cardSlot;
    Key certificate;
    bool hasBeenCanceled = false;
};

CertificateToPIVCardCommand::Private *CertificateToPIVCardCommand::d_func()
{
    return static_cast<Private *>(d.get());
}
const CertificateToPIVCardCommand::Private *CertificateToPIVCardCommand::d_func() const
{
    return static_cast<const Private *>(d.get());
}

#define q q_func()
#define d d_func()


CertificateToPIVCardCommand::Private::Private(CertificateToPIVCardCommand *qq, const std::string &slot, const std::string &serialno)
    : CardCommand::Private(qq, serialno, nullptr)
    , cardSlot(slot)
{
}

CertificateToPIVCardCommand::Private::~Private()
{
}

namespace {
static Key getCertificateToWriteToPIVCard(const std::string &cardSlot, const std::shared_ptr<PIVCard> &card)
{
    if (!cardSlot.empty()) {
        const std::string cardKeygrip = card->keyInfo(cardSlot).grip;
        const auto certificate = KeyCache::instance()->findSubkeyByKeyGrip(cardKeygrip).parent();
        if (certificate.isNull() || certificate.protocol() != GpgME::CMS) {
            return Key();
        }
        if ((cardSlot == PIVCard::pivAuthenticationKeyRef() && certificate.canSign()) ||
            (cardSlot == PIVCard::cardAuthenticationKeyRef() && certificate.canSign()) ||
            (cardSlot == PIVCard::digitalSignatureKeyRef() && certificate.canSign()) ||
            (cardSlot == PIVCard::keyManagementKeyRef() && certificate.canEncrypt())) {
            return certificate;
        }
    }

    return Key();
}
}

void CertificateToPIVCardCommand::Private::start()
{
    qCDebug(KLEOPATRA_LOG) << "CertificateToPIVCardCommand::Private::start()";

    const auto pivCard = SmartCard::ReaderStatus::instance()->getCard<PIVCard>(serialNumber());
    if (!pivCard) {
        error(i18n("Failed to find the PIV card with the serial number: %1", QString::fromStdString(serialNumber())));
        finished();
        return;
    }

    certificate = getCertificateToWriteToPIVCard(cardSlot, pivCard);
    if (certificate.isNull()) {
        error(i18n("Sorry! No suitable certificate to write to this card slot was found."));
        finished();
        return;
    }

    const QString certificateInfo = i18nc("X.509 certificate DN (validity, created: date)", "%1 (%2, created: %3)",
                                          DN(certificate.userID(0).id()).prettyDN(),
                                          Formatting::complianceStringShort(certificate),
                                          Formatting::creationDateString(certificate));
    const QString message = i18nc(
        "@info %1 name of card slot, %2 serial number of card",
        "<p>Please confirm that you want to write the following certificate to the %1 slot of card %2:</p>"
        "<center>%3</center>",
        PIVCard::keyDisplayName(cardSlot), QString::fromStdString(serialNumber()), certificateInfo);
    auto confirmButton = KStandardGuiItem::yes();
    confirmButton.setText(i18nc("@action:button", "Write certificate"));
    confirmButton.setToolTip(QString());
    const auto choice = KMessageBox::questionYesNo(
        parentWidgetOrView(), message, i18nc("@title:window", "Write certificate to card"),
        confirmButton, KStandardGuiItem::cancel(), QString(), KMessageBox::Notify | KMessageBox::WindowModal);
    if (choice != KMessageBox::Yes) {
        finished();
        return;
    }

    startCertificateToPIVCard();
}

void CertificateToPIVCardCommand::Private::startCertificateToPIVCard()
{
    qCDebug(KLEOPATRA_LOG) << "CertificateToPIVCardCommand::Private::startCertificateToPIVCard()";

    auto ctx = Context::createForProtocol(GpgME::CMS);
    QGpgME::QByteArrayDataProvider dp;
    Data data(&dp);
    const Error err = ctx->exportPublicKeys(certificate.primaryFingerprint(), data);
    if (err) {
        error(i18nc("@info", "Exporting the certificate failed: %1", QString::fromUtf8(err.asString())),
              i18nc("@title", "Error"));
        finished();
        return;
    }
    const QByteArray certificateData = dp.data();

    const auto pivCard = SmartCard::ReaderStatus::instance()->getCard<PIVCard>(serialNumber());
    if (!pivCard) {
        error(i18n("Failed to find the PIV card with the serial number: %1", QString::fromStdString(serialNumber())));
        finished();
        return;
    }

    const QByteArray command = QByteArrayLiteral("SCD WRITECERT ") + QByteArray::fromStdString(cardSlot);
    auto transaction = std::unique_ptr<AssuanTransaction>(new WriteCertAssuanTransaction(certificateData));
    ReaderStatus::mutableInstance()->startTransaction(pivCard, command, q_func(), "certificateToPIVCardDone", std::move(transaction));
}

void CertificateToPIVCardCommand::Private::authenticate()
{
    qCDebug(KLEOPATRA_LOG) << "CertificateToPIVCardCommand::authenticate()";

    auto cmd = new AuthenticatePIVCardApplicationCommand(serialNumber(), parentWidgetOrView());
    connect(cmd, &AuthenticatePIVCardApplicationCommand::finished,
            q, [this]() { authenticationFinished(); });
    connect(cmd, &AuthenticatePIVCardApplicationCommand::canceled,
            q, [this]() { authenticationCanceled(); });
    cmd->start();
}

void CertificateToPIVCardCommand::Private::authenticationFinished()
{
    qCDebug(KLEOPATRA_LOG) << "CertificateToPIVCardCommand::authenticationFinished()";
    if (!hasBeenCanceled) {
        startCertificateToPIVCard();
    }
}

void CertificateToPIVCardCommand::Private::authenticationCanceled()
{
    qCDebug(KLEOPATRA_LOG) << "CertificateToPIVCardCommand::authenticationCanceled()";
    hasBeenCanceled = true;
    canceled();
}

CertificateToPIVCardCommand::CertificateToPIVCardCommand(const std::string& cardSlot, const std::string &serialno)
    : CardCommand(new Private(this, cardSlot, serialno))
{
}

CertificateToPIVCardCommand::~CertificateToPIVCardCommand()
{
    qCDebug(KLEOPATRA_LOG) << "CertificateToPIVCardCommand::~CertificateToPIVCardCommand()";
}

void CertificateToPIVCardCommand::certificateToPIVCardDone(const Error &err)
{
    qCDebug(KLEOPATRA_LOG) << "CertificateToPIVCardCommand::certificateToPIVCardDone():"
                           << err.asString() << "(" << err.code() << ")";
    if (err) {
#ifdef GPG_ERROR_HAS_NO_AUTH
        // gpgme 1.13 reports "BAD PIN" instead of "NO AUTH"
        if (err.code() == GPG_ERR_NO_AUTH || err.code() == GPG_ERR_BAD_PIN) {
            d->authenticate();
            return;
        }
#endif

        d->error(i18nc("@info", "Writing the certificate to the card failed: %1", QString::fromUtf8(err.asString())),
                 i18nc("@title", "Error"));
    } else if (!err.isCanceled()) {
        KMessageBox::information(d->parentWidgetOrView(),
                                 i18nc("@info", "Writing the certificate to the card succeeded."),
                                 i18nc("@title", "Success"));
        ReaderStatus::mutableInstance()->updateStatus();
    }

    d->finished();
}

void CertificateToPIVCardCommand::doStart()
{
    qCDebug(KLEOPATRA_LOG) << "CertificateToPIVCardCommand::doStart()";

    d->start();
}

void CertificateToPIVCardCommand::doCancel()
{
}

#undef q_func
#undef d_func
