#include "bignum.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void bignum_zeroout(bignum *a) {
	for (int i = 0; i < a->length; i++) {
		a->digits[i] = 0;
	}
}

bignum* bignum_new(uint16_t bytes) {  
	int size = bytes / sizeof(uint64_t);
	if (bytes % sizeof(uint64_t))
		size += sizeof(uint64_t);

	bignum *b = malloc(size * 64 + sizeof(uint16_t));
	b->length = size;
	bignum_zeroout(b);
	return b;
}

// will be replaced by a function that returns a string later on
// i don't hate this any less than you do
void bignum_print(const bignum *a) {
	for (int i = a->length - 1; i >= 0; i--) {
		int j = sizeof(uint64_t) * 8;
		while (j > 0) {
			j -= 4;
			char c = '0' + ((a->digits[i] >> j) & 0xf);
			if (c > '9') c += 'A' - '9' - 1;
			write(1, &c, 1);
		}
		printf(" ");
	}
	puts("");
}

// will do very weird things if the input string isn't hex
void bignum_fromhex(bignum *target, const char *hex) {
	bignum_zeroout(target);

	int nibble = 0;
	uint64_t digit;
	for (int i = strlen(hex) - 1; i >= 0; i--) {
		char c = hex[i];
		     if ('0' <= c && c <= '9') digit = hex[i] - '0';
		else if ('a' <= c && c <= 'f') digit = hex[i] - 'a' + 0xa;
		else if ('A' <= c && c <= 'F') digit = hex[i] - 'A' + 0xA;
		else digit = 0;

		if (nibble & 1) digit <<= 4;
		int byte = (nibble >> 1) & 0b111;
		digit <<= 8 * byte;

		target->digits[nibble >> 4] |= digit;

		nibble++;
	}
}


static inline void bignum_addat(bignum *target, uint16_t pos, uint64_t to_add) {

	int overflow = __builtin_add_overflow(
			target->digits[pos], to_add, &target->digits[pos]);

	return;
	while (overflow) {
		if (++pos >= target->length) break;
		overflow = __builtin_add_overflow(
				target->digits[pos], 1, &target->digits[pos]);
	}
}

void bignum_mul(bignum *result, const bignum *a, const bignum *b) {
	bignum_zeroout(result);
	uint16_t length = result->length;

	for (uint16_t i = 0; i < a->length; i++) {
		if (a->digits[i] == 0) continue;
		for (uint16_t j = 0; j+i < length; j++) { // potential overflow
			if (b->digits[j] == 0) continue;
			__int128 product = (__int128)(a->digits[i]) * (__int128)(b->digits[j]);
			uint64_t low  = product >> 64;
			uint64_t high = product;
			bignum_addat(result, j+i, high);
			if (j+i + 1 < length)
				bignum_addat(result, j+i + 1, low);
		}
	}
}
