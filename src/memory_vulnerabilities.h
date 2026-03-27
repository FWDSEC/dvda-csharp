#ifndef MEMORY_VULNERABILITIES_H
#define MEMORY_VULNERABILITIES_H

#include <string>
#include <QString>
#include <vector>

// ============================================================
//
// This class intentionally stores all credentials and sensitive
// data as persistent member variables that are NEVER zeroed out
// or securely erased from memory.
//
// Attack surface:
//   - All passwords remain in process memory as std::string / QString
//   - Memory is never overwritten with zeros after use
//   - Credentials can be extracted via:
//       * GDB / LLDB memory inspection
//       * Core dumps (if enabled)
//       * Memory forensics tools (e.g., Volatility)
//       * /proc/[pid]/mem on Linux
//       * Process memory scanners
//
// Real-world impact: An attacker with access to the system
// (or a core dump) can extract all credentials from memory,
// even after the user has "logged out" of the application.
//
// How to demonstrate:
//   1. Run dvda and login with any user
//   2. Attach GDB/LLDB: lldb -p $(pgrep dvda)
//   3. Search memory: memory find --string "admin123" -- start end
//   4. All passwords, AES keys, and FTP credentials will be found
// ============================================================

class MemoryVulnerabilities
{
public:
    static MemoryVulnerabilities& instance();

    // Store credentials in memory — NEVER cleared
    void storeDBPassword(const std::string& password);
    void storeUserCredentials(const QString& username, const QString& password, const QString& email);
    void storeAESKey(const std::string& key, const std::string& iv);
    void storeFTPCredentials(const std::string& server, const std::string& username, const std::string& password);
    void storeConnectionString(const std::string& connStr);

    // Append to the history — credentials accumulate over time
    void addToLoginHistory(const std::string& username, const std::string& password);

    // "Secure" clear that doesn't actually clear anything
    void secureClear();

private:
    MemoryVulnerabilities() = default;
    ~MemoryVulnerabilities() = default;

    // Non-copyable
    MemoryVulnerabilities(const MemoryVulnerabilities&) = delete;
    MemoryVulnerabilities& operator=(const MemoryVulnerabilities&) = delete;

    // They are std::string (heap-allocated) and QString (COW/heap), so even if
    // reassigned, the old values may remain in freed heap memory.

    std::string m_dbPassword;           // Database password — never zeroed
    std::string m_aesKey;               // AES encryption key — never zeroed
    std::string m_aesIV;                // AES initialization vector — never zeroed
    std::string m_connectionString;     // Full DB connection string — never zeroed

    QString m_currentUsername;           // Current logged-in username
    QString m_currentPassword;          // Current logged-in password — never zeroed
    QString m_currentEmail;             // Current user email

    // These are also visible in the binary via `strings` command
    std::string m_ftpServer;
    std::string m_ftpUsername;
    std::string m_ftpPassword;           // FTP password — never zeroed

    // Every login attempt's credentials remain in memory forever
    std::vector<std::pair<std::string, std::string>> m_loginHistory;
};

#endif // MEMORY_VULNERABILITIES_H
