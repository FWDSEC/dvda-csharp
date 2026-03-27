#include "library_loader.h"
#include "insecure_logger.h"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

void* LibraryLoader::s_libraryHandle = nullptr;

// ============================================================
//
// The dlopen() call uses a relative path, so the dynamic linker
// searches the current working directory first. An attacker can:
//   1. Place a malicious libdvda_helper.dylib in the app directory
//   2. Run the app from that directory
//   3. The malicious library executes with app privileges
//
// No signature verification is performed.
// No path validation is performed.
// No integrity check is performed.
// ============================================================
bool LibraryLoader::loadHelperLibrary()
{
    InsecureLogger::instance().log("LIBRARY", "Attempting to load helper library...");

#ifdef _WIN32
    s_libraryHandle = LoadLibraryA("dvda_helper.dll");
    if (!s_libraryHandle) {
        DWORD error = GetLastError();
        InsecureLogger::instance().log("LIBRARY",
            QString("Failed to load dvda_helper.dll (error code: %1)").arg(error));
        std::cerr << "[DVDA] WARNING: Could not load dvda_helper.dll" << std::endl;
        return false;
    }
#elif defined(__APPLE__)
    // No RTLD_FIRST flag, no absolute path, no signature verification
    s_libraryHandle = dlopen("libdvda_helper.dylib", RTLD_LAZY);
    if (!s_libraryHandle) {
        InsecureLogger::instance().log("LIBRARY",
            QString("Failed to load libdvda_helper.dylib: %1").arg(dlerror()));
        std::cerr << "[DVDA] WARNING: Could not load libdvda_helper.dylib: "
                  << dlerror() << std::endl;
        std::cerr << "[DVDA] Continuing without helper library..." << std::endl;
        return false;
    }
#else
    s_libraryHandle = dlopen("libdvda_helper.so", RTLD_LAZY);
    if (!s_libraryHandle) {
        InsecureLogger::instance().log("LIBRARY",
            QString("Failed to load libdvda_helper.so: %1").arg(dlerror()));
        std::cerr << "[DVDA] WARNING: Could not load libdvda_helper.so: "
                  << dlerror() << std::endl;
        std::cerr << "[DVDA] Continuing without helper library..." << std::endl;
        return false;
    }
#endif

    InsecureLogger::instance().log("LIBRARY",
        "Helper library loaded successfully (NO SIGNATURE VERIFICATION PERFORMED)");
    std::cout << "[DVDA] Helper library loaded from CWD — NO INTEGRITY CHECK" << std::endl;

    return true;
}

QString LibraryLoader::getHelperVersion()
{
    if (!s_libraryHandle) {
        return "Helper library not loaded";
    }

#ifdef _WIN32
    typedef const char* (*VersionFunc)();
    VersionFunc func = (VersionFunc)GetProcAddress((HMODULE)s_libraryHandle, "dvda_helper_version");
#else
    typedef const char* (*VersionFunc)();
    VersionFunc func = (VersionFunc)dlsym(s_libraryHandle, "dvda_helper_version");
#endif

    if (!func) {
        return "Could not find dvda_helper_version symbol";
    }

    const char* version = func();
    InsecureLogger::instance().log("LIBRARY", QString("Helper version: %1").arg(version));
    return QString(version);
}

bool LibraryLoader::initHelper()
{
    if (!s_libraryHandle) {
        return false;
    }

#ifdef _WIN32
    typedef int (*InitFunc)();
    InitFunc func = (InitFunc)GetProcAddress((HMODULE)s_libraryHandle, "dvda_helper_init");
#else
    typedef int (*InitFunc)();
    InitFunc func = (InitFunc)dlsym(s_libraryHandle, "dvda_helper_init");
#endif

    if (!func) {
        std::cerr << "[DVDA] Could not find dvda_helper_init symbol" << std::endl;
        return false;
    }

    int result = func();
    return (result == 0);
}

QString LibraryLoader::generateReport()
{
    if (!s_libraryHandle) {
        return "Helper library not loaded — cannot generate report";
    }

#ifdef _WIN32
    typedef const char* (*ReportFunc)();
    ReportFunc func = (ReportFunc)GetProcAddress((HMODULE)s_libraryHandle, "dvda_helper_generate_report");
#else
    typedef const char* (*ReportFunc)();
    ReportFunc func = (ReportFunc)dlsym(s_libraryHandle, "dvda_helper_generate_report");
#endif

    if (!func) {
        return "Could not find dvda_helper_generate_report symbol";
    }

    const char* report = func();
    InsecureLogger::instance().log("LIBRARY", QString("Report: %1").arg(report));
    return QString(report);
}
