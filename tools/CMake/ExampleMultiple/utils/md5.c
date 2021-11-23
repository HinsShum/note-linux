#include <string.h>
#include "md5.h"

#define __md5_f(x,y,z) ((x & y) | (~x & z))
#define __md5_g(x,y,z) ((x & z) | (y & ~z))
#define __md5_h(x,y,z) (x^y^z)
#define __md5_i(x,y,z) (y ^ (x | ~z))
#define __md5_rotate_left(x,n) ((x << n) | (x >> (32-n)))
#if 0
#define __md5_ff(a,b,c,d,x,s,ac) \
{ \
	a += __md5_f(b,c,d) + x + ac; \
	a = __md5_rotate_left(a,s); \
	a += b; \
}
#define __md5_gg(a,b,c,d,x,s,ac) \
{ \
	a += __md5_g(b,c,d) + x + ac; \
	a = __md5_rotate_left(a,s); \
	a += b; \
}
#define __md5_hh(a,b,c,d,x,s,ac) \
{ \
	a += __md5_h(b,c,d) + x + ac; \
	a = __md5_rotate_left(a,s); \
	a += b; \
}
#define __md5_ii(a,b,c,d,x,s,ac) \
{ \
	a += __md5_i(b,c,d) + x + ac; \
	a = __md5_rotate_left(a,s); \
	a += b; \
}
#endif
static void __md5_ff(uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d,
                     uint32_t *x, uint32_t s, uint32_t ac) {
    *a += __md5_f(*b, *c, *d) + *x + ac;
    *a = __md5_rotate_left(*a, s);
    *a += *b;
}

static void __md5_gg(uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d,
                     uint32_t *x, uint32_t s, uint32_t ac) {
    *a += __md5_g(*b, *c, *d) + *x + ac;
    *a = __md5_rotate_left(*a, s);
    *a += *b;
}

static void __md5_hh(uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d,
                     uint32_t *x, uint32_t s, uint32_t ac) {
    *a += __md5_h(*b, *c, *d) + *x + ac;
    *a = __md5_rotate_left(*a, s);
    *a += *b;
}

static void __md5_ii(uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d,
                     uint32_t *x, uint32_t s, uint32_t ac) {
    *a += __md5_i(*b, *c, *d) + *x + ac;
    *a = __md5_rotate_left(*a, s);
    *a += *b;
}

static uint8_t PADDING[] = { 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static void md5_encode(uint8_t *output, uint32_t *input, uint32_t len)
{
	uint32_t i = 0, j = 0;
	while (j < len)
	{
		output[j] = input[i] & 0xFF;
		output[j + 1] = (input[i] >> 8) & 0xFF;
		output[j + 2] = (input[i] >> 16) & 0xFF;
		output[j + 3] = (input[i] >> 24) & 0xFF;
		i++;
		j += 4;
	}
}
static void md5_decode(uint32_t *output, uint8_t *input, uint32_t len)
{
	uint32_t i = 0, j = 0;
	while (j < len)
	{
		output[i] = ((uint32_t)input[j]) |
			((uint32_t)input[j + 1] << 8) |
			((uint32_t)input[j + 2] << 16) |
			((uint32_t)input[j + 3] << 24);
		i++;
		j += 4;
	}
}
static void md5_transform(uint32_t state[4], uint8_t block[64])
{
	uint32_t a = state[0];
	uint32_t b = state[1];
	uint32_t c = state[2];
	uint32_t d = state[3];
	uint32_t x[64];
	md5_decode(x, block, 64);

    __md5_ff(&a, &b, &c, &d, &x[0], 7, 0xd76aa478); /* 1 */
    __md5_ff(&d, &a, &b, &c, &x[1], 12, 0xe8c7b756); /* 2 */
    __md5_ff(&c, &d, &a, &b, &x[2], 17, 0x242070db); /* 3 */
    __md5_ff(&b, &c, &d, &a, &x[3], 22, 0xc1bdceee); /* 4 */
    __md5_ff(&a, &b, &c, &d, &x[4], 7, 0xf57c0faf); /* 5 */
    __md5_ff(&d, &a, &b, &c, &x[5], 12, 0x4787c62a); /* 6 */
    __md5_ff(&c, &d, &a, &b, &x[6], 17, 0xa8304613); /* 7 */
    __md5_ff(&b, &c, &d, &a, &x[7], 22, 0xfd469501); /* 8 */
    __md5_ff(&a, &b, &c, &d, &x[8], 7, 0x698098d8); /* 9 */
    __md5_ff(&d, &a, &b, &c, &x[9], 12, 0x8b44f7af); /* 10 */
    __md5_ff(&c, &d, &a, &b, &x[10], 17, 0xffff5bb1); /* 11 */
    __md5_ff(&b, &c, &d, &a, &x[11], 22, 0x895cd7be); /* 12 */
    __md5_ff(&a, &b, &c, &d, &x[12], 7, 0x6b901122); /* 13 */
    __md5_ff(&d, &a, &b, &c, &x[13], 12, 0xfd987193); /* 14 */
    __md5_ff(&c, &d, &a, &b, &x[14], 17, 0xa679438e); /* 15 */
    __md5_ff(&b, &c, &d, &a, &x[15], 22, 0x49b40821); /* 16 */

	/* Round 2 */
    __md5_gg(&a, &b, &c, &d, &x[1], 5, 0xf61e2562); /* 17 */
    __md5_gg(&d, &a, &b, &c, &x[6], 9, 0xc040b340); /* 18 */
    __md5_gg(&c, &d, &a, &b, &x[11], 14, 0x265e5a51); /* 19 */
    __md5_gg(&b, &c, &d, &a, &x[0], 20, 0xe9b6c7aa); /* 20 */
    __md5_gg(&a, &b, &c, &d, &x[5], 5, 0xd62f105d); /* 21 */
    __md5_gg(&d, &a, &b, &c, &x[10], 9, 0x2441453); /* 22 */
    __md5_gg(&c, &d, &a, &b, &x[15], 14, 0xd8a1e681); /* 23 */
    __md5_gg(&b, &c, &d, &a, &x[4], 20, 0xe7d3fbc8); /* 24 */
    __md5_gg(&a, &b, &c, &d, &x[9], 5, 0x21e1cde6); /* 25 */
    __md5_gg(&d, &a, &b, &c, &x[14], 9, 0xc33707d6); /* 26 */
    __md5_gg(&c, &d, &a, &b, &x[3], 14, 0xf4d50d87); /* 27 */
    __md5_gg(&b, &c, &d, &a, &x[8], 20, 0x455a14ed); /* 28 */
    __md5_gg(&a, &b, &c, &d, &x[13], 5, 0xa9e3e905); /* 29 */
    __md5_gg(&d, &a, &b, &c, &x[2], 9, 0xfcefa3f8); /* 30 */
    __md5_gg(&c, &d, &a, &b, &x[7], 14, 0x676f02d9); /* 31 */
    __md5_gg(&b, &c, &d, &a, &x[12], 20, 0x8d2a4c8a); /* 32 */

	/* Round 3 */
    __md5_hh(&a, &b, &c, &d, &x[5], 4, 0xfffa3942); /* 33 */
    __md5_hh(&d, &a, &b, &c, &x[8], 11, 0x8771f681); /* 34 */
    __md5_hh(&c, &d, &a, &b, &x[11], 16, 0x6d9d6122); /* 35 */
    __md5_hh(&b, &c, &d, &a, &x[14], 23, 0xfde5380c); /* 36 */
    __md5_hh(&a, &b, &c, &d, &x[1], 4, 0xa4beea44); /* 37 */
    __md5_hh(&d, &a, &b, &c, &x[4], 11, 0x4bdecfa9); /* 38 */
    __md5_hh(&c, &d, &a, &b, &x[7], 16, 0xf6bb4b60); /* 39 */
    __md5_hh(&b, &c, &d, &a, &x[10], 23, 0xbebfbc70); /* 40 */
    __md5_hh(&a, &b, &c, &d, &x[13], 4, 0x289b7ec6); /* 41 */
    __md5_hh(&d, &a, &b, &c, &x[0], 11, 0xeaa127fa); /* 42 */
    __md5_hh(&c, &d, &a, &b, &x[3], 16, 0xd4ef3085); /* 43 */
    __md5_hh(&b, &c, &d, &a, &x[6], 23, 0x4881d05); /* 44 */
    __md5_hh(&a, &b, &c, &d, &x[9], 4, 0xd9d4d039); /* 45 */
    __md5_hh(&d, &a, &b, &c, &x[12], 11, 0xe6db99e5); /* 46 */
    __md5_hh(&c, &d, &a, &b, &x[15], 16, 0x1fa27cf8); /* 47 */
    __md5_hh(&b, &c, &d, &a, &x[2], 23, 0xc4ac5665); /* 48 */

	/* Round 4 */
    __md5_ii(&a, &b, &c, &d, &x[0], 6, 0xf4292244); /* 49 */
    __md5_ii(&d, &a, &b, &c, &x[7], 10, 0x432aff97); /* 50 */
    __md5_ii(&c, &d, &a, &b, &x[14], 15, 0xab9423a7); /* 51 */
    __md5_ii(&b, &c, &d, &a, &x[5], 21, 0xfc93a039); /* 52 */
    __md5_ii(&a, &b, &c, &d, &x[12], 6, 0x655b59c3); /* 53 */
    __md5_ii(&d, &a, &b, &c, &x[3], 10, 0x8f0ccc92); /* 54 */
    __md5_ii(&c, &d, &a, &b, &x[10], 15, 0xffeff47d); /* 55 */
    __md5_ii(&b, &c, &d, &a, &x[1], 21, 0x85845dd1); /* 56 */
    __md5_ii(&a, &b, &c, &d, &x[8], 6, 0x6fa87e4f); /* 57 */
    __md5_ii(&d, &a, &b, &c, &x[15], 10, 0xfe2ce6e0); /* 58 */
    __md5_ii(&c, &d, &a, &b, &x[6], 15, 0xa3014314); /* 59 */
    __md5_ii(&b, &c, &d, &a, &x[13], 21, 0x4e0811a1); /* 60 */
    __md5_ii(&a, &b, &c, &d, &x[4], 6, 0xf7537e82); /* 61 */
    __md5_ii(&d, &a, &b, &c, &x[11], 10, 0xbd3af235); /* 62 */
    __md5_ii(&c, &d, &a, &b, &x[2], 15, 0x2ad7d2bb); /* 63 */
    __md5_ii(&b, &c, &d, &a, &x[9], 21, 0xeb86d391); /* 64 */

	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;
}

void md5_init(struct st_md5_ctx *context)
{
	context->count[0] = 0;
	context->count[1] = 0;
	context->state[0] = 0x67452301;
	context->state[1] = 0xEFCDAB89;
	context->state[2] = 0x98BADCFE;
	context->state[3] = 0x10325476;
}

void md5_update(struct st_md5_ctx *context, uint8_t *input, uint32_t inputlen)
{
	uint32_t i = 0, index = 0, partlen = 0;
	index = (context->count[0] >> 3) & 0x3F;
	partlen = 64 - index;
	context->count[0] += inputlen << 3;
	if (context->count[0] < (inputlen << 3))
		context->count[1]++;
	context->count[1] += inputlen >> 29;

	if (inputlen >= partlen)
	{
		memcpy(&context->buffer[index], input, partlen);
		md5_transform(context->state, context->buffer);
		for (i = partlen; i + 64 <= inputlen; i += 64)
			md5_transform(context->state, &input[i]);
		index = 0;
	}
	else
	{
		i = 0;
	}
	memcpy(&context->buffer[index], &input[i], inputlen - i);
}

void md5_final(struct st_md5_ctx *context, uint8_t *digest)
{
	uint32_t index = 0, padlen = 0;
	uint8_t bits[8];
	index = (context->count[0] >> 3) & 0x3F;
	padlen = (index < 56) ? (56 - index) : (120 - index);
	md5_encode(bits, context->count, 8);
	md5_update(context, PADDING, padlen);
	md5_update(context, bits, 8);
	md5_encode(digest, context->state, 16);
}
