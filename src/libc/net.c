#include <stdint.h>
#include <sys/socket.h>

uint16_t htons(uint16_t hosts) {
	return hosts << 8 | hosts >> 8;
}
