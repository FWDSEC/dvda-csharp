cmake_minimum_required(VERSION 3.16)
project(DVDA VERSION 2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================
#
# ============================================================
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O0 -g")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Network)
find_package(OpenSSL REQUIRED)

# --- Main Application ---
set(SOURCES
    src/main.cpp
    src/database_setup.cpp
    src/dbaccess.cpp
    src/sessionmanager.cpp
    src/loginwindow.cpp
    src/registerwindow.cpp
    src/mainwindow.cpp
    src/adminwindow.cpp
    src/insecure_logger.cpp
    src/memory_vulnerabilities.cpp
    src/library_loader.cpp
    src/network_manager.cpp
)

set(HEADERS
    src/database_setup.h
    src/dbaccess.h
    src/sessionmanager.h
    src/loginwindow.h
    src/registerwindow.h
    src/mainwindow.h
    src/adminwindow.h
    src/insecure_logger.h
    src/memory_vulnerabilities.h
    src/library_loader.h
    src/network_manager.h
    src/dvda_helper.h
)

add_executable(dvda ${SOURCES} ${HEADERS})

target_link_libraries(dvda PRIVATE
    Qt6::Widgets
    Qt6::Sql
    Qt6::Network
    OpenSSL::Crypto
)

# Link dl library for dlopen on Unix-like systems
if(UNIX)
    target_link_libraries(dvda PRIVATE dl)
endif()

# --- Helper Shared Library (Epic 9: DLL Hijacking target) ---
add_library(dvda_helper SHARED src/dvda_helper.cpp)
target_include_directories(dvda_helper PUBLIC src/)


# Copy config file and helper library to build directory
file(COPY ${CMAKE_SOURCE_DIR}/config/app.conf DESTINATION ${CMAKE_BINARY_DIR}/config/)

# Ensure the helper library is built to the same directory as the executable
set_target_properties(dvda_helper PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# ============================================================
#
#
#    $ file build/dvda
#
#
#
#
# ============================================================
