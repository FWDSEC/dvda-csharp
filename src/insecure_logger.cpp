#include "insecure_logger.h"
#include <QDir>
#include <QDateTime>
#include <iostream>

InsecureLogger& InsecureLogger::instance()
{
    static InsecureLogger inst;
    return inst;
}

InsecureLogger::InsecureLogger()
    : m_fileOpen(false)
{
    // with no access control restrictions
    QString logDir = QDir::homePath() + "/.dvda";
    QDir dir(logDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QString logPath = logDir + "/app.log";
    m_logFile.setFileName(logPath);

    // Never rotated, never cleaned up, accumulates sensitive data over time
    if (m_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        m_logStream.setDevice(&m_logFile);
        m_fileOpen = true;

        // Log startup marker
        m_logStream << "\n========================================\n";
        m_logStream << "DVDA Application Started: "
                    << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << "\n";
        m_logStream << "========================================\n";
        m_logStream.flush();
    }
}

InsecureLogger::~InsecureLogger()
{
    if (m_fileOpen) {
        m_logStream << "DVDA Application Closed: "
                    << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << "\n";
        m_logStream.flush();
        m_logFile.close();
    }
}

QString InsecureLogger::logFilePath() const
{
    return QDir::homePath() + "/.dvda/app.log";
}

void InsecureLogger::log(const QString& component, const QString& message)
{
    QMutexLocker locker(&m_mutex);

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    QString logLine = QString("[%1] [%2] %3").arg(timestamp, component, message);

    // Write to stdout
    std::cout << logLine.toStdString() << std::endl;

    if (m_fileOpen) {
        m_logStream << logLine << "\n";
        m_logStream.flush();  // Flush immediately so data is always on disk
    }
}

// exploitation even easier. The log also serves as evidence of any exploitation.
void InsecureLogger::logSQL(const QString& query)
{
    log("SQL", "QUERY: " + query);
}

// Labels like "DB Password", "AES Key" make it trivial to find sensitive data
void InsecureLogger::logCredential(const QString& label, const QString& value)
{
    log("CREDENTIAL", label + ": " + value);
}

void InsecureLogger::logConnection(const QString& connectionString)
{
    log("CONNECTION", "Connection String: " + connectionString);
}

// Even if the network capture misses them, the log file has them
void InsecureLogger::logFTP(const QString& server, const QString& username, const QString& password)
{
    log("FTP", "Server: " + server + " | Username: " + username + " | Password: " + password);
}

void InsecureLogger::logSessionData(const QString& username, const QString& email, const QString& password)
{
    log("SESSION", "User: " + username + " | Email: " + email + " | Password: " + password);
}
