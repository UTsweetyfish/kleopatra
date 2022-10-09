/*  smartcard/pivcard.h

    This file is part of Kleopatra, the KDE keymanager
    SPDX-FileCopyrightText: 2020 g10 Code GmbH
    SPDX-FileContributor: Ingo Klöcker <dev@ingo-kloecker.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once

#include "card.h"

namespace Kleo
{
namespace SmartCard
{
struct KeyPairInfo;

/** Class to work with PIV smartcards or compatible tokens */
class PIVCard: public Card
{
public:
    explicit PIVCard(const Card &card);

    static const std::string AppName;

    static std::string pivAuthenticationKeyRef();
    static std::string cardAuthenticationKeyRef();
    static std::string digitalSignatureKeyRef();
    static std::string keyManagementKeyRef();

    static std::string pinKeyRef();
    static std::string pukKeyRef();

    static const std::vector<KeyPairInfo> & supportedKeys();
    static QString keyDisplayName(const std::string &keyRef);
    static std::vector< std::pair<std::string, QString> > supportedAlgorithms(const std::string &keyRef);

    std::string keyAlgorithm(const std::string &keyRef) const;
    void setKeyAlgorithm(const std::string &keyRef, const std::string &algorithm);

    std::string certificateData(const std::string &keyRef) const;
    void setCertificateData(const std::string &keyRef, const std::string &data);
};
} // namespace Smartcard
} // namespace Kleopatra

