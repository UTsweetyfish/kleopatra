/* -*- mode: c++; c-basic-offset:4 -*-
    initialization.h

    This file is part of KleopatraClient, the Kleopatra interface library
    SPDX-FileCopyrightText: 2008 Klarälvdalens Datakonsult AB

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <config-kleopatra.h>

#include "initialization.h"

#include <assuan.h>
#include <gpg-error.h>

using namespace KleopatraClientCopy;

Initialization::Initialization()
{
#ifndef HAVE_ASSUAN2
    assuan_set_assuan_err_source(GPG_ERR_SOURCE_DEFAULT);
#else
    assuan_set_gpg_err_source(GPG_ERR_SOURCE_DEFAULT);
#endif
}

Initialization::~Initialization()
{

}
