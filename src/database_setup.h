#ifndef DATABASE_SETUP_H
#define DATABASE_SETUP_H

#include <QString>

namespace DatabaseSetup {
    // Creates the SQLite database file and tables if they don't exist.
    // Seeds default users on first run.
    bool initializeDatabase(const QString& dbPath);
}

#endif // DATABASE_SETUP_H
