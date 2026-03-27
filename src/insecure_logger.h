#ifndef INSECURE_LOGGER_H
#define INSECURE_LOGGER_H

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QMutex>

// ============================================================
//
// This logger writes ALL application events — including SQL queries,
// credentials, connection strings, and decrypted passwords — to both
//
// Attack surface:
//   - Log file is world-readable by default
//   - Contains full SQL queries (with injected user input)
//   - Contains decrypted database passwords
//   - Contains FTP credentials
//   - Contains user session data
//   - Log file is never rotated or cleaned up
//
// Real-world impact: An attacker with filesystem access can read
// the log file to extract all credentials and understand the full
// application flow, including exact SQL queries being executed.
// ============================================================

class InsecureLogger
{
public:
    static InsecureLogger& instance();

    // Log a message to both stdout and the log file
    void log(const QString& component, const QString& message);

    // Convenience methods for common log types
    void logSQL(const QString& query);
    void logCredential(const QString& label, const QString& value);
    void logConnection(const QString& connectionString);
    void logFTP(const QString& server, const QString& username, const QString& password);
    void logSessionData(const QString& username, const QString& email, const QString& password);

    QString logFilePath() const;

private:
    InsecureLogger();
    ~InsecureLogger();

    // Non-copyable
    InsecureLogger(const InsecureLogger&) = delete;
    InsecureLogger& operator=(const InsecureLogger&) = delete;

    QFile m_logFile;
    QTextStream m_logStream;
    QMutex m_mutex;
    bool m_fileOpen;
};

#endif // INSECURE_LOGGER_H
