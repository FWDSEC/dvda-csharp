#include "sessionmanager.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <iostream>

QString SessionManager::sessionFilePath()
{
    return QDir::homePath() + "/.dvda/session.ini";
}

// ============================================================
// ============================================================
void SessionManager::saveSession(const QString& username, const QString& password,
                                  const QString& email, bool isLoggedIn)
{
    // Ensure directory exists
    QDir dir(QDir::homePath() + "/.dvda");
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QFile file(sessionFilePath());
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "username=" << username << "\n";
        out << "password=" << password << "\n";  // PASSWORD IN PLAINTEXT!
        out << "email=" << email << "\n";
        out << "isLoggedIn=" << (isLoggedIn ? "true" : "false") << "\n";
        file.close();
    }

    std::cout << "[DVDA] Session saved for user: " << username.toStdString() << std::endl;
    std::cout << "[DVDA] Session file: " << sessionFilePath().toStdString() << std::endl;
}

static QString readSessionValue(const QString& filePath, const QString& key)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return "null";
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.startsWith(key + "=")) {
            file.close();
            return line.mid(key.length() + 1);  // Return everything after "key="
        }
    }
    file.close();
    return "null";
}

QString SessionManager::getUsername()
{
    return readSessionValue(sessionFilePath(), "username");
}

QString SessionManager::getPassword()
{
    return readSessionValue(sessionFilePath(), "password");
}

QString SessionManager::getEmail()
{
    return readSessionValue(sessionFilePath(), "email");
}

bool SessionManager::isLoggedIn()
{
    return readSessionValue(sessionFilePath(), "isLoggedIn") == "true";
}

// The file remains on disk with the "null" marker values.
// Previous credential values may survive in filesystem journal/slack space.
void SessionManager::clearSession()
{
    QFile file(sessionFilePath());
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "username=null\n";
        out << "password=null\n";
        out << "email=null\n";
        out << "isLoggedIn=false\n";
        file.close();
    }

    std::cout << "[DVDA] Session cleared" << std::endl;
}
