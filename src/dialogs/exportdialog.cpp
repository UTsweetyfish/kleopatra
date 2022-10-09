/*  SPDX-FileCopyrightText: 2017 Intevation GmbH

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "exportdialog.h"

#include "kleopatra_debug.h"

#include "view/waitwidget.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

#include <gpgme++/key.h>

#include <QGpgME/Protocol>
#include <QGpgME/ExportJob>

#include <KLocalizedString>
#include <KSharedConfig>
#include <KConfigGroup>

#include <Libkleo/Formatting>

#include <gpgme++/gpgmepp_version.h>
#if GPGMEPP_VERSION >= 0x10E00 // 1.14.0
# define GPGME_HAS_EXPORT_FLAGS
#endif

using namespace Kleo;

class ExportWidget::Private
{
public:
    Private(ExportWidget *qq)
        : q(qq)
    {}

    void setupUi();

    GpgME::Key key;
    GpgME::Subkey subkey;
    QTextEdit *textEdit;
    WaitWidget *waitWidget;
    unsigned int flags;
private:
    ExportWidget *const q;
};

void ExportWidget::Private::setupUi()
{
    auto vlay = new QVBoxLayout(q);
    vlay->setContentsMargins(0, 0, 0, 0);

    textEdit = new QTextEdit;
    textEdit->setVisible(false);
    textEdit->setReadOnly(true);

    auto fixedFont = QFont(QStringLiteral("Monospace"));
    fixedFont.setStyleHint(QFont::TypeWriter);

    textEdit->setFont(fixedFont);
    textEdit->setReadOnly(true);
    vlay->addWidget(textEdit);

    waitWidget = new WaitWidget;
    waitWidget->setText(i18n("Exporting ..."));
    vlay->addWidget(waitWidget);
}

ExportWidget::ExportWidget(QWidget *parent)
    : QWidget(parent)
    , d(new Private(this))
{
    d->setupUi();
}

ExportWidget::~ExportWidget()
{
}

static QString injectComments(const GpgME::Key &key, const QByteArray &data)
{
    QString ret = QString::fromUtf8(data);

    if (key.protocol() != GpgME::OpenPGP) {
        return ret;
    }

    auto overView = Formatting::toolTip(key, Formatting::Fingerprint |
                                             Formatting::UserIDs |
                                             Formatting::Issuer |
                                             Formatting::Subject |
                                             Formatting::ExpiryDates |
                                             Formatting::CertificateType |
                                             Formatting::CertificateUsage);

    // Fixup the HTML coming from the toolTip for our own format.
    overView.remove(QLatin1String("<tr><th>"));
    overView.replace(QLatin1String("</th><td>"), QLatin1String("\t"));
    overView.replace(QLatin1String("</td></tr>"), QLatin1String("\n"));
    overView.remove(QLatin1String("<table border=\"0\">"));
    overView.remove(QLatin1String("\n</table>"));
    overView.replace(QLatin1String("&lt;"), QLatin1String("<"));
    overView.replace(QLatin1String("&gt;"), QLatin1String(">"));

    auto overViewLines = overView.split(QLatin1Char('\n'));

    // Format comments so that they fit for RFC 4880
    auto comments = QStringLiteral("Comment: ");
    comments += overViewLines.join(QLatin1String("\nComment: ")) + QLatin1Char('\n');

    ret.insert(37 /* -----BEGIN PGP PUBLIC KEY BLOCK-----\n */, comments);

    return ret;
}

void ExportWidget::exportResult(const GpgME::Error &err, const QByteArray &data)
{
    d->waitWidget->setVisible(false);
    d->textEdit->setVisible(true);

    if (err) {
        /* Should not happen. But well,.. */
        d->textEdit->setText(i18nc("%1 is error message", "Failed to export: '%1'", QString::fromLatin1(err.asString())));
    }

    if (!d->flags) {
        d->textEdit->setText(injectComments(d->key, data));
    } else {
        d->textEdit->setText(QString::fromUtf8(data));
    }
}

void ExportWidget::setKey(const GpgME::Subkey &key, unsigned int flags)
{
    d->waitWidget->setVisible(true);
    d->textEdit->setVisible(false);
    d->key = key.parent();
    d->subkey = key;
    d->flags = flags;

    auto protocol = d->key.protocol() == GpgME::CMS ?
                                         QGpgME::smime() : QGpgME::openpgp();

    auto job = protocol->publicKeyExportJob(true);

    connect(job, &QGpgME::ExportJob::result,
            this, &ExportWidget::exportResult);

#ifdef GPGME_HAS_EXPORT_FLAGS
    job->setExportFlags(flags);
#endif
    job->start(QStringList() << QLatin1String(key.fingerprint()) + QLatin1Char('!'));
}

void ExportWidget::setKey(const GpgME::Key &key, unsigned int flags)
{
    d->waitWidget->setVisible(true);
    d->textEdit->setVisible(false);
    d->key = key;
    d->flags = flags;

    auto protocol = key.protocol() == GpgME::CMS ?
                                      QGpgME::smime() : QGpgME::openpgp();

    auto job = protocol->publicKeyExportJob(true);

    connect(job, &QGpgME::ExportJob::result,
            this, &ExportWidget::exportResult);

#ifdef GPGME_HAS_EXPORT_FLAGS
    job->setExportFlags(flags);
#endif
    job->start(QStringList() << QLatin1String(key.primaryFingerprint()));
}

GpgME::Key ExportWidget::key() const
{
    return d->key;
}

ExportDialog::ExportDialog(QWidget *parent)
    : QDialog(parent),
      mWidget(new ExportWidget(this))
{
    KConfigGroup dialog(KSharedConfig::openStateConfig(), "ExportDialog");
    const auto size = dialog.readEntry("Size", QSize(600, 800));
    if (size.isValid()) {
        resize(size);
    }
    setWindowTitle(i18nc("@title:window", "Export"));

    auto l = new QVBoxLayout(this);
    l->addWidget(mWidget);

    auto bbox = new QDialogButtonBox(this);
    auto btn = bbox->addButton(QDialogButtonBox::Close);
    connect(btn, &QPushButton::pressed, this, &QDialog::accept);
    l->addWidget(bbox);
}

ExportDialog::~ExportDialog()
{
    KConfigGroup dialog(KSharedConfig::openStateConfig(), "ExportDialog");
    dialog.writeEntry("Size", size());
    dialog.sync();
}

void ExportDialog::setKey(const GpgME::Key &key, unsigned int flags)
{
    mWidget->setKey(key, flags);
}

void ExportDialog::setKey(const GpgME::Subkey &key, unsigned int flags)
{
    mWidget->setKey(key, flags);
}

GpgME::Key ExportDialog::key() const
{
    return mWidget->key();
}

