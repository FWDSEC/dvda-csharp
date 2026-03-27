#include "dbaccess.h"
#include "insecure_logger.h"
#include "memory_vulnerabilities.h"
#include <QSettings>
#include <QCoreApplication>
#include <QSqlError>
#include <QDir>
#include <iostream>

#include <openssl/evp.h>
#include <openssl/aes.h>
#include <QByteArray>

DBAccess::DBAccess()
{
}

DBAccess::~DBAccess()
{
    if (m_db.isOpen()) {
        closeConnection();
    }
}

// ============================================================
// Anyone with access to the config file can decrypt the password.
// ============================================================
QString DBAccess::decryptPassword()
{

    QString configPath = QCoreApplication::applicationDirPath() + "/config/app.conf";
    QSettings settings(configPath, QSettings::IniFormat);

    QString dbpassword = settings.value("Database/DBPASSWORD").toString();
    QString key = settings.value("Database/AESKEY").toString();
    QString iv = settings.value("Database/IV").toString();

    InsecureLogger::instance().logCredential("Encrypted DB Password", dbpassword);
    InsecureLogger::instance().logCredential("AES Key", key);
    InsecureLogger::instance().logCredential("IV", iv);

    MemoryVulnerabilities::instance().storeAESKey(key.toStdString(), iv.toStdString());

    QByteArray encryptedBytes = QByteArray::fromBase64(dbpassword.toLatin1());
    QByteArray keyBytes = key.toLatin1();
    QByteArray ivBytes = iv.toLatin1();

    // AES-256-CBC decryption using OpenSSL
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        InsecureLogger::instance().log("CRYPTO", "ERROR: Failed to create cipher context");
        return "";
    }

    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
                       reinterpret_cast<const unsigned char*>(keyBytes.constData()),
                       reinterpret_cast<const unsigned char*>(ivBytes.constData()));

    int outLen = encryptedBytes.size() + AES_BLOCK_SIZE;
    unsigned char* outBuf = new unsigned char[outLen];
    int finalLen = 0;

    EVP_DecryptUpdate(ctx, outBuf, &outLen,
                      reinterpret_cast<const unsigned char*>(encryptedBytes.constData()),
                      encryptedBytes.size());

    EVP_DecryptFinal_ex(ctx, outBuf + outLen, &finalLen);

    EVP_CIPHER_CTX_free(ctx);

    m_decryptedPassword = QString::fromLatin1(reinterpret_cast<char*>(outBuf), outLen + finalLen);
    delete[] outBuf;

    InsecureLogger::instance().logCredential("Decrypted DB Password", m_decryptedPassword);

    MemoryVulnerabilities::instance().storeDBPassword(m_decryptedPassword.toStdString());

    return m_decryptedPassword;
}

// ============================================================
// ============================================================
void DBAccess::openConnection()
{
    QString configPath = QCoreApplication::applicationDirPath() + "/config/app.conf";
    QSettings settings(configPath, QSettings::IniFormat);

    QString dbserver = settings.value("Database/DBSERVER").toString();
    QString dbname = settings.value("Database/DBNAME").toString();
    QString dbusername = settings.value("Database/DBUSERNAME").toString();

    // For the Qt/SQLite port, we use a local SQLite file
    QString dbPath = QDir::homePath() + "/.dvda/dvda.db";

    InsecureLogger::instance().logCredential("DB Server", dbserver);
    InsecureLogger::instance().logCredential("DB Name", dbname);
    InsecureLogger::instance().logCredential("DB Username", dbusername);

    QString decryptedPw = decryptPassword();

    QString connectionStr = "Data Source=" + dbserver + "; Database=" + dbname
                          + "; User=" + dbusername + "; Password=" + decryptedPw;
    InsecureLogger::instance().logConnection(connectionStr);

    MemoryVulnerabilities::instance().storeConnectionString(connectionStr.toStdString());

    if (QSqlDatabase::contains("dvda_connection")) {
        m_db = QSqlDatabase::database("dvda_connection");
    } else {
        m_db = QSqlDatabase::addDatabase("QSQLITE", "dvda_connection");
        m_db.setDatabaseName(dbPath);
    }

    if (!m_db.open()) {
        InsecureLogger::instance().log("DB", "ERROR: Could not open database: "
            + m_db.lastError().text());
    }
}

void DBAccess::closeConnection()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

// ============================================================
// ============================================================
QVector<QVariantMap> DBAccess::checkLogin(const QString& username, const QString& password)
{
    QVector<QVariantMap> results;

    QString sqlcmd = "SELECT * FROM users WHERE username='" + username
                   + "' AND password='" + password + "'";

    InsecureLogger::instance().logSQL(sqlcmd);

    MemoryVulnerabilities::instance().addToLoginHistory(
        username.toStdString(), password.toStdString());

    QSqlQuery query(m_db);
    query.exec(sqlcmd);

    while (query.next()) {
        QVariantMap row;
        row["id"] = query.value(0);
        row["username"] = query.value(1);
        row["password"] = query.value(2);
        row["email"] = query.value(3);
        row["isadmin"] = query.value(4);
        results.append(row);

        MemoryVulnerabilities::instance().storeUserCredentials(
            query.value(1).toString(),
            query.value(2).toString(),
            query.value(3).toString());

        InsecureLogger::instance().logSessionData(
            query.value(1).toString(),
            query.value(3).toString(),
            query.value(2).toString());
    }

    return results;
}

// ============================================================
// ============================================================
bool DBAccess::registerUser(const QString& username, const QString& password, const QString& email)
{
    QString sqlquery = "INSERT INTO users (username, password, email, isadmin) VALUES ('"
                     + username + "', '" + password + "', '" + email + "', 0)";

    InsecureLogger::instance().logSQL(sqlquery);

    QSqlQuery query(m_db);
    bool ok = query.exec(sqlquery);

    if (!ok) {
        InsecureLogger::instance().log("DB", "Registration error: "
            + query.lastError().text());
    }

    return ok;
}

// ============================================================
// ============================================================
bool DBAccess::addExpenses(const QString& date, const QString& item,
                            const QString& price, const QString& email, const QString& time)
{
    QString sqlquery = "INSERT INTO expenses (email, item, price, date, time) VALUES ('"
                     + email + "', '" + item + "', '" + price + "', '" + date + "', '" + time + "')";

    InsecureLogger::instance().logSQL(sqlquery);

    QSqlQuery query(m_db);
    bool ok = query.exec(sqlquery);

    if (!ok) {
        InsecureLogger::instance().log("DB", "Add expense error: "
            + query.lastError().text());
    }

    return ok;
}

// ============================================================
// ============================================================
QVector<QVariantMap> DBAccess::viewExpenses(const QString& email)
{
    QVector<QVariantMap> results;

    QString sqlcmd = "SELECT item, price, date, time FROM expenses WHERE email='" + email + "'";

    InsecureLogger::instance().logSQL(sqlcmd);

    QSqlQuery query(m_db);
    query.exec(sqlcmd);

    while (query.next()) {
        QVariantMap row;
        row["item"] = query.value(0);
        row["price"] = query.value(1);
        row["date"] = query.value(2);
        row["time"] = query.value(3);
        results.append(row);
    }

    return results;
}

// ============================================================
// ============================================================
bool DBAccess::clearExpenses(const QString& email)
{
    QString sqlcmd = "DELETE FROM expenses WHERE email='" + email + "'";

    InsecureLogger::instance().logSQL(sqlcmd);

    QSqlQuery query(m_db);
    bool ok = query.exec(sqlcmd);

    if (!ok) {
        InsecureLogger::instance().log("DB", "Clear expense error: "
            + query.lastError().text());
    }

    return ok;
}

QVector<QVariantMap> DBAccess::getExpensesOfAll()
{
    QVector<QVariantMap> results;

    QString sqlcmd = "SELECT * FROM expenses";
    InsecureLogger::instance().logSQL(sqlcmd);

    QSqlQuery query(m_db);
    query.exec(sqlcmd);

    while (query.next()) {
        QVariantMap row;
        row["id"] = query.value(0);
        row["email"] = query.value(1);
        row["item"] = query.value(2);
        row["price"] = query.value(3);
        row["date"] = query.value(4);
        row["time"] = query.value(5);
        results.append(row);
    }

    return results;
}
