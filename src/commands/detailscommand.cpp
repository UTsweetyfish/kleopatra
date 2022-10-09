/* -*- mode: c++; c-basic-offset:4 -*-
    commands/detailscommand.cpp

    This file is part of Kleopatra, the KDE keymanager
    SPDX-FileCopyrightText: 2007 Klarälvdalens Datakonsult AB

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <config-kleopatra.h>

#include "detailscommand.h"
#include "command_p.h"

#include <dialogs/certificatedetailsdialog.h>

#include "kleopatra_debug.h"


using namespace Kleo;
using namespace Kleo::Commands;
using namespace GpgME;

class DetailsCommand::Private : public Command::Private
{
    friend class ::Kleo::Commands::DetailsCommand;
    DetailsCommand *q_func() const
    {
        return static_cast<DetailsCommand *>(q);
    }
public:
    explicit Private(DetailsCommand *qq, KeyListController *c);
    ~Private();

private:
    void ensureDialogCreated()
    {
        if (dialog) {
            return;
        }

        auto dlg = new CertificateDetailsDialog;
        applyWindowID(dlg);
        dlg->setAttribute(Qt::WA_DeleteOnClose);
        connect(dlg, &QDialog::finished, q_func(), [this] (int) {
                slotDialogClosed();
            });

        dialog = dlg;
    }

    void ensureDialogVisible()
    {
        ensureDialogCreated();
        if (dialog->isVisible()) {
            dialog->raise();
        } else {
            dialog->show();
        }
    }

    void init()
    {
        q->setWarnWhenRunningAtShutdown(false);
    }

private:
    void slotDialogClosed();

private:
    QPointer<CertificateDetailsDialog> dialog;
};

DetailsCommand::Private *DetailsCommand::d_func()
{
    return static_cast<Private *>(d.get());
}
const DetailsCommand::Private *DetailsCommand::d_func() const
{
    return static_cast<const Private *>(d.get());
}

#define q q_func()
#define d d_func()

DetailsCommand::Private::Private(DetailsCommand *qq, KeyListController *c)
    : Command::Private(qq, c),
      dialog()
{

}

DetailsCommand::Private::~Private() {}

DetailsCommand::DetailsCommand(KeyListController *p)
    : Command(new Private(this, p))
{
    d->init();
}

DetailsCommand::DetailsCommand(QAbstractItemView *v, KeyListController *p)
    : Command(v, new Private(this, p))
{
    d->init();
}

DetailsCommand::DetailsCommand(const Key &key, KeyListController *p)
    : Command(new Private(this, p))
{
    Q_ASSERT(!key.isNull());
    d->init();
    setKey(key);
}

DetailsCommand::DetailsCommand(const Key &key, QAbstractItemView *v, KeyListController *p)
    : Command(v, new Private(this, p))
{
    Q_ASSERT(!key.isNull());
    d->init();
    setKey(key);
}

DetailsCommand::~DetailsCommand() {}

void DetailsCommand::doStart()
{
    const std::vector<Key> keys = d->keys();
    Key key;
    if (keys.size() == 1) {
        key = keys.front();
    } else {
        qCWarning(KLEOPATRA_LOG) << "can only work with one certificate at a time";
    }

    if (key.isNull()) {
        d->finished();
        return;
    }

    d->ensureDialogCreated();

    d->dialog->setKey(key);

    d->ensureDialogVisible();
}

void DetailsCommand::doCancel()
{
    if (d->dialog) {
        d->dialog->close();
    }
}

void DetailsCommand::Private::slotDialogClosed()
{
    finished();
}

#undef q_func
#undef d_func

#include "moc_detailscommand.cpp"
