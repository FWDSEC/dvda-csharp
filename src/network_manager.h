#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <QString>

// ============================================================
//
// This class handles all network communication using PLAINTEXT
// protocols (HTTP instead of HTTPS, FTP instead of SFTP/FTPS).
//
// Attack surface:
//   - All network traffic is unencrypted
//   - Data exported via FTP is unencrypted
//   - SSL/TLS certificate verification is DISABLED
//
// How to demonstrate:
//   1. Start Wireshark on the network interface
//   2. Trigger any network operation (FTP upload, API call)
//   3. Filter by: ftp || http || tcp.port == 21
//
// Real-world impact:
//   - Man-in-the-Middle (MITM) attacks can intercept all credentials
//   - Network sniffing on shared WiFi exposes everything
//   - ISP or network admin can read all transmitted data
// ============================================================

class NetworkManager
{
public:

    static bool ftpUpload(const QString& server, const QString& username,
                          const QString& password, const QString& localFile,
                          const QString& remoteFile);

    // Data transmitted without encryption
    static QString httpGet(const QString& url);
    static QString httpPost(const QString& url, const QString& data);

    // Even when HTTPS is used, certificates are not validated,
    // making MITM attacks trivial
    static void disableSSLVerification();

    // Server hostname resolution is not encrypted
    static QString resolveHost(const QString& hostname);
};

#endif // NETWORK_MANAGER_H
