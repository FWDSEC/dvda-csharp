#include "adminwindow.h"
#include "dbaccess.h"
#include "insecure_logger.h"
#include "memory_vulnerabilities.h"
#include "network_manager.h"
#include "library_loader.h"

#include <QVBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QThread>
#include <QSettings>
#include <QCoreApplication>
#include <QUrl>
#include <iostream>

AdminWindow::AdminWindow(QWidget* parent)
    : QWidget(parent)
{
    setWindowTitle("DVDA - Admin Panel");
    setMinimumSize(500, 350);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QLabel* titleLabel = new QLabel("Admin Panel");
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont;
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);

    mainLayout->addSpacing(20);

    QGroupBox* ftpGroup = new QGroupBox("FTP Data Export");
    QVBoxLayout* ftpLayout = new QVBoxLayout(ftpGroup);

    QLabel* descLabel = new QLabel("Export all user expenses to FTP server:");
    ftpLayout->addWidget(descLabel);

    m_ftpBtn = new QPushButton("Upload to FTP Server");
    ftpLayout->addWidget(m_ftpBtn);

    m_statusLabel = new QLabel("");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    ftpLayout->addWidget(m_statusLabel);

    mainLayout->addWidget(ftpGroup);

    QGroupBox* helperGroup = new QGroupBox("Helper Library (DLL Hijacking Demo)");
    QVBoxLayout* helperLayout = new QVBoxLayout(helperGroup);

    QString helperVersion = LibraryLoader::getHelperVersion();
    QLabel* helperLabel = new QLabel("Loaded Library: " + helperVersion);
    helperLayout->addWidget(helperLabel);

    QString report = LibraryLoader::generateReport();
    QLabel* reportLabel = new QLabel("Report: " + report);
    reportLabel->setWordWrap(true);
    helperLayout->addWidget(reportLabel);

    mainLayout->addWidget(helperGroup);

    mainLayout->addStretch();

    connect(m_ftpBtn, &QPushButton::clicked, this, &AdminWindow::onFtpUpload);
}

// ============================================================
// ============================================================
void AdminWindow::onFtpUpload()
{
    m_statusLabel->setText("Please wait while uploading your data...");

    // Read FTP server from config
    QString configPath = QCoreApplication::applicationDirPath() + "/config/app.conf";
    QSettings settings(configPath, QSettings::IniFormat);
    QString ftpserver = settings.value("FTP/FTPSERVER").toString();

    InsecureLogger::instance().log("ADMIN", "Starting FTP export...");

    // Fetch all expenses
    DBAccess db;
    db.openConnection();
    QVector<QVariantMap> allExpenses = db.getExpensesOfAll();
    db.closeConnection();

    // Write to temp CSV
    QString tempPath = QDir::tempPath() + "/ftp-admin.csv";

    QFile file(tempPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Could not create temp file");
        m_statusLabel->setText("Upload failed");
        return;
    }

    QTextStream out(&file);
    out << "id,email,item,price,date,time\n";
    for (const auto& row : allExpenses) {
        out << "\"" << row["id"].toString() << "\","
            << "\"" << row["email"].toString() << "\","
            << "\"" << row["item"].toString() << "\","
            << "\"" << row["price"].toString() << "\","
            << "\"" << row["date"].toString() << "\","
            << "\"" << row["time"].toString() << "\"\n";
    }
    file.close();

    InsecureLogger::instance().log("ADMIN", "Temp CSV created at: " + tempPath);

    MemoryVulnerabilities::instance().storeFTPCredentials(
        ftpserver.toStdString(), "dvda", "p@ssw0rd");

    NetworkManager::ftpUpload(ftpserver, "dvda", "p@ssw0rd", tempPath, "admin.csv");

    m_statusLabel->setText("Upload complete (check console/log for details)");
    InsecureLogger::instance().log("ADMIN", "FTP export completed");
}

// These string literals are visible with `strings` on the compiled binary
void AdminWindow::uploadToFtp(const QString& ftpServer, const QString& username,
                               const QString& password, const QString& filename)
{
    InsecureLogger::instance().logFTP(ftpServer, username, password);

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        InsecureLogger::instance().log("FTP", "ERROR: Could not open file for upload");
        return;
    }

    QUrl url(ftpServer + "/admin.csv");
    url.setUserName(username);
    url.setPassword(password);

    InsecureLogger::instance().log("FTP", "FTP URL: " + url.toString());

    std::cout << "[DVDA] FTP upload simulated — in production, all data sent in cleartext"
              << std::endl;

    file.close();
}
