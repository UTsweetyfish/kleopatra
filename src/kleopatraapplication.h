/*
    kleopatraapplication.h

    This file is part of Kleopatra, the KDE keymanager
    SPDX-FileCopyrightText: 2008 Klarälvdalens Datakonsult AB

    SPDX-FileCopyrightText: 2016 Bundesamt für Sicherheit in der Informationstechnik
    SPDX-FileContributor: Intevation GmbH

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QApplication>
#include <QCommandLineParser>

#include <utils/pimpl_ptr.h>

#include <gpgme++/global.h>

class MainWindow;
class SysTrayIcon;

class KleopatraApplication : public QApplication
{
    Q_OBJECT
public:
    /** Create a new Application object. You have to
     * make sure to call init afterwards to get a valid object.
     * This is to delay initialisation after the UniqueService
     * call is done and our init / call might be forwarded to
     * another instance. */
    KleopatraApplication(int &argc, char *argv[]);
    ~KleopatraApplication();

    /** Initialize the application. Without calling init any
     * other call to KleopatraApplication will result in undefined behavior
     * and likely crash. */
    void init();

    static KleopatraApplication *instance()
    {
        return qobject_cast<KleopatraApplication *>(qApp);
    }

    /** Starts a new instance or a command from the command line.
     *
     * Handles the parser options and starts the according commands.
     * If ignoreNewInstance is set this function does nothing.
     * The parser should have been initialized with kleopatra_options and
     * already processed.
     * If kleopatra is not session restored
     *
     * @param parser: The command line parser to use.
     * @param workingDirectory: Optional working directory for file arguments.
     *
     * @returns an empty QString on success. A localized error message otherwise.
     * */
    QString newInstance(const QCommandLineParser &parser,
                        const QString &workingDirectory = QString());

    void setMainWindow(MainWindow *mw);

    const MainWindow *mainWindow() const;
    MainWindow *mainWindow();

    const SysTrayIcon *sysTrayIcon() const;
    SysTrayIcon *sysTrayIcon();

    void setIgnoreNewInstance(bool on);
    bool ignoreNewInstance() const;
    void toggleMainWindowVisibility();
    void restoreMainWindow();
    void openConfigDialogWithForeignParent(WId parentWId);

public Q_SLOTS:
    void openOrRaiseMainWindow();
    void openOrRaiseConfigDialog();
#ifndef QT_NO_SYSTEMTRAYICON
    void startMonitoringSmartCard();
    void importCertificatesFromFile(const QStringList &files, GpgME::Protocol proto);
#endif
    void encryptFiles(const QStringList &files, GpgME::Protocol proto);
    void signFiles(const QStringList &files, GpgME::Protocol proto);
    void signEncryptFiles(const QStringList &files, GpgME::Protocol proto);
    void decryptFiles(const QStringList &files, GpgME::Protocol proto);
    void verifyFiles(const QStringList &files, GpgME::Protocol proto);
    void decryptVerifyFiles(const QStringList &files, GpgME::Protocol proto);
    void checksumFiles(const QStringList &files, GpgME::Protocol /* unused */);
    void slotActivateRequested(const QStringList &arguments, const QString &workingDirectory);

Q_SIGNALS:
    /* Emitted from slotActivateRequested to enable setting the
     * correct exitValue */
    void setExitValue(int value);

private:
    class Private;
    kdtools::pimpl_ptr<Private> d;
};

