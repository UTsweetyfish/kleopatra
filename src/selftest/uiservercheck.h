/* -*- mode: c++; c-basic-offset:4 -*-
    selftest/uiservercheck.h

    This file is part of Kleopatra, the KDE keymanager
    SPDX-FileCopyrightText: 2008 Klarälvdalens Datakonsult AB

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <selftest/selftest.h>

#include <memory>

namespace Kleo
{

class SelfTest;

std::shared_ptr<SelfTest> makeUiServerConnectivitySelfTest();

}

