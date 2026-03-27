#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <QString>

// This includes username, password, and email — full credential exposure on disk.

class SessionManager
{
public:

    static void saveSession(const QString& username, const QString& password,
                            const QString& email, bool isLoggedIn);


    static QString getUsername();
    static QString getPassword();
    static QString getEmail();
    static bool isLoggedIn();

    // "Logout" — sets values to "null" strings (doesn't delete them)
    static void clearSession();

    static QString sessionFilePath();
};

#endif // SESSIONMANAGER_H
