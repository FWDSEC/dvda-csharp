FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# Install build dependencies
RUN apt-get update && apt-get install -y \
    cmake \
    g++ \
    qt6-base-dev \
    libqt6sql6-sqlite \
    libssl-dev \
    file \
    binutils \
    && rm -rf /var/lib/apt/lists/*

# Copy source code
WORKDIR /app
COPY CMakeLists.txt .
COPY src/ src/
COPY config/ config/

# Build the application
RUN mkdir build && cd build && \
    cmake .. 2>&1 && \
    cmake --build . 2>&1

# Verify the binary
RUN echo "=== BINARY TYPE ===" && \
    file build/dvda && \
    echo "=== SYMBOLS (Challenge 14) ===" && \
    nm build/dvda 2>/dev/null | grep -ic "password\|login\|decrypt" || true && \
    echo "=== HARDCODED STRINGS (Challenge 10) ===" && \
    strings build/dvda | grep -E "p@ssw0rd|admin123|J8gLXc" || true && \
    echo "=== HELPER LIBRARY ===" && \
    file build/libdvda_helper.so && \
    echo "=== BUILD SUCCESS ==="
