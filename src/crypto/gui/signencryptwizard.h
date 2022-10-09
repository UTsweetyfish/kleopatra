/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/gui/signencryptwizard.h

    This file is part of Kleopatra, the KDE keymanager
    SPDX-FileCopyrightText: 2007 Klarälvdalens Datakonsult AB

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <crypto/gui/wizard.h>

#include <crypto/gui/signerresolvepage.h>

#include <utils/pimpl_ptr.h>

#include <gpgme++/global.h>
#include <kmime/kmime_header_parsing.h>

#include <memory>
#include <vector>

namespace GpgME
{
class Key;
}

class QFileInfo;
template <typename T> class QList;
using QFileInfoList = QList<QFileInfo>;

namespace Kleo
{
namespace Crypto
{

class Task;
class TaskCollection;

namespace Gui
{

class ObjectsPage;
class ResolveRecipientsPage;
class ResultPage;
class SignerResolvePage;

class SignEncryptWizard : public Wizard
{
    Q_OBJECT
public:
    explicit SignEncryptWizard(QWidget *parent = nullptr, Qt::WindowFlags f = {});
    ~SignEncryptWizard() override;

    enum Page {
        ResolveSignerPage = 0,
        ObjectsPage,
        ResolveRecipientsPage,
        ResultPage
    };

    void setCommitPage(Page);

    GpgME::Protocol presetProtocol() const;
    void setPresetProtocol(GpgME::Protocol proto);

    GpgME::Protocol selectedProtocol() const;

    /// SignOrEncryptFiles mode subinterface
    //@{

    QFileInfoList resolvedFiles() const;
    void setFiles(const QStringList &files);

    bool signingSelected() const;
    void setSigningSelected(bool selected);

    bool encryptionSelected() const;
    void setEncryptionSelected(bool selected);

    bool isSigningUserMutable() const;
    void setSigningUserMutable(bool isMutable);

    bool isEncryptionUserMutable() const;
    void setEncryptionUserMutable(bool isMutable);

    bool isMultipleProtocolsAllowed() const;
    void setMultipleProtocolsAllowed(bool allowed);

    //@}

    void setRecipients(const std::vector<KMime::Types::Mailbox> &recipients, const std::vector<KMime::Types::Mailbox> &encryptoToSelfRecipients);

    /** if true, the user is allowed to remove/add recipients via the UI.
     * Defaults to @p false.
     */
    bool recipientsUserMutable() const;
    void setRecipientsUserMutable(bool isMutable);

    void setSignersAndCandidates(const std::vector<KMime::Types::Mailbox> &signers, const std::vector< std::vector<GpgME::Key> > &keys);

    void setTaskCollection(const std::shared_ptr<TaskCollection> &tasks);

    std::vector<GpgME::Key> resolvedCertificates() const;
    std::vector<GpgME::Key> resolvedSigners() const;

    bool isAsciiArmorEnabled() const;
    void setAsciiArmorEnabled(bool enabled);

    bool keepResultPageOpenWhenDone() const;
    void setKeepResultPageOpenWhenDone(bool keep);

    void onNext(int currentId) override;

Q_SIGNALS:
    void signersResolved();
    void objectsResolved();
    void recipientsResolved();

protected:
    Gui::SignerResolvePage *signerResolvePage();
    const Gui::SignerResolvePage *signerResolvePage() const;
    Gui::ObjectsPage *objectsPage();
    Gui::ResultPage *resultPage();
    Gui::ResolveRecipientsPage *resolveRecipientsPage();

    void setSignerResolvePageValidator(const std::shared_ptr<SignerResolvePage::Validator> &validator);

private:
    class Private;
    kdtools::pimpl_ptr<Private> d;
};

}
}
}

