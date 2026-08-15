/* Cryptography: SHA-256 and keyed HMAC-SHA-256.
 *
 * The digest and MAC are returned as ABS_STR values holding the lowercase hex
 * encoding, ready to print, store, or compare.
 *
 *   sha256("abc")                        -> ba7816bf8f01cfea414140de5dae2223...
 *   hmac_sha256("key", "message")        -> a 64-character hex MAC
 *
 * The SHA-256 core is a compact FIPS 180-4 implementation in the public-domain
 * style (no third-party dependencies). Empty and single-block inputs are
 * covered; keys longer than the 64-byte block are pre-hashed per RFC 2104.
 */

#include "abscom/abs.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* --- SHA-256 core (FIPS 180-4) --- */

#define ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
#define CH(x, y, z) (((x) & (y)) ^ ((~(x)) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define SIG0(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define SIG1(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define gam0(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ ((x) >> 3))
#define gam1(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ ((x) >> 10))

static const uint32_t K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
    0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
    0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
    0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
    0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
    0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

static void sha256_transform(uint32_t *state, const uint8_t *data) {
    uint32_t a, b, c, d, e, f, g, h, t1, t2, m[64];
    uint32_t i;
    size_t j;
    for (i = 0, j = 0; i < 16; ++i, j += 4)
        m[i] = ((uint32_t)data[j] << 24) | ((uint32_t)data[j + 1] << 16) |
               ((uint32_t)data[j + 2] << 8) | (uint32_t)data[j + 3];
    for (; i < 64; ++i)
        m[i] = gam1(m[i - 2]) + m[i - 7] + gam0(m[i - 15]) + m[i - 16];
    a = state[0]; b = state[1]; c = state[2]; d = state[3];
    e = state[4]; f = state[5]; g = state[6]; h = state[7];
    for (i = 0; i < 64; ++i) {
        t1 = h + SIG1(e) + CH(e, f, g) + K[i] + m[i];
        t2 = SIG0(a) + MAJ(a, b, c);
        h = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
    }
    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
    state[4] += e; state[5] += f; state[6] += g; state[7] += h;
}

static void sha256_bytes(const void *input, size_t len, uint8_t *out) {
    uint32_t state[8] = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };
    uint8_t buf[64];
    size_t i, rem;
    uint64_t bits = (uint64_t)len * 8;
    const uint8_t *msg = (const uint8_t *)input;

    for (i = 0; len - i >= 64; i += 64) sha256_transform(state, msg + i);
    rem = len - i;
    memcpy(buf, msg + i, rem);
    buf[rem++] = 0x80;
    if (rem > 56) {
        memset(buf + rem, 0, 64 - rem);
        sha256_transform(state, buf);
        rem = 0;
    }
    memset(buf + rem, 0, 56 - rem);
    for (i = 0; i < 8; i++) buf[63 - i] = (uint8_t)((bits >> (i * 8)) & 0xFF);
    sha256_transform(state, buf);
    for (i = 0; i < 8; i++) {
        out[i * 4] = (uint8_t)(state[i] >> 24);
        out[i * 4 + 1] = (uint8_t)(state[i] >> 16);
        out[i * 4 + 2] = (uint8_t)(state[i] >> 8);
        out[i * 4 + 3] = (uint8_t)state[i];
    }
}

static var hex_from_bytes(const uint8_t *digest, size_t n) {
    char hex[65];
    size_t i;
    for (i = 0; i < n; i++) sprintf(hex + (i * 2), "%02x", digest[i]);
    return abs_new_str(hex);
}

var sha256(const char *input) {
    if (!input) return abs_new_error("sha256 requires a string");
    uint8_t digest[32];
    sha256_bytes(input, strlen(input), digest);
    return hex_from_bytes(digest, 32);
}

var hmac_sha256(const char *key, const char *msg) {
    if (!key || !msg) return abs_new_error("hmac_sha256 requires key and message");
    uint8_t k[64] = { 0 }, o_key[64], i_key[64];
    uint8_t inner[32], outer[32];
    size_t klen = strlen(key);
    size_t mlen = strlen(msg);
    int i;

    if (klen > 64) sha256_bytes(key, klen, k);
    else memcpy(k, key, klen);
    for (i = 0; i < 64; i++) {
        o_key[i] = k[i] ^ 0x5c;
        i_key[i] = k[i] ^ 0x36;
    }

    /* inner = SHA256(ipad || message) */
    uint8_t *i_buf = (uint8_t *)malloc(64 + mlen);
    if (!i_buf) return abs_new_error("Out of memory");
    memcpy(i_buf, i_key, 64);
    memcpy(i_buf + 64, msg, mlen);
    sha256_bytes(i_buf, 64 + mlen, inner);
    free(i_buf);

    /* outer = SHA256(opad || inner) */
    uint8_t o_buf[96];
    memcpy(o_buf, o_key, 64);
    memcpy(o_buf + 64, inner, 32);
    sha256_bytes(o_buf, 96, outer);

    return hex_from_bytes(outer, 32);
}
