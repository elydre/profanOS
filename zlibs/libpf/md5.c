/*****************************************************************************\
|   === md5.c : 2025 ===                                                      |
|                                                                             |
|    MD5 Message-Digest Algorithm implementation                   .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS - under GPLv3                  q. /|\  "   |
|    Original code from the GNU C library                          `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <string.h>
#include <stdio.h>

#define BLOCKSIZE 4096 // file read block size (multiple of 64)

typedef struct md5_ctx {
    uint32_t A;
    uint32_t B;
    uint32_t C;
    uint32_t D;
    uint32_t total[2];
    uint32_t buflen;
    char buffer[128];
} md5_ctx_t;


static void md5_init_ctx(md5_ctx_t *ctx) {
    ctx->A = 0x67452301;
    ctx->B = 0xefcdab89;
    ctx->C = 0x98badcfe;
    ctx->D = 0x10325476;
    ctx->total[0] = ctx->total[1] = 0;
    ctx->buflen = 0;
}

static void *md5_read_ctx(const md5_ctx_t *ctx, void *resbuf) {
    ((uint32_t *) resbuf)[0] = ctx->A;
    ((uint32_t *) resbuf)[1] = ctx->B;
    ((uint32_t *) resbuf)[2] = ctx->C;
    ((uint32_t *) resbuf)[3] = ctx->D;
    return resbuf;
}

#define FF(b, c, d) (d ^ (b & (c ^ d)))
#define FG(b, c, d) FF (d, b, c)
#define FH(b, c, d) (b ^ c ^ d)
#define FI(b, c, d) (c ^ (b | ~d))

static void md5_process_block(const void *buffer, uint32_t len, md5_ctx_t *ctx) {
    uint32_t correct_words[16];
    const uint32_t *words = buffer;
    uint32_t nwords = len / sizeof(uint32_t);
    const uint32_t *endp = words + nwords;
    uint32_t A = ctx->A;
    uint32_t B = ctx->B;
    uint32_t C = ctx->C;
    uint32_t D = ctx->D;

    // first increment the byte count
    ctx->total[0] += len;
    if (ctx->total[0] < len)
        ++ctx->total[1];

    // process all bytes in the buffer with 64 bytes in each round of the loop
    while (words < endp) {
        uint32_t *cwp = correct_words;
        uint32_t A_save = A;
        uint32_t B_save = B;
        uint32_t C_save = C;
        uint32_t D_save = D;

        #define OP(a, b, c, d, s, T)        \
            do {                            \
                a += FF(b, c, d) + (*cwp++ = *words) + T; \
                words++;                    \
                CYCLIC(a, s);               \
                a += b;                     \
            } while (0)

        #define CYCLIC(w, s) (w = (w << s) | (w >> (32 - s)))

        // round 1
        OP(A, B, C, D, 7,  0xd76aa478);
        OP(D, A, B, C, 12, 0xe8c7b756);
        OP(C, D, A, B, 17, 0x242070db);
        OP(B, C, D, A, 22, 0xc1bdceee);
        OP(A, B, C, D, 7,  0xf57c0faf);
        OP(D, A, B, C, 12, 0x4787c62a);
        OP(C, D, A, B, 17, 0xa8304613);
        OP(B, C, D, A, 22, 0xfd469501);
        OP(A, B, C, D, 7,  0x698098d8);
        OP(D, A, B, C, 12, 0x8b44f7af);
        OP(C, D, A, B, 17, 0xffff5bb1);
        OP(B, C, D, A, 22, 0x895cd7be);
        OP(A, B, C, D, 7,  0x6b901122);
        OP(D, A, B, C, 12, 0xfd987193);
        OP(C, D, A, B, 17, 0xa679438e);
        OP(B, C, D, A, 22, 0x49b40821);

        #undef OP
        #define OP(f, a, b, c, d, k, s, T)  \
            do {                            \
                a += f (b, c, d) + correct_words[k] + T; \
                CYCLIC (a, s);              \
                a += b;                     \
            } while (0)

        // round 2
        OP(FG, A, B, C, D, 1,  5,  0xf61e2562);
        OP(FG, D, A, B, C, 6,  9,  0xc040b340);
        OP(FG, C, D, A, B, 11, 14, 0x265e5a51);
        OP(FG, B, C, D, A, 0,  20, 0xe9b6c7aa);
        OP(FG, A, B, C, D, 5,  5,  0xd62f105d);
        OP(FG, D, A, B, C, 10, 9,  0x02441453);
        OP(FG, C, D, A, B, 15, 14, 0xd8a1e681);
        OP(FG, B, C, D, A, 4,  20, 0xe7d3fbc8);
        OP(FG, A, B, C, D, 9,  5,  0x21e1cde6);
        OP(FG, D, A, B, C, 14, 9,  0xc33707d6);
        OP(FG, C, D, A, B, 3,  14, 0xf4d50d87);
        OP(FG, B, C, D, A, 8,  20, 0x455a14ed);
        OP(FG, A, B, C, D, 13, 5,  0xa9e3e905);
        OP(FG, D, A, B, C, 2,  9,  0xfcefa3f8);
        OP(FG, C, D, A, B, 7,  14, 0x676f02d9);
        OP(FG, B, C, D, A, 12, 20, 0x8d2a4c8a);

        // round 3
        OP(FH, A, B, C, D, 5,  4,  0xfffa3942);
        OP(FH, D, A, B, C, 8,  11, 0x8771f681);
        OP(FH, C, D, A, B, 11, 16, 0x6d9d6122);
        OP(FH, B, C, D, A, 14, 23, 0xfde5380c);
        OP(FH, A, B, C, D, 1,  4,  0xa4beea44);
        OP(FH, D, A, B, C, 4,  11, 0x4bdecfa9);
        OP(FH, C, D, A, B, 7,  16, 0xf6bb4b60);
        OP(FH, B, C, D, A, 10, 23, 0xbebfbc70);
        OP(FH, A, B, C, D, 13, 4,  0x289b7ec6);
        OP(FH, D, A, B, C, 0,  11, 0xeaa127fa);
        OP(FH, C, D, A, B, 3,  16, 0xd4ef3085);
        OP(FH, B, C, D, A, 6,  23, 0x04881d05);
        OP(FH, A, B, C, D, 9,  4,  0xd9d4d039);
        OP(FH, D, A, B, C, 12, 11, 0xe6db99e5);
        OP(FH, C, D, A, B, 15, 16, 0x1fa27cf8);
        OP(FH, B, C, D, A, 2,  23, 0xc4ac5665);

        // round 4
        OP(FI, A, B, C, D, 0,  6,  0xf4292244);
        OP(FI, D, A, B, C, 7,  10, 0x432aff97);
        OP(FI, C, D, A, B, 14, 15, 0xab9423a7);
        OP(FI, B, C, D, A, 5,  21, 0xfc93a039);
        OP(FI, A, B, C, D, 12, 6,  0x655b59c3);
        OP(FI, D, A, B, C, 3,  10, 0x8f0ccc92);
        OP(FI, C, D, A, B, 10, 15, 0xffeff47d);
        OP(FI, B, C, D, A, 1,  21, 0x85845dd1);
        OP(FI, A, B, C, D, 8,  6,  0x6fa87e4f);
        OP(FI, D, A, B, C, 15, 10, 0xfe2ce6e0);
        OP(FI, C, D, A, B, 6,  15, 0xa3014314);
        OP(FI, B, C, D, A, 13, 21, 0x4e0811a1);
        OP(FI, A, B, C, D, 4,  6,  0xf7537e82);
        OP(FI, D, A, B, C, 11, 10, 0xbd3af235);
        OP(FI, C, D, A, B, 2,  15, 0x2ad7d2bb);
        OP(FI, B, C, D, A, 9,  21, 0xeb86d391);

        // add the starting values of the context
        A += A_save;
        B += B_save;
        C += C_save;
        D += D_save;
    }

    // put checksum in context given as argument
    ctx->A = A;
    ctx->B = B;
    ctx->C = C;
    ctx->D = D;
}

static void *md5_finish_ctx(md5_ctx_t *ctx, void *resbuf) {
    uint32_t bytes = ctx->buflen;
    uint32_t pad;

    // count remaining bytes
    ctx->total[0] += bytes;
    if (ctx->total[0] < bytes)
        ++ctx->total[1];
    pad = bytes >= 56 ? 64 + 56 - bytes : 56 - bytes;

    ctx->buffer[bytes] = 0x80;
    memset(&ctx->buffer[bytes + 1], 0, pad - 1);

    // put the 64-bit file length in *bits* at the end of the buffer
    *(uint32_t *) & ctx->buffer[bytes + pad] = ctx->total[0] << 3;
    *(uint32_t *) & ctx->buffer[bytes + pad + 4] = (ctx->total[1] << 3) | (ctx->total[0] >> 29);

    // process last bytes
    md5_process_block(ctx->buffer, bytes + pad + 8, ctx);
    return md5_read_ctx(ctx, resbuf);
}

static void md5_process_bytes(const void *buffer, uint32_t len, md5_ctx_t *ctx) {
    if (ctx->buflen != 0) {
        uint32_t left_over = ctx->buflen;
        uint32_t add = 128 - left_over > len ? len : 128 - left_over;
        memcpy(&ctx->buffer[left_over], buffer, add);
        ctx->buflen += add;
        if (left_over + add > 64) {
            md5_process_block(ctx->buffer, (left_over + add) & ~63, ctx);
            // the regions in the following copy operation cannot overlap
            memcpy(ctx->buffer, &ctx->buffer[(left_over + add) & ~63],
                (left_over + add) & 63);
            ctx->buflen = (left_over + add) & 63;
        }
        buffer = (const char *) buffer + add;
        len -= add;
    }

    // process available complete blocks
    if (len > 64) {
        md5_process_block(buffer, len & ~63, ctx);
        buffer = (const char *) buffer + (len & ~63);
        len &= 63;
    }

    // move remaining bytes in internal buffer
    if (len > 0) {
        memcpy(ctx->buffer, buffer, len);
        ctx->buflen = len;
    }
}

int md5_stream(FILE *stream, void *resblock) {
    char buffer[BLOCKSIZE + 72];
    md5_ctx_t ctx;
    uint32_t sum;

    md5_init_ctx(&ctx);

    while (1) {
        uint32_t n;
        sum = 0;

        // read block - take care for partial reads
        do {
            n = fread(buffer + sum, 1, BLOCKSIZE - sum, stream);
            sum += n;
        } while (sum < BLOCKSIZE && n != 0);

        if (n == 0 && ferror(stream))
            return 1;
        if (n == 0)
            break;

        md5_process_block(buffer, BLOCKSIZE, &ctx);
    }

    if (sum > 0)
        md5_process_bytes(buffer, sum, &ctx);

    // construct result in desired memory
    md5_finish_ctx(&ctx, resblock);
    return 0;
}

void *md5_buffer(const void *buffer, uint32_t len, void *resblock) {
    md5_ctx_t ctx;

    md5_init_ctx(&ctx);
    md5_process_bytes(buffer, len, &ctx);
    return md5_finish_ctx(&ctx, resblock);
}
