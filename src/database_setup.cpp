#include "database_setup.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QFileInfo>
#include <iostream>

namespace DatabaseSetup {

bool initializeDatabase(const QString& dbPath)
{
    // Ensure the directory exists
    QFileInfo fi(dbPath);
    QDir dir = fi.absoluteDir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        std::cerr << "ERROR: Could not open database: "
                  << db.lastError().text().toStdString() << std::endl;
        return false;
    }

    QSqlQuery query(db);

    // Create USERS table
    bool ok = query.exec(
        "CREATE TABLE IF NOT EXISTS users ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  username TEXT NOT NULL,"
        "  password TEXT NOT NULL," 
        "  email TEXT,"
        "  isadmin INTEGER DEFAULT 0"
        ")"
    );
    if (!ok) {
        std::cerr << "ERROR creating users table: "
                  << query.lastError().text().toStdString() << std::endl;
        return false;
    }

    // Create EXPENSES table
    ok = query.exec(
        "CREATE TABLE IF NOT EXISTS expenses ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  email TEXT NOT NULL,"
        "  item TEXT NOT NULL,"
        "  price TEXT NOT NULL,"
        "  date TEXT NOT NULL,"
        "  time TEXT"
        ")"
    );
    if (!ok) {
        std::cerr << "ERROR creating expenses table: "
                  << query.lastError().text().toStdString() << std::endl;
        return false;
    }

    // Seed default users if table is empty
    query.exec("SELECT COUNT(*) FROM users");
    if (query.next() && query.value(0).toInt() == 0) {
        query.exec("INSERT INTO users (username, password, email, isadmin) VALUES "
                   "('admin', 'admin123', 'admin@damnvulnerablethickclientapp.com', 1)");
        query.exec("INSERT INTO users (username, password, email, isadmin) VALUES "
                   "('rebecca', 'rebecca', 'rebecca@test.com', 0)");
        query.exec("INSERT INTO users (username, password, email, isadmin) VALUES "
                   "('raymond', 'raymond', 'raymond@test.com', 0)");

        std::cout << "[DVDA] Database seeded with default users." << std::endl;
    }

    std::cout << "[DVDA] Database initialized at: " << dbPath.toStdString() << std::endl;
    return true;
}

} // namespace DatabaseSetup
