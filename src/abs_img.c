/* Computer vision: NetPBM PPM images (P3 text and P6 binary) plus a
 * zero-padded 2D convolution. AbsImg owns a width*height*3 RGB buffer and is
 * released with abs_img_free(). */

#include "abscom/abs.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static AbsImg *abs_img_new(int width, int height) {
    if (width <= 0 || height <= 0) return NULL;
    AbsImg *img = (AbsImg *)malloc(sizeof(AbsImg));
    if (!img) return NULL;
    img->width = width;
    img->height = height;
    img->rgb = (unsigned char *)malloc((size_t)width * (size_t)height * 3);
    if (!img->rgb) {
        free(img);
        return NULL;
    }
    return img;
}

static int abs_ppm_next_token(FILE *fp, char *buf, size_t cap) {
    /* Skip whitespace and '#'-to-end-of-line comments, then read one token. */
    int c;
    size_t i = 0;
    for (;;) {
        c = fgetc(fp);
        if (c == '#') {
            while ((c = fgetc(fp)) != '\n' && c != EOF) {
                /* swallow comment */
            }
            if (c == EOF) return 0;
        } else if (c == EOF) {
            return 0;
        } else if (c != ' ' && c != '\t' && c != '\r' && c != '\n') {
            break;
        }
    }
    do {
        if (i + 1 < cap) buf[i++] = (char)c;
        c = fgetc(fp);
    } while (c != ' ' && c != '\t' && c != '\r' && c != '\n' &&
             c != '#' && c != EOF);
    if (c == '#') ungetc(c, fp);
    buf[i] = '\0';
    return 1;
}

AbsImg *abs_img_load_ppm(const char *filename) {
    if (!filename) return NULL;
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "abs_img_load_ppm: cannot open %s\n", filename);
        return NULL;
    }

    char magic[8];
    if (fscanf(fp, "%7s", magic) != 1 ||
        (strcmp(magic, "P3") != 0 && strcmp(magic, "P6") != 0)) {
        fprintf(stderr, "abs_img_load_ppm: not a PPM file (magic %s)\n", magic);
        fclose(fp);
        return NULL;
    }

    char tok[64];
    AbsImg *img = NULL;
    if (!abs_ppm_next_token(fp, tok, sizeof(tok))) goto fail;
    int width = atoi(tok);
    if (!abs_ppm_next_token(fp, tok, sizeof(tok))) goto fail;
    int height = atoi(tok);
    if (!abs_ppm_next_token(fp, tok, sizeof(tok))) goto fail;
    int maxval = atoi(tok);
    if (width <= 0 || height <= 0 || maxval <= 0 || maxval > 65535) {
        fprintf(stderr, "abs_img_load_ppm: bad header (%dx%d max %d)\n",
                width, height, maxval);
        goto fail;
    }

    img = abs_img_new(width, height);
    if (!img) goto fail;

    if (strcmp(magic, "P6") == 0) {
        /* The token reader already consumed the whitespace after maxval;
         * swallow any *further* whitespace up to the first data byte. */
        int c = fgetc(fp);
        while (c == ' ' || c == '\t' || c == '\r' || c == '\n')
            c = fgetc(fp);
        if (c == EOF) goto fail;
        size_t want = (size_t)width * (size_t)height * 3;
        if (maxval > 255) {
            /* 16-bit P6: two bytes per channel, take the high byte. */
            unsigned char *p = img->rgb;
            img->rgb[0] = (unsigned char)c;
            fgetc(fp); /* low byte */
            p++;
            for (size_t i = 1; i < want; i++) {
                int hi = fgetc(fp);
                fgetc(fp);
                if (hi == EOF) goto fail;
                *p++ = (unsigned char)hi;
            }
        } else {
            img->rgb[0] = (unsigned char)c;
            if (fread(img->rgb + 1, 1, want - 1, fp) != want - 1)
                goto fail;
        }
    } else {
        /* P3: maxval-scaled ASCII values. */
        for (int i = 0; i < width * height * 3; i++) {
            if (!abs_ppm_next_token(fp, tok, sizeof(tok))) goto fail;
            int v = atoi(tok);
            if (maxval != 255) v = (v * 255 + maxval / 2) / maxval;
            if (v < 0) v = 0;
            if (v > 255) v = 255;
            img->rgb[i] = (unsigned char)v;
        }
    }

    fclose(fp);
    return img;

fail:
    if (img) abs_img_free(img);
    fclose(fp);
    return NULL;
}

void abs_img_save_ppm(const AbsImg *img, const char *filename) {
    if (!img || !filename) return;
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "abs_img_save_ppm: cannot open %s\n", filename);
        return;
    }
    fprintf(fp, "P3\n%d %d\n255\n", img->width, img->height);
    int n = img->width * img->height * 3;
    for (int i = 0; i < n; i++) {
        fprintf(fp, "%d%s", img->rgb[i],
                (i % 3 == 2) ? "\n" : ((i % 15 == 14) ? "\n" : " "));
    }
    fclose(fp);
}

static unsigned char abs_clamp_u8(int v) {
    if (v < 0) return 0;
    if (v > 255) return 255;
    return (unsigned char)v;
}

AbsImg *abs_img_conv2d(const AbsImg *img, int kernel_size,
                       const double *kernel) {
    if (!img || !kernel) return NULL;
    if (kernel_size <= 0 || kernel_size % 2 == 0) return NULL;

    int pad = kernel_size / 2;
    int w = img->width;
    int h = img->height;
    AbsImg *out = abs_img_new(w, h);
    if (!out) return NULL;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            for (int c = 0; c < 3; c++) {
                double acc = 0.0;
                for (int ky = 0; ky < kernel_size; ky++) {
                    int sy = y + ky - pad;
                    if (sy < 0 || sy >= h) continue; /* zero padding */
                    for (int kx = 0; kx < kernel_size; kx++) {
                        int sx = x + kx - pad;
                        if (sx < 0 || sx >= w) continue;
                        double k = kernel[ky * kernel_size + kx];
                        acc += k * (double)img->rgb[(sy * w + sx) * 3 + c];
                    }
                }
                out->rgb[(y * w + x) * 3 + c] = abs_clamp_u8((int)acc);
            }
        }
    }
    return out;
}

void abs_img_free(AbsImg *img) {
    if (!img) return;
    free(img->rgb);
    free(img);
}
