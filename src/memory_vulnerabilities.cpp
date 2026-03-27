#include "memory_vulnerabilities.h"
#include <iostream>

MemoryVulnerabilities& MemoryVulnerabilities::instance()
{
    static MemoryVulnerabilities inst;
    return inst;
}

// Even after this variable goes out of scope, the old memory
// is deallocated but NOT zeroed — the data remains in the heap.
void MemoryVulnerabilities::storeDBPassword(const std::string& password)
{
    m_dbPassword = password;
    std::cout << "[DVDA-MEM] DB password stored in memory at address: "
              << static_cast<const void*>(m_dbPassword.c_str()) << std::endl;
}

// QString uses implicit sharing (COW), meaning copies of the password
// may exist in multiple memory locations.
void MemoryVulnerabilities::storeUserCredentials(const QString& username,
                                                   const QString& password,
                                                   const QString& email)
{
    m_currentUsername = username;
    m_currentPassword = password;
    m_currentEmail = email;

    // Also add to login history — credentials ACCUMULATE
    addToLoginHistory(username.toStdString(), password.toStdString());

    std::cout << "[DVDA-MEM] User credentials stored in memory for: "
              << username.toStdString() << std::endl;
}

// attacker can decrypt the database password
void MemoryVulnerabilities::storeAESKey(const std::string& key, const std::string& iv)
{
    m_aesKey = key;
    m_aesIV = iv;
    std::cout << "[DVDA-MEM] AES key stored in memory at address: "
              << static_cast<const void*>(m_aesKey.c_str()) << std::endl;
}

// Also visible as string literals in the binary
void MemoryVulnerabilities::storeFTPCredentials(const std::string& server,
                                                  const std::string& username,
                                                  const std::string& password)
{
    m_ftpServer = server;
    m_ftpUsername = username;
    m_ftpPassword = password;
    std::cout << "[DVDA-MEM] FTP credentials stored in memory for server: "
              << server << std::endl;
}

void MemoryVulnerabilities::storeConnectionString(const std::string& connStr)
{
    m_connectionString = connStr;
    std::cout << "[DVDA-MEM] Connection string stored in memory at address: "
              << static_cast<const void*>(m_connectionString.c_str()) << std::endl;
}

// Even failed login attempts are stored, so brute force attempts
// will have all tried passwords visible in memory
void MemoryVulnerabilities::addToLoginHistory(const std::string& username,
                                                const std::string& password)
{
    m_loginHistory.push_back({username, password});
    std::cout << "[DVDA-MEM] Login history now contains "
              << m_loginHistory.size() << " entries" << std::endl;
}

// It claims to clear sensitive data but actually does nothing useful.
// The string::clear() method deallocates or empties the string BUT
// does not overwrite the underlying memory with zeros.
// The old data remains in freed heap pages and can be recovered.
void MemoryVulnerabilities::secureClear()
{
    // This looks like it clears data, but it doesn't securely erase memory.
    // After clear(), the old char data remains in the heap.
    // A proper implementation would use:
    //   memset_s() or explicit_bzero() before clearing
    //   volatile pointers to prevent compiler optimization

    m_dbPassword.clear();       // Data still in heap!
    m_aesKey.clear();           // Data still in heap!
    m_aesIV.clear();            // Data still in heap!
    m_connectionString.clear(); // Data still in heap!
    m_currentUsername.clear();   // Data still in heap!
    m_currentPassword.clear();  // Data still in heap!
    m_currentEmail.clear();     // Data still in heap!
    m_ftpServer.clear();        // Data still in heap!
    m_ftpUsername.clear();       // Data still in heap!
    m_ftpPassword.clear();      // Data still in heap!

    // Login history entries are also just cleared, not zeroed
    m_loginHistory.clear();     // All old entries still in heap!

    std::cout << "[DVDA-MEM] 'Secure' clear completed (data still in heap memory)"
              << std::endl;
}
