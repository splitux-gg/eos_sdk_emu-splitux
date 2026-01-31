#include "lan_common.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <unistd.h>
#endif

bool get_local_ip(char* out_ip, int out_size) {
    if (!out_ip || out_size < 16) {
        return false;
    }

#ifdef _WIN32
    // Windows implementation using GetAdaptersInfo
    IP_ADAPTER_INFO adapter_info[16];
    DWORD buf_len = sizeof(adapter_info);

    DWORD result = GetAdaptersInfo(adapter_info, &buf_len);
    if (result != ERROR_SUCCESS) {
        strcpy(out_ip, "127.0.0.1");
        return false;
    }

    PIP_ADAPTER_INFO adapter = adapter_info;
    while (adapter) {
        // Skip loopback
        if (strcmp(adapter->IpAddressList.IpAddress.String, "127.0.0.1") != 0 &&
            strcmp(adapter->IpAddressList.IpAddress.String, "0.0.0.0") != 0) {
            strncpy(out_ip, adapter->IpAddressList.IpAddress.String, out_size - 1);
            out_ip[out_size - 1] = '\0';
            return true;
        }
        adapter = adapter->Next;
    }

    strcpy(out_ip, "127.0.0.1");
    return false;
#else
    // Linux/POSIX implementation using getifaddrs
    struct ifaddrs *ifaddr, *ifa;

    if (getifaddrs(&ifaddr) == -1) {
        strcpy(out_ip, "127.0.0.1");
        return false;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;
        if (ifa->ifa_addr->sa_family != AF_INET) continue;

        struct sockaddr_in* addr = (struct sockaddr_in*)ifa->ifa_addr;

        // Skip loopback
        if (addr->sin_addr.s_addr == htonl(INADDR_LOOPBACK)) continue;

        inet_ntop(AF_INET, &addr->sin_addr, out_ip, out_size);
        freeifaddrs(ifaddr);
        return true;
    }

    freeifaddrs(ifaddr);
    strcpy(out_ip, "127.0.0.1");
    return false;
#endif
}

bool parse_address(const char* addr, char* out_ip, uint16_t* out_port) {
    if (!addr || !out_ip || !out_port) return false;

    const char* colon = strchr(addr, ':');
    if (!colon) return false;

    int ip_len = colon - addr;
    if (ip_len >= 16) return false;

    strncpy(out_ip, addr, ip_len);
    out_ip[ip_len] = '\0';

    *out_port = (uint16_t)atoi(colon + 1);
    return *out_port > 0;
}

void format_address(char* out, int out_size, const char* ip, uint16_t port) {
    if (!out || !ip || out_size < 1) return;
    snprintf(out, out_size, "%s:%d", ip, port);
}

uint64_t get_time_ms(void) {
#ifdef _WIN32
    // Windows implementation using GetTickCount64
    return GetTickCount64();
#else
    // Linux/POSIX implementation using clock_gettime
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
#endif
}

// CRC32 lookup table
static uint32_t crc32_table[256];
static bool crc32_table_initialized = false;

static void init_crc32_table(void) {
    if (crc32_table_initialized) return;

    for (uint32_t i = 0; i < 256; i++) {
        uint32_t crc = i;
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320;
            } else {
                crc >>= 1;
            }
        }
        crc32_table[i] = crc;
    }

    crc32_table_initialized = true;
}

uint32_t crc32(const void* data, size_t len) {
    if (!data || len == 0) return 0;

    init_crc32_table();

    const uint8_t* bytes = (const uint8_t*)data;
    uint32_t crc = 0xFFFFFFFF;

    for (size_t i = 0; i < len; i++) {
        uint8_t index = (crc ^ bytes[i]) & 0xFF;
        crc = (crc >> 8) ^ crc32_table[index];
    }

    return ~crc;
}

bool should_use_localhost_mode(void) {
    const char* env = getenv("EOSLAN_LOCALHOST_MODE");
    return (env && atoi(env) != 0);
}
