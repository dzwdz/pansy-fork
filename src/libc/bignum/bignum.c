#include "bignum.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static void bignum_zeroout(bignum *a) {
	for (int i = 0; i < a->length; i++) {
		a->digits[i] = 0;
	}
}

bignum* bignum_new(uint16_t bytes) {  
	int size = bytes / sizeof(uint64_t);
	if (bytes % sizeof(uint64_t))
		size += sizeof(uint64_t);

	bignum *b = malloc(sizeof(bignum) + size * sizeof(uint64_t));
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

	memcpy(dest, src, to_copy * sizeof(uint64_t));
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
uint16_t bignum_order(const bignum *bn) {
	for (int i = bn->length - 1; i >= 0; i--) {
		if (bn->digits[i] != 0)
			return i + 1;
	}
	return 0;
}

uint64_t bignum_log2(const bignum *bn) {
	uint16_t order = bignum_order(bn);
	if (order == 0) return 0;

	uint64_t digit = bn->digits[order - 1];
	uint64_t bits = order * sizeof(uint64_t) * 8;

	// we know that digit must not equal 0
	// TODO add an assert
	uint64_t mask = ~(~0ull >> 1); // most significant bit
	while (!(digit & mask)) {
		bits--;
		digit <<= 1;
	}

	return bits;
}

void bignum_sub(bignum *result, const bignum *a, const bignum *b) {
	int length = result->length;
	if (a->length < length) length = a->length;
	if (b->length < length) length = b->length;

	bool overflow = false;
	for (int i = 0; i < length; i++) {
		// i am handling overflows in a dumb (and slow) way because __builtin_sub_overflow
		// didn't seem to work
		// possible bottleneck, todo
		
		if (overflow) {
			if (a->digits[i] == 0) {
				result->digits[i] = ~0;
			} else {
				result->digits[i]--;
				overflow = false;
			}
		}

		if (a->digits[i] >= b->digits[i]) {
			result->digits[i] = a->digits[i] - b->digits[i];
		} else {
			overflow = true;
			result->digits[i] = ((~0) - b->digits[i]) + a->digits[i] + 1;
		}
	}

	for (int i = length; i < result->length; i++) {
		result->digits[i] = overflow ? ~0 : 0;
	}
}

// returns -1 if a < b
//          0 if a = b
//          1 if a > b
int8_t bignum_compare (const bignum *a, const bignum *b) {
	// handle numbers of different sizes
	if (a->length < b->length) {
		for (int i = a->length; i < b->length; i++) {
			if (b->digits[i] != 0) return -1;
		}
	} else if (b->length < a->length) {
		for (int i = b->length; i < a->length; i++) {
			if (a->digits[i] != 0) return -1;
		}
	}

	for (int i = a->length - 1; i >= 0; i--) {
		if (a->digits[i] == b->digits[i]) continue;
		if (a->digits[i] <  b->digits[i]) return -1;
		                            else  return  1;
	}

	return 0;
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

	int dividend_order = bignum_order(dividend);
	int divisor_order = bignum_order(divisor);

	// assert divisor_order != 0

	bignum_zeroout(quotient);

	// is the dividend has less digits than the dividend, we can just skip
	// the whole math
	if (dividend_order < divisor_order) {
		bignum_copy(remainder, dividend);
		return;
	}

	// remainder = [divisor_order - 1] most significant digits of dividend
	// the most significant digit of the dividend is at [dividend_order - 1]
	//     if we're taking 1 digit we'd start at [dividend_order - 1], copy 1
	//                     2 digits              [dividend_order - 2], copy 2
	bignum_zeroout(remainder);
	memcpy(remainder->digits, &dividend->digits[dividend_order - (divisor_order - 1)], (divisor_order - 1) * sizeof(uint64_t));

	bignum *intermediate = bignum_new(divisor_order + 2);
	bignum *d = bignum_new(1);
	bignum *multiple = bignum_new(divisor_order + 2);

	for (int i = 0; i <= dividend_order - divisor_order; i++) {
		bignum_zeroout(intermediate);
		// the least significant digit is the [divisor_order + i]th ms one of
		// the dividend
		intermediate->digits[0] =
			dividend->digits[dividend_order - (divisor_order + i)];
		// then the rest of it is the remainder
		memcpy(&intermediate->digits[1],
		       remainder->digits,
		       divisor_order * sizeof(uint64_t));

		// binary search for the biggest d for which d * divisor < intermediate
		uint64_t low =  0,
		        high = ~0;
		while (low <= high) {
			d->digits[0] = (low >> 1) + (high >> 1);

			bignum_mul(multiple, d, divisor);
			int8_t diff = bignum_compare(multiple, intermediate);

			if (diff < 0) {
				low = d->digits[0] + 1;
			} else if (diff > 0) {
				high = d->digits[0] - 1;
			} else { break; }

			if (low == ~0) break; // look i'm too lazy to think about how to fix the binary search
		}

		// d might be wrong, TODO have a big fat think about this
		d->digits[0] = high;
		quotient->digits[dividend_order - divisor_order - i] = d->digits[0];

		// remainder = intermediate - d * divisor
		bignum_copy(remainder, intermediate);
		bignum_mul(multiple, d, divisor);
		bignum_sub(remainder, remainder, multiple);
	}

	free(intermediate);
	free(d);
	free(multiple);
}
