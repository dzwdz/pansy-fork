#include "bignum.h"
#include <stdbool.h>
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

	bignum *b = malloc(size * sizeof(uint64_t) + sizeof(uint16_t));
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

void bignum_copy(bignum *dest, const bignum *src) {
	// todo optimize
	bignum_zeroout(dest);

	// take the minimum of dest->length, src->length
	int to_copy = dest->length;
	if (src->length < to_copy)
		to_copy = src->length;

	memcpy(dest, src, to_copy);
}


static inline void bignum_addat(bignum *target, uint16_t pos, uint64_t to_add) {

	int overflow = __builtin_add_overflow(
			target->digits[pos], to_add, &target->digits[pos]);

	while (overflow) {
		if (++pos >= target->length) break;
		overflow = __builtin_add_overflow(
				target->digits[pos], 1, &target->digits[pos]);
	}
}

// returns the index of the last nonzero digit + 1
//         x == 0      = 0
//     0 < x < 2**64   = 1
// 2**64 < x < 2**128  = 2
// etc
static inline uint16_t bignum_order(const bignum *bn) {
	for (int i = bn->length - 1; i >= 0; i--) {
		if (bn->digits[i] != 0)
			return i + 1;
	}
	return 0;
}

void bignum_sub(bignum *result, const bignum *a, const bignum *b) {
	int length = result->length;
	if (a->length < length) length = a->length;
	if (b->length < length) length = b->length;

	bool overflow = false;
	for (int i = 0; i < length; i++) {
		if (overflow)
			overflow = __builtin_sub_overflow(a->digits[i], 1, &result->digits[i]);

		overflow |= __builtin_sub_overflow(a->digits[i], b->digits[i], &result->digits[i]);
	}

	for (int i = length; i < result->length; i++) {
		result->digits[i] = overflow ? ~0 : 0;
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

// https://youtu.be/iGVZIDQl6m0?t=1231
// i'm ripping this algorithm off a youtube video
// shoutouts to the author
//
// this has overflows all over the place
void bignum_div(const bignum *dividend, const bignum *divisor,
		bignum *quotient, bignum *remainder) {

	/*
	int divisor_order = bignum_order(divisor);
	int dividend_order = bignum_order(dividend);

	// assert divisor_order != 0

	// is the dividend has less digits than the dividend, we can just skip
	// the whole math
	if (dividend_order < divisor_order) {
		bignum_copy(remainder, dividend);
		bignum_zeroout(dividend);
		return;
	}

	// remainder = divisor_order - 1 most sig. digits of dividend;
	//
	// dividend 123456789   order = 9
	// divisor         123  order = 3
	//          1234567     skipping 7 = 9 - 3 + 1
	//                 8    starting at 9 - 3 + 2
	//                 89   copying 2 = 3 - 1 digits
	bignum_zeroout(remainder);
	memcpy(remainder->digits,
	       &dividend->[dividend_order - divisor_order + 2],
		   divisor_order - 1);

	bignum *intermediate = bignum_new(divisor_order + 2);

	for (int i = 0; i <= dividend_order + divisor_order; i++) {
		// this might be wrong
		intermediate->digits[0] =
			dividend->[dividend_order - divisor_order + 1 - i];
		memcpy(&intermediate->digits[1],
			   remainder->digits,
			   divisor_order - 1);

		// binary search for the largest d / multiple of the divisor that's less
		// than it
		uint64_t min_d = 0, max_d = ~0, d_next;
		while (min_d <= max_d) {
			d_next = (min_d >> 1) + (max_d >> 1);

			[[ a custom algorithm for comparing a bignum * uint64 with a bignum ]];

			if (bigger) {
				min_d = d_next + 1;
			} else if (smaller) {
				max_d = d_next - 1;
			} else { break; } // we could end the outer loop here, maybe. honestly idk
		}
		[[ swap the quotient around ]]

		// remainder = divident - d * divisor
		bignum_copy(remainder, dividend);
		bignum_mul(intermediate, divisor, d);
		bignum_sub(remainder, intermediate);
	}

	free(intermediate);*/
}
