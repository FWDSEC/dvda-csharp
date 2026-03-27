#include "network_manager.h"
#include "insecure_logger.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSslConfiguration>
#include <QSslSocket>
#include <QEventLoop>
#include <QUrl>
#include <QFile>
#include <QHostInfo>
#include <iostream>

// ============================================================
// Wireshark filter: ftp || tcp.port == 21
//
// FTP protocol flow visible in packet capture:
//   220 FTP Server Ready
//   331 Password required
//   230 Login successful
//   STOR admin.csv              ← filename visible
//   [raw CSV data follows]      ← all data unencrypted
// ============================================================
bool NetworkManager::ftpUpload(const QString& server, const QString& username,
                                const QString& password, const QString& localFile,
                                const QString& remoteFile)
{
    InsecureLogger::instance().log("NETWORK",
        "FTP Upload: " + server + " (PLAINTEXT — NOT SFTP/FTPS)");
    InsecureLogger::instance().logFTP(server, username, password);

    QUrl ftpUrl;
    ftpUrl.setScheme("ftp");           // NOT ftps or sftp
    ftpUrl.setHost(server);
    ftpUrl.setPort(21);                // Standard FTP port — unencrypted
    ftpUrl.setUserName(username);      // Sent as FTP USER command
    ftpUrl.setPassword(password);      // Sent as FTP PASS command
    ftpUrl.setPath("/" + remoteFile);

    InsecureLogger::instance().log("NETWORK",
        "FTP URL: " + ftpUrl.toString());
    std::cout << "[DVDA-NET] FTP URL (with creds): " << ftpUrl.toString().toStdString() << std::endl;

    QFile file(localFile);
    if (!file.open(QIODevice::ReadOnly)) {
        std::cerr << "[DVDA-NET] Cannot open file for upload: "
                  << localFile.toStdString() << std::endl;
        return false;
    }

    QByteArray fileData = file.readAll();
    file.close();

    // Qt6 doesn't have built-in FTP client, simulating the upload
    // In a real scenario with libcurl/custom FTP:
    std::cout << "[DVDA-NET] FTP STOR " << remoteFile.toStdString()
              << " (" << fileData.size() << " bytes) — ALL DATA IN CLEARTEXT" << std::endl;

    InsecureLogger::instance().log("NETWORK",
        QString("FTP upload simulated: %1 bytes sent over CLEARTEXT FTP")
        .arg(fileData.size()));

    return true;
}

// ============================================================
// All request headers, URLs, and response data are unencrypted.
// Wireshark filter: http || tcp.port == 80
// ============================================================
QString NetworkManager::httpGet(const QString& url)
{
    InsecureLogger::instance().log("NETWORK", "HTTP GET: " + url + " (PLAINTEXT — NOT HTTPS)");

    QNetworkAccessManager manager;

    disableSSLVerification();

    QUrl requestUrl(url);
    QNetworkRequest request(requestUrl);

    request.setRawHeader("User-Agent", "DVDA/2.0 (Damn Vulnerable Thick Client)");

    QNetworkReply* reply = manager.get(request);

    // Wait for reply
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    QString response;
    if (reply->error() == QNetworkReply::NoError) {
        response = QString(reply->readAll());
        InsecureLogger::instance().log("NETWORK",
            "HTTP Response: " + response.left(500));
    } else {
        response = "ERROR: " + reply->errorString();
        InsecureLogger::instance().log("NETWORK",
            "HTTP Error: " + reply->errorString());
    }

    reply->deleteLater();
    return response;
}

// ============================================================
// POST body (which may contain credentials) sent unencrypted
// ============================================================
QString NetworkManager::httpPost(const QString& url, const QString& data)
{
    InsecureLogger::instance().log("NETWORK",
        "HTTP POST: " + url + " (PLAINTEXT — NOT HTTPS)");

    InsecureLogger::instance().log("NETWORK", "POST Body: " + data);

    QNetworkAccessManager manager;
    disableSSLVerification();

    QUrl requestUrl(url);
    QNetworkRequest request(requestUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("User-Agent", "DVDA/2.0 (Damn Vulnerable Thick Client)");

    QNetworkReply* reply = manager.post(request, data.toUtf8());

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    QString response;
    if (reply->error() == QNetworkReply::NoError) {
        response = QString(reply->readAll());
    } else {
        response = "ERROR: " + reply->errorString();
    }

    reply->deleteLater();
    return response;
}

// ============================================================
// An attacker with a self-signed certificate can intercept
// all "encrypted" traffic.
//
// ============================================================
void NetworkManager::disableSSLVerification()
{
    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);  // DANGEROUS!
    QSslConfiguration::setDefaultConfiguration(sslConfig);

    InsecureLogger::instance().log("NETWORK",
        "SSL CERTIFICATE VERIFICATION DISABLED — VULNERABLE TO MITM");
    std::cout << "[DVDA-NET] WARNING: SSL verification disabled — MITM possible"
              << std::endl;
}

// DNS resolution — standard system resolver (unencrypted)
QString NetworkManager::resolveHost(const QString& hostname)
{
    InsecureLogger::instance().log("NETWORK",
        "DNS Resolution: " + hostname + " (PLAINTEXT DNS — NOT DoH/DoT)");

    QHostInfo info = QHostInfo::fromName(hostname);
    if (info.error() != QHostInfo::NoError) {
        return "Resolution failed: " + info.errorString();
    }

    QString result;
    for (const auto& addr : info.addresses()) {
        result += addr.toString() + " ";
    }

    InsecureLogger::instance().log("NETWORK",
        "Resolved " + hostname + " → " + result.trimmed());

    return result.trimmed();
}
