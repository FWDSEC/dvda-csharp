#include <QApplication>
#include <QDir>
#include <iostream>

#include "database_setup.h"
#include "loginwindow.h"
#include "insecure_logger.h"
#include "memory_vulnerabilities.h"
#include "library_loader.h"
#include "network_manager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("DVDA");
    app.setApplicationVersion("2.0");

    // ============================================================
    // Reveals exact application name and version to log files
    // ============================================================
    InsecureLogger& logger = InsecureLogger::instance();
    logger.log("STARTUP", "================================================");
    logger.log("STARTUP", "DVDA - Damn Vulnerable Thick Client Application");
    logger.log("STARTUP", "C++/Qt Edition v2.0");
    logger.log("STARTUP", "FOR EDUCATIONAL PURPOSES ONLY");
    logger.log("STARTUP", "================================================");
    logger.log("STARTUP", "Log file: " + logger.logFilePath());

    std::cout << "================================================" << std::endl;
    std::cout << " DVDA - Damn Vulnerable Thick Client Application" << std::endl;
    std::cout << " C++/Qt Edition v2.0" << std::endl;
    std::cout << " FOR EDUCATIONAL PURPOSES ONLY" << std::endl;
    std::cout << "================================================" << std::endl;

    // Initialize SQLite database
    QString dbPath = QDir::homePath() + "/.dvda/dvda.db";
    if (!DatabaseSetup::initializeDatabase(dbPath)) {
        logger.log("STARTUP", "FATAL: Could not initialize database!");
        std::cerr << "FATAL: Could not initialize database!" << std::endl;
        return 1;
    }
    logger.log("STARTUP", "Database initialized at: " + dbPath);

    // ============================================================
    // Load helper library from CWD without verification
    // ============================================================
    logger.log("STARTUP", "Attempting to load helper library from CWD...");
    if (LibraryLoader::loadHelperLibrary()) {
        QString version = LibraryLoader::getHelperVersion();
        logger.log("STARTUP", "Helper library version: " + version);

        if (LibraryLoader::initHelper()) {
            logger.log("STARTUP", "Helper library initialized successfully");
        }
    } else {
        logger.log("STARTUP", "Helper library not found — continuing without it");
    }

    // ============================================================
    // All HTTPS connections will accept any certificate
    // ============================================================
    NetworkManager::disableSSLVerification();
    logger.log("STARTUP", "SSL verification disabled globally");

    // Show login window
    LoginWindow loginWindow;
    loginWindow.show();

    logger.log("STARTUP", "Login window displayed — application ready");
    return app.exec();
}
