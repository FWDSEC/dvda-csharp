# DVDA — Damn Vulnerable Desktop Application

> A deliberately insecure C++/Qt6 desktop application for learning and teaching application security.

![C++17](https://img.shields.io/badge/C%2B%2B-17-blue)
![Qt6](https://img.shields.io/badge/Qt-6-green)
![License](https://img.shields.io/badge/license-MIT-orange)
![Platform](https://img.shields.io/badge/platform-macOS%20%7C%20Linux%20%7C%20Windows-lightgrey)

## About

DVDA is a **C++/Qt6 port** of the original [Damn Vulnerable Thick Client Application (DVTA)](https://github.com/srini0x00/dvta), which was written in C#/.NET. This port is designed to teach **C++ developers** how to identify and exploit common security vulnerabilities in desktop (thick client) applications.

The application simulates a simple expense tracker with login, registration, admin panel, and data export features — all intentionally built with real-world security flaws.

> ⚠️ **FOR EDUCATIONAL PURPOSES ONLY.** Do not deploy this application in any production environment.

## Challenges

DVDA contains **15 security challenges** mapped to OWASP categories:

| # | Challenge | OWASP Category | Difficulty |
|---|-----------|---------------|-----------|
| 1 | SQL Injection — Authentication Bypass | A03: Injection | ⭐ Easy |
| 2 | SQL Injection — Registration | A03: Injection | ⭐ Easy |
| 3 | SQL Injection — Data Exfiltration | A03: Injection | ⭐⭐ Medium |
| 4 | Insecure Local Data Storage | A02: Cryptographic Failures | ⭐ Easy |
| 5 | Insecure Logging | A09: Security Logging Failures | ⭐ Easy |
| 6 | Weak Cryptography & Exposed Keys | A02: Cryptographic Failures | ⭐⭐ Medium |
| 7 | CSV Injection | A03: Injection | ⭐⭐ Medium |
| 8 | Sensitive Data in Memory | A02: Cryptographic Failures | ⭐⭐⭐ Hard |
| 9 | DLL / Dylib / SO Hijacking | A08: Software Integrity Failures | ⭐⭐ Medium |
| 10 | Hardcoded Credentials | A07: Auth Failures | ⭐ Easy |
| 11 | Cleartext Data in Transit | A02: Cryptographic Failures | ⭐⭐ Medium |
| 12 | Debugger Detection Bypass | A04: Insecure Design | ⭐⭐ Medium |
| 13 | Plaintext Passwords in Database | A02: Cryptographic Failures | ⭐ Easy |
| 14 | Reverse Engineering (No Obfuscation) | A04: Insecure Design | ⭐⭐ Medium |
| 15 | Configuration File Exposure | A05: Security Misconfiguration | ⭐ Easy |

## Prerequisites

| Platform | Requirements |
|----------|-------------|
| **macOS** | Qt6, CMake, OpenSSL (`brew install qt cmake openssl`) |
| **Linux** | Qt6, CMake, OpenSSL (`apt install qt6-base-dev cmake libssl-dev g++`) |
| **Windows** | Qt6, CMake, OpenSSL, MSVC or MinGW |

## Build & Run

```bash
git clone https://github.com/YOUR_USERNAME/dvda.git
cd dvda
mkdir build && cd build
cmake ..
cmake --build .
./dvda
```

### Docker (Linux)

```bash
docker build -t dvda .
docker run --rm dvda
```

## Default Credentials

| Username | Password | Role |
|----------|----------|------|
| admin | admin123 | Admin |
| rebecca | rebecca | User |
| raymond | raymond | User |

## Recommended Tools

| Tool | Purpose |
|------|---------|
| `sqlite3` | Database inspection |
| `strings` / `nm` | Binary analysis |
| `lldb` / `gdb` | Memory forensics & debugging |
| Wireshark / `tcpdump` | Network traffic analysis |
| Ghidra / radare2 | Reverse engineering |

## Credits

- **Original DVTA (C#/.NET):** [srini0x00/dvta](https://github.com/srini0x00/dvta)
- **C++/Qt6 Port:** Ported to teach C++ developers about thick client security vulnerabilities

## License

This project is for **educational purposes only**. Use responsibly.
