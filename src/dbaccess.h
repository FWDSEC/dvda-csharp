#ifndef DBACCESS_H
#define DBACCESS_H

#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVector>
#include <QVariantMap>

// No parameterized queries are used anywhere in this class.

class DBAccess
{
public:
    DBAccess();
    ~DBAccess();

    // Connection management
    void openConnection();
    void closeConnection();

    // Decrypted password is logged to stdout.
    QString decryptPassword();

    // Authentication
    // Example attack: username = "' OR '1'='1" bypasses authentication
    QVector<QVariantMap> checkLogin(const QString& username, const QString& password);

    // Registration

    bool registerUser(const QString& username, const QString& password, const QString& email);

    // Expense operations
    bool addExpenses(const QString& date, const QString& item,
                     const QString& price, const QString& email, const QString& time);
    QVector<QVariantMap> viewExpenses(const QString& email);
    bool clearExpenses(const QString& email);
    QVector<QVariantMap> getExpensesOfAll();

private:
    QSqlDatabase m_db;

    QString m_decryptedPassword;
};

#endif // DBACCESS_H
