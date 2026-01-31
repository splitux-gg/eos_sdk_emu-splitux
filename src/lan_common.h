#ifndef EOS_LAN_COMMON_H
#define EOS_LAN_COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * Get local IP address (first non-loopback interface).
 */
bool get_local_ip(char* out_ip, int out_size);

/**
 * Parse "IP:port" string.
 */
bool parse_address(const char* addr, char* out_ip, uint16_t* out_port);

/**
 * Format "IP:port" string.
 */
void format_address(char* out, int out_size, const char* ip, uint16_t port);

/**
 * Get current time in milliseconds.
 */
uint64_t get_time_ms(void);

/**
 * CRC32 checksum.
 */
uint32_t crc32(const void* data, size_t len);

/**
 * Check if localhost discovery mode should be enabled.
 * Returns true if EOSLAN_LOCALHOST_MODE env var is set to non-zero.
 */
bool should_use_localhost_mode(void);

#endif // EOS_LAN_COMMON_H
