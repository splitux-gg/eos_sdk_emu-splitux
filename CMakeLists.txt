cmake_minimum_required(VERSION 3.20)
project(EOSSDK-LAN VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# PRIMARY TARGET: Windows DLL
set(OUTPUT_NAME "EOSSDK-Win64-Shipping")

# Source files
set(SOURCES
    src/platform.c
    src/stubs.c
    src/palworld_stubs.c
    src/auth.c
    src/logging.c
    src/callbacks.c
    src/lan_common.c
    src/lan_discovery.c
    src/lan_p2p.c
    src/connect.c
    src/sessions.c
    src/session_modification.c
    src/session_search.c
    src/session_details.c
    src/p2p.c
)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/internal
)

# Windows DLL
add_library(${OUTPUT_NAME} SHARED ${SOURCES})

# Windows-specific settings
if(WIN32)
    target_link_libraries(${OUTPUT_NAME} ws2_32)
    target_compile_definitions(${OUTPUT_NAME} PRIVATE
        _CRT_SECURE_NO_WARNINGS
        WIN32_LEAN_AND_MEAN
        EOS_BUILD_DLL=1
        EOS_USE_DLLEXPORT=1
    )
    set_target_properties(${OUTPUT_NAME} PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS ON
    )
else()
    # Linux/Unix settings
    target_compile_definitions(${OUTPUT_NAME} PRIVATE
        EOS_BUILD_DLL=1
        EOS_USE_DLLEXPORT=1
    )
endif()

# Mock game executable for testing
add_executable(mock-game test-bench/mock-game/main.c)
target_link_libraries(mock-game ${OUTPUT_NAME})
target_include_directories(mock-game PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Install rules
install(TARGETS ${OUTPUT_NAME} RUNTIME DESTINATION bin)
install(TARGETS mock-game RUNTIME DESTINATION bin)
