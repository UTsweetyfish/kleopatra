/* crypto/gui/decryptverifyfilesdialog.h

    This file is part of Kleopatra, the KDE keymanager
    SPDX-FileCopyrightText: 2007 Klarälvdalens Datakonsult AB
    SPDX-FileCopyrightText: 2016 Bundesamt für Sicherheit in der Informationstechnik
    SPDX-FileContributor: Intevation GmbH

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once

#include <QDialog>
#include <QString>
#include <QDialogButtonBox>
#include <QHash>
#include "crypto/task.h"

#include <memory>

class QVBoxLayout;
class QProgressBar;
template <typename K, typename U> class QHash;
class QLabel;

namespace Kleo
{
class FileNameRequester;
namespace Crypto
{
class TaskCollection;

namespace Gui
{
class ResultListWidget;

class DecryptVerifyFilesDialog : public QDialog
{
    Q_OBJECT
public:
    explicit DecryptVerifyFilesDialog(const std::shared_ptr<TaskCollection> &coll,
                                      QWidget *parent = nullptr);
    ~DecryptVerifyFilesDialog();

    void setOutputLocation(const QString &dir);
    QString outputLocation() const;

protected Q_SLOTS:
    void progress(const QString &msg, int progress, int total);
    void started(const std::shared_ptr<Task> &result);
    void allDone();
    void btnClicked(QAbstractButton *btn);
    void checkAccept();

protected:
    void readConfig();
    void writeConfig();

protected:
    QLabel *labelForTag(const QString &tag);

private:
    std::shared_ptr<TaskCollection> m_tasks;
    QProgressBar *m_progressBar;
    QHash<QString, QLabel *> m_progressLabelByTag;
    QVBoxLayout *m_progressLabelLayout;
    int m_lastErrorItemIndex;
    ResultListWidget *m_resultList;
    FileNameRequester *m_outputLocationFNR;
    QDialogButtonBox::StandardButton m_saveButton = QDialogButtonBox::NoButton;
    QDialogButtonBox *const m_buttonBox;
};

} // namespace Gui
} //namespace Crypto;
} // namespace Kleo

