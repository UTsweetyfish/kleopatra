/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/decryptverifyoperationwidget.cpp

    This file is part of Kleopatra, the KDE keymanager
    SPDX-FileCopyrightText: 2007 Klarälvdalens Datakonsult AB

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <config-kleopatra.h>

#include "decryptverifyoperationwidget.h"

#include <utils/archivedefinition.h>

#include <Libkleo/FileNameRequester>

#include <KLocalizedString>

#include <QLabel>
#include <QCheckBox>
#include <QStackedLayout>
#include <QComboBox>

using namespace Kleo;
using namespace Kleo::Crypto::Gui;

class DecryptVerifyOperationWidget::Private
{
    friend class ::Kleo::Crypto::Gui::DecryptVerifyOperationWidget;
    DecryptVerifyOperationWidget *const q;
public:
    explicit Private(DecryptVerifyOperationWidget *qq);
    ~Private();

    void enableDisableWidgets()
    {
        const bool detached = ui.verifyDetachedCB.isChecked();
        const bool archive  = ui.archiveCB.isChecked();
        ui.archiveCB.setEnabled(!detached);
        ui.archivesCB.setEnabled(archive && !detached);
    }
private:
    struct UI {
        QGridLayout   glay;
        QLabel         inputLB;
        QStackedLayout inputStack;
        QLabel            inputFileNameLB;
        FileNameRequester inputFileNameRQ;
        //------
        QCheckBox      verifyDetachedCB;
        //------
        QLabel         signedDataLB;
        QStackedLayout signedDataStack;
        QLabel            signedDataFileNameLB;
        FileNameRequester signedDataFileNameRQ;
        //------
        QHBoxLayout    hlay;
        QCheckBox       archiveCB;
        QComboBox       archivesCB;

        explicit UI(DecryptVerifyOperationWidget *q);
    } ui;
};

DecryptVerifyOperationWidget::Private::UI::UI(DecryptVerifyOperationWidget *q)
    : glay(q),
      inputLB(i18n("Input file:"), q),
      inputStack(),
      inputFileNameLB(q),
      inputFileNameRQ(q),
      verifyDetachedCB(i18n("&Input file is a detached signature"), q),
      signedDataLB(i18n("&Signed data:"), q),
      signedDataStack(),
      signedDataFileNameLB(q),
      signedDataFileNameRQ(q),
      hlay(),
      archiveCB(i18n("&Input file is an archive; unpack with:"), q),
      archivesCB(q)
{
    KDAB_SET_OBJECT_NAME(glay);
    KDAB_SET_OBJECT_NAME(inputLB);
    KDAB_SET_OBJECT_NAME(inputStack);
    KDAB_SET_OBJECT_NAME(inputFileNameLB);
    KDAB_SET_OBJECT_NAME(inputFileNameRQ);
    KDAB_SET_OBJECT_NAME(verifyDetachedCB);
    KDAB_SET_OBJECT_NAME(signedDataLB);
    KDAB_SET_OBJECT_NAME(signedDataStack);
    KDAB_SET_OBJECT_NAME(signedDataFileNameLB);
    KDAB_SET_OBJECT_NAME(signedDataFileNameRQ);
    KDAB_SET_OBJECT_NAME(hlay);
    KDAB_SET_OBJECT_NAME(archiveCB);
    KDAB_SET_OBJECT_NAME(archivesCB);

    inputStack.setContentsMargins(0, 0, 0, 0);
    signedDataStack.setContentsMargins(0, 0, 0, 0);

    signedDataLB.setEnabled(false);
    signedDataFileNameLB.setEnabled(false);
    signedDataFileNameRQ.setEnabled(false);
    archivesCB.setEnabled(false);

    glay.setContentsMargins(0, 0, 0, 0);
    glay.addWidget(&inputLB, 0, 0);
    glay.addLayout(&inputStack, 0, 1);
    inputStack.addWidget(&inputFileNameLB);
    inputStack.addWidget(&inputFileNameRQ);

    glay.addWidget(&verifyDetachedCB, 1, 0, 1, 2);

    glay.addWidget(&signedDataLB, 2, 0);
    glay.addLayout(&signedDataStack, 2, 1);
    signedDataStack.addWidget(&signedDataFileNameLB);
    signedDataStack.addWidget(&signedDataFileNameRQ);

    glay.addLayout(&hlay, 3, 0, 1, 2);
    hlay.addWidget(&archiveCB);
    hlay.addWidget(&archivesCB, 1);

    connect(&verifyDetachedCB, &QCheckBox::toggled, &signedDataLB, &QLabel::setEnabled);
    connect(&verifyDetachedCB, &QCheckBox::toggled, &signedDataFileNameLB, &QLabel::setEnabled);
    connect(&verifyDetachedCB, &QCheckBox::toggled, &signedDataFileNameRQ, &FileNameRequester::setEnabled);
    connect(&verifyDetachedCB, SIGNAL(toggled(bool)), q, SLOT(enableDisableWidgets()));
    connect(&archiveCB, SIGNAL(toggled(bool)), q, SLOT(enableDisableWidgets()));
}

DecryptVerifyOperationWidget::Private::Private(DecryptVerifyOperationWidget *qq)
    : q(qq),
      ui(q)
{

}

DecryptVerifyOperationWidget::Private::~Private() {}

DecryptVerifyOperationWidget::DecryptVerifyOperationWidget(QWidget *p)
    : QWidget(p), d(new Private(this))
{
    setMode(DecryptVerifyOpaque);
}

DecryptVerifyOperationWidget::~DecryptVerifyOperationWidget() {}

void DecryptVerifyOperationWidget::setArchiveDefinitions(const std::vector< std::shared_ptr<ArchiveDefinition> > &archiveDefinitions)
{
    d->ui.archivesCB.clear();
    for (const std::shared_ptr<ArchiveDefinition> &ad : archiveDefinitions) {
        d->ui.archivesCB.addItem(ad->label(), QVariant::fromValue(ad));
    }
}

void DecryptVerifyOperationWidget::setMode(Mode mode)
{
    setMode(mode, std::shared_ptr<ArchiveDefinition>());
}

void DecryptVerifyOperationWidget::setMode(Mode mode, const std::shared_ptr<ArchiveDefinition> &ad)
{
    d->ui.verifyDetachedCB.setChecked(mode != DecryptVerifyOpaque);

    QWidget *inputWidget;
    QWidget *signedDataWidget;
    if (mode == VerifyDetachedWithSignedData) {
        inputWidget      = &d->ui.inputFileNameRQ;
        signedDataWidget = &d->ui.signedDataFileNameLB;
    } else {
        inputWidget      = &d->ui.inputFileNameLB;
        signedDataWidget = &d->ui.signedDataFileNameRQ;
    }

    d->ui.inputStack.setCurrentWidget(inputWidget);
    d->ui.signedDataStack.setCurrentWidget(signedDataWidget);

    d->ui.inputLB.setBuddy(inputWidget);
    d->ui.signedDataLB.setBuddy(signedDataWidget);

    d->ui.archiveCB.setChecked(ad.get() != nullptr);
    for (int i = 0, end = d->ui.archivesCB.count(); i != end; ++i)
        if (ad == d->ui.archivesCB.itemData(i).value< std::shared_ptr<ArchiveDefinition> >()) {
            d->ui.archivesCB.setCurrentIndex(i);
            return;
        }
}

DecryptVerifyOperationWidget::Mode DecryptVerifyOperationWidget::mode() const
{
    if (d->ui.verifyDetachedCB.isChecked())
        if (d->ui.inputStack.currentIndex() == 0) {
            return VerifyDetachedWithSignature;
        } else {
            return VerifyDetachedWithSignedData;
        }
    else {
        return DecryptVerifyOpaque;
    }
}

void DecryptVerifyOperationWidget::setInputFileName(const QString &name)
{
    d->ui.inputFileNameLB.setText(name);
    d->ui.inputFileNameRQ.setFileName(name);
}

QString DecryptVerifyOperationWidget::inputFileName() const
{
    if (d->ui.inputStack.currentIndex() == 0) {
        return d->ui.inputFileNameLB.text();
    } else {
        return d->ui.inputFileNameRQ.fileName();
    }
}

void DecryptVerifyOperationWidget::setSignedDataFileName(const QString &name)
{
    d->ui.signedDataFileNameLB.setText(name);
    d->ui.signedDataFileNameRQ.setFileName(name);
}

QString DecryptVerifyOperationWidget::signedDataFileName() const
{
    if (d->ui.signedDataStack.currentIndex() == 0) {
        return d->ui.signedDataFileNameLB.text();
    } else {
        return d->ui.signedDataFileNameRQ.fileName();
    }
}

std::shared_ptr<ArchiveDefinition> DecryptVerifyOperationWidget::selectedArchiveDefinition() const
{
    if (mode() == DecryptVerifyOpaque && d->ui.archiveCB.isChecked()) {
        return d->ui.archivesCB.itemData(d->ui.archivesCB.currentIndex()).value< std::shared_ptr<ArchiveDefinition> >();
    } else {
        return std::shared_ptr<ArchiveDefinition>();
    }
}

#include "moc_decryptverifyoperationwidget.cpp"
