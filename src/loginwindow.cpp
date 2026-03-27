#include "loginwindow.h"
#include "dbaccess.h"
#include "sessionmanager.h"
#include "registerwindow.h"
#include "mainwindow.h"
#include "adminwindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QGroupBox>
#include <QSettings>
#include <QCoreApplication>
#include <QApplication>
#include <QTimer>
#include <iostream>

// Platform-specific includes for debugger detection
#ifdef __APPLE__
#include <sys/types.h>
#include <sys/ptrace.h>
#include <unistd.h>
#elif defined(__linux__)
#include <sys/ptrace.h>
#include <unistd.h>
#elif defined(_WIN32)
#include <windows.h>
#endif

LoginWindow::LoginWindow(QWidget* parent)
    : QWidget(parent)
{
    // ============================================================
    // or attaching the debugger after startup
    // ============================================================
    if (isBeingDebugged()) {
        std::cout << "[DVDA] Debugger detected! Exiting..." << std::endl;
        QTimer::singleShot(0, qApp, &QApplication::quit);
        return;
    }

    setWindowTitle("DVDA - Damn Vulnerable Thick Client Application (C++/Qt)");
    setMinimumSize(450, 400);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QLabel* titleLabel = new QLabel("DVDA Login");
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont;
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);

    QLabel* subtitleLabel = new QLabel("Damn Vulnerable Thick Client Application");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(subtitleLabel);

    mainLayout->addSpacing(20);

    QGroupBox* loginGroup = new QGroupBox("Login");
    QVBoxLayout* loginLayout = new QVBoxLayout(loginGroup);

    loginLayout->addWidget(new QLabel("Username:"));
    m_usernameEdit = new QLineEdit();
    m_usernameEdit->setPlaceholderText("Enter username");
    loginLayout->addWidget(m_usernameEdit);

    loginLayout->addWidget(new QLabel("Password:"));
    m_passwordEdit = new QLineEdit();
    m_passwordEdit->setPlaceholderText("Enter password");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    loginLayout->addWidget(m_passwordEdit);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    m_loginBtn = new QPushButton("Login");
    m_registerBtn = new QPushButton("Register");
    btnLayout->addWidget(m_loginBtn);
    btnLayout->addWidget(m_registerBtn);
    loginLayout->addLayout(btnLayout);

    mainLayout->addWidget(loginGroup);

    QGroupBox* configGroup = new QGroupBox("Server Configuration");
    QVBoxLayout* configLayout = new QVBoxLayout(configGroup);

    configLayout->addWidget(new QLabel("Server IP:"));
    m_serverEdit = new QLineEdit();
    m_serverEdit->setPlaceholderText("e.g. 192.168.56.110");
    configLayout->addWidget(m_serverEdit);

    m_configureBtn = new QPushButton("Configure Server");
    configLayout->addWidget(m_configureBtn);

    mainLayout->addWidget(configGroup);

    mainLayout->addStretch();

    // Connect signals
    connect(m_loginBtn, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    connect(m_registerBtn, &QPushButton::clicked, this, &LoginWindow::onRegisterClicked);
    connect(m_configureBtn, &QPushButton::clicked, this, &LoginWindow::onConfigureClicked);
}

// ============================================================
// Example: username = "' OR '1'='1" and any password
// ============================================================
void LoginWindow::onLoginClicked()
{
    QString username = m_usernameEdit->text().trimmed();
    QString password = m_passwordEdit->text().trimmed();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter all the fields!");
        return;
    }

    DBAccess db;
    db.openConnection();

    QVector<QVariantMap> results = db.checkLogin(username, password);

    if (!results.isEmpty()) {
        QString user;
        QString pass;
        QString email;
        int isadmin = 0;

        for (const auto& row : results) {
            user = row["username"].toString();
            pass = row["password"].toString();
            email = row["email"].toString();
            isadmin = row["isadmin"].toInt();

            if (user != "admin") {
                SessionManager::saveSession(user, pass, email, true);
            }
        }

        m_usernameEdit->clear();
        m_passwordEdit->clear();

        db.closeConnection();

        if (isadmin != 1) {
            // Regular user — show Main window
            this->hide();
            MainWindow* mainWin = new MainWindow();
            mainWin->setAttribute(Qt::WA_DeleteOnClose);
            connect(mainWin, &MainWindow::destroyed, qApp, &QApplication::quit);
            mainWin->show();
        } else {
            // Admin user — show Admin window
            this->hide();
            AdminWindow* adminWin = new AdminWindow();
            adminWin->setAttribute(Qt::WA_DeleteOnClose);
            connect(adminWin, &AdminWindow::destroyed, qApp, &QApplication::quit);
            adminWin->show();
        }
    } else {
        QMessageBox::warning(this, "Login Failed", "Invalid Login");
        m_usernameEdit->clear();
        m_passwordEdit->clear();
        db.closeConnection();
    }
}

void LoginWindow::onRegisterClicked()
{
    RegisterWindow* regWin = new RegisterWindow(this);
    regWin->setAttribute(Qt::WA_DeleteOnClose);
    regWin->show();
}

// ============================================================
// Config file contains AES keys, DB credentials, etc.
// ============================================================
void LoginWindow::onConfigureClicked()
{
    QString serverIp = m_serverEdit->text().trimmed();
    if (serverIp.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter a server IP!");
        return;
    }

    QString configPath = QCoreApplication::applicationDirPath() + "/config/app.conf";
    QSettings settings(configPath, QSettings::IniFormat);

    // Update DB server
    QString dbserver = serverIp + "\\SQLEXPRESS";
    settings.setValue("Database/DBSERVER", dbserver);

    // Update FTP server
    settings.setValue("FTP/FTPSERVER", serverIp);

    settings.sync();
    m_serverEdit->clear();

    QMessageBox::information(this, "Success", "Server successfully configured");
    std::cout << "[DVDA] Server configured to: " << serverIp.toStdString() << std::endl;
}

// ============================================================
//   - Binary patching (NOP out the check)
//   - LD_PRELOAD / DYLD_INSERT_LIBRARIES
//   - Attaching debugger after startup
//   - Running under a modified ptrace
// ============================================================
bool LoginWindow::isBeingDebugged()
{
#ifdef __APPLE__
    // macOS: Try to ptrace ourselves. If a debugger is attached, this fails.
    if (ptrace(PT_DENY_ATTACH, 0, 0, 0) == -1) {
        return true;
    }
    return false;
#elif defined(__linux__)
    // Linux: Check /proc/self/status for TracerPid
    if (ptrace(PTRACE_TRACEME, 0, nullptr, nullptr) == -1) {
        return true;
    }
    // Detach immediately so normal operation continues
    ptrace(PTRACE_DETACH, 0, nullptr, nullptr);
    return false;
#elif defined(_WIN32)
    return IsDebuggerPresent();
#else
    return false;
#endif
}
