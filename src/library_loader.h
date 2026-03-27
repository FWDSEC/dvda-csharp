#ifndef LIBRARY_LOADER_H
#define LIBRARY_LOADER_H

#include <QString>

// ============================================================
//
// This class loads a shared library from the CURRENT WORKING
// DIRECTORY without any path validation or signature verification.
// An attacker can hijack execution by placing a malicious library.
// ============================================================

class LibraryLoader
{
public:
    // Attempts to load libdvda_helper from CWD
    static bool loadHelperLibrary();

    // Get the version string from the loaded library
    static QString getHelperVersion();

    // Initialize the helper
    static bool initHelper();

    // Generate a report via the helper
    static QString generateReport();

private:
    static void* s_libraryHandle;
};

#endif // LIBRARY_LOADER_H
