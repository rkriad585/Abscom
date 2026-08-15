/* Tests for the Ultimate layer: computational backends (AVX/GPU dispatch),
 * scalar autograd, PPM vision + convolution, plotting, and dataframes. */

#include "abscom/abs.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);  \
            return 1;                                                        \
        }                                                                    \
    } while (0)

static int close_to(double a, double b, double eps) {
    return fabs(a - b) < eps;
}

/* --- Backend selection + matrix dispatch --- */

static int test_backend(void) {
    var A = abs_matrix_new(2, 3);
    var B = abs_matrix_new(3, 2);
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 3; j++)
            abs_matrix_set(A, i, j, (double)(i * 3 + j + 1));
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 2; j++)
            abs_matrix_set(B, i, j, (double)(i * 2 + j + 1));

    double expected[2][2] = {{22.0, 28.0}, {49.0, 64.0}};

    abs_set_backend(ABS_CPU);
    CHECK(abs_get_backend() == ABS_CPU);
    CHECK(strcmp(abs_backend_name(ABS_CPU), "CPU") == 0);
    var C = abs_matrix_mul(A, B);
    CHECK(!is_err(C));
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 2; j++)
            CHECK(close_to(abs_matrix_get(C, i, j), expected[i][j], 1e-9));

    abs_set_backend(ABS_CPU_AVX);
    CHECK(abs_get_backend() == ABS_CPU_AVX);
    CHECK(strcmp(abs_backend_name(ABS_CPU_AVX), "CPU_AVX") == 0);
    var C2 = abs_matrix_mul(A, B);
    CHECK(!is_err(C2));
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 2; j++)
            CHECK(close_to(abs_matrix_get(C2, i, j), expected[i][j], 1e-9));

    abs_set_backend(ABS_GPU_CUDA);
    CHECK(strcmp(abs_backend_name(ABS_GPU_CUDA), "GPU_CUDA") == 0);
    var C3 = abs_matrix_mul(A, B); /* GPU stub falls back to CPU */
    CHECK(!is_err(C3));
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 2; j++)
            CHECK(close_to(abs_matrix_get(C3, i, j), expected[i][j], 1e-9));

    abs_set_backend(ABS_CPU);
    return 0;
}

/* --- Scalar autograd --- */

static int test_autograd(void) {
    /* f = (a * b) + relu(c)  with a=2, b=3, c=-5
     * -> f = 6 + 0 = 6, da = 3, db = 2, dc = 0 */
    AbsScalar *a = abs_scalar_new(2.0);
    AbsScalar *b = abs_scalar_new(3.0);
    AbsScalar *c = abs_scalar_new(-5.0);
    AbsScalar *t = abs_scalar_mul(a, b);
    AbsScalar *r = abs_scalar_relu(c);
    AbsScalar *f = abs_scalar_add(t, r);
    CHECK(close_to(abs_scalar_val(f), 6.0, 1e-12));
    abs_scalar_backward(f);
    CHECK(close_to(abs_scalar_grad(f), 1.0, 1e-12));
    CHECK(close_to(abs_scalar_grad(a), 3.0, 1e-12));
    CHECK(close_to(abs_scalar_grad(b), 2.0, 1e-12));
    CHECK(close_to(abs_scalar_grad(c), 0.0, 1e-12));

    /* ReLU dead branch: gradient must stay 0. */
    AbsScalar *n = abs_scalar_new(-2.0);
    AbsScalar *relu_out = abs_scalar_relu(n);
    abs_scalar_backward(relu_out);
    CHECK(abs_scalar_val(relu_out) == 0.0);
    CHECK(abs_scalar_grad(n) == 0.0);

    /* Sigmoid: analytic vs numeric gradient at x = 0.5. */
    AbsScalar *x = abs_scalar_new(0.5);
    AbsScalar *s = abs_scalar_sigmoid(x);
    abs_scalar_backward(s);
    double sig = abs_scalar_val(s);
    CHECK(close_to(abs_scalar_grad(x), sig * (1.0 - sig), 1e-12));
    double h = 1e-7;
    double num = (1.0 / (1.0 + exp(-(0.5 + h))) -
                  1.0 / (1.0 + exp(-(0.5 - h)))) / (2.0 * h);
    CHECK(close_to(abs_scalar_grad(x), num, 1e-5));

    /* Shared subtree: f = (a+b) + (a*b) with a=2, b=3.
     * df/da = 1 + b = 4, df/db = 1 + a = 3. */
    AbsScalar *s1 = abs_scalar_new(2.0);
    AbsScalar *s2 = abs_scalar_new(3.0);
    AbsScalar *u = abs_scalar_add(s1, s2);
    AbsScalar *v = abs_scalar_mul(s1, s2);
    AbsScalar *w = abs_scalar_add(u, v);
    abs_scalar_backward(w);
    CHECK(close_to(abs_scalar_grad(s1), 4.0, 1e-12));
    CHECK(close_to(abs_scalar_grad(s2), 3.0, 1e-12));

    /* zero_grad wipes the accumulated gradients. */
    abs_scalar_zero_grad(w);
    CHECK(abs_scalar_grad(s1) == 0.0 && abs_scalar_grad(s2) == 0.0);

    /* Free the whole graphs (shared subtree must not be freed twice). */
    abs_scalar_free(f);
    abs_scalar_free(relu_out);
    abs_scalar_free(s);
    abs_scalar_free(w);
    return 0;
}

/* --- PPM image I/O + convolution --- */

static int test_vision(void) {
    AbsImg src;
    src.width = 3;
    src.height = 3;
    src.rgb = (unsigned char *)malloc((size_t)9 * 3);
    for (int i = 0; i < 9 * 3; i++) src.rgb[i] = 255;

    /* P3 save / load round-trip. */
    abs_img_save_ppm(&src, "test_ultra.ppm");
    AbsImg *back = abs_img_load_ppm("test_ultra.ppm");
    CHECK(back != NULL);
    CHECK(back->width == 3 && back->height == 3);
    CHECK(memcmp(back->rgb, src.rgb, (size_t)9 * 3) == 0);
    abs_img_free(back);
    remove("test_ultra.ppm");

    /* P6 binary load. */
    FILE *fp = fopen("test_ultra6.ppm", "wb");
    CHECK(fp != NULL);
    fprintf(fp, "P6\n2 2\n255\n");
    unsigned char raw[12] = {
        255, 0, 0, 0, 255, 0, 0, 0, 255, 255, 255, 255
    };
    fwrite(raw, 1, sizeof(raw), fp);
    fclose(fp);
    back = abs_img_load_ppm("test_ultra6.ppm");
    CHECK(back != NULL);
    CHECK(back->width == 2 && back->height == 2);
    CHECK(memcmp(back->rgb, raw, sizeof(raw)) == 0);
    abs_img_free(back);
    remove("test_ultra6.ppm");

    /* Identity convolution leaves the image unchanged. */
    double identity[9] = {0, 0, 0, 0, 1, 0, 0, 0, 0};
    AbsImg *same = abs_img_conv2d(&src, 3, identity);
    CHECK(same != NULL);
    CHECK(memcmp(same->rgb, src.rgb, (size_t)9 * 3) == 0);
    abs_img_free(same);

    /* 3x3 box blur (all weights 1/9) on a solid 255 image:
     * center stays 255, a corner is zero-padded to 4/9 * 255. */
    double blur[9];
    for (int i = 0; i < 9; i++) blur[i] = 1.0 / 9.0;
    AbsImg *blurred = abs_img_conv2d(&src, 3, blur);
    CHECK(blurred != NULL);
    CHECK(blurred->rgb[0] == 113);      /* corner (0,0): 4*255/9 */
    CHECK(blurred->rgb[(1 * 3 + 1) * 3] == 255); /* center */
    abs_img_free(blurred);

    /* Reject invalid input. */
    CHECK(abs_img_conv2d(&src, 0, blur) == NULL);
    CHECK(abs_img_conv2d(&src, 4, blur) == NULL); /* even kernel */

    free(src.rgb);
    return 0;
}

/* --- Plotting --- */

static int test_plot(void) {
    double y[5] = {1.0, 2.0, 3.0, 2.0, 1.0};
    abs_plot_ascii(y, 5, 5); /* just must not crash */

    double flat[3] = {5.0, 5.0, 5.0};
    abs_plot_ascii(flat, 3, 4); /* flat series must not divide by zero */

    abs_plot_svg(NULL, y, 5, "test_ultra.svg");
    FILE *fp = fopen("test_ultra.svg", "rb");
    CHECK(fp != NULL);
    char buf[1024];
    size_t got = fread(buf, 1, sizeof(buf) - 1, fp);
    buf[got] = '\0';
    fclose(fp);
    CHECK(strstr(buf, "<svg") != NULL);
    CHECK(strstr(buf, "polyline") != NULL);
    remove("test_ultra.svg");

    double one[1] = {42.0};
    abs_plot_svg(NULL, one, 1, "test_ultra.svg"); /* single point, no /0 */
    fp = fopen("test_ultra.svg", "rb");
    CHECK(fp != NULL);
    fclose(fp);
    remove("test_ultra.svg");
    return 0;
}

/* --- DataFrame --- */

static int test_dataframe(void) {
    AbsDF *df = abs_df_create(3);
    CHECK(df != NULL);
    double ages[3] = {25.0, 30.0, 22.0};
    const char *names[3] = {"Alice", "Bob", "Carol"};
    double scores[3] = {88.5, 91.0, 79.25};

    abs_df_add_col_double(df, "Age", ages);
    abs_df_add_col_string(df, "Name", names);
    abs_df_add_col_double(df, "Score", scores);
    abs_df_print(df);

    CHECK(df->rows == 3 && df->col_count == 3);
    CHECK(strcmp(df->cols[0]->name, "Age") == 0);
    CHECK(df->cols[0]->type == ABS_COL_DOUBLE);
    CHECK(df->cols[1]->type == ABS_COL_STRING);
    CHECK(close_to(df->cols[0]->doubles[1], 30.0, 1e-12));
    CHECK(strcmp(df->cols[1]->strings[2], "Carol") == 0);
    CHECK(close_to(df->cols[2]->doubles[0], 88.5, 1e-12));

    /* The column data is copied: mutating the input must not affect the DF. */
    ages[0] = 99.0;
    CHECK(close_to(df->cols[0]->doubles[0], 25.0, 1e-12));

    abs_df_free(df);
    return 0;
}

int main(void) {
    abs_init();

    printf("--- backend ---\n");
    if (test_backend()) return 1;
    printf("--- autograd ---\n");
    if (test_autograd()) return 1;
    printf("--- vision ---\n");
    if (test_vision()) return 1;
    printf("--- plot ---\n");
    if (test_plot()) return 1;
    printf("--- dataframe ---\n");
    if (test_dataframe()) return 1;

    abs_cleanup();
    printf("test_ultra: OK\n");
    return 0;
}
