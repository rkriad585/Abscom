/* Ultimate layer demo: computational backends (AVX/GPU), scalar autograd,
 * PPM image loading/saving with convolution, ASCII + SVG plotting, and a
 * mixed-type DataFrame. */

#include "abscom/abs.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static AbsImg *make_dummy_ppm(int width, int height) {
    /* 10x10 white image with a red horizontal cross through the center. */
    AbsImg *img = (AbsImg *)malloc(sizeof(AbsImg));
    img->width = width;
    img->height = height;
    img->rgb = (unsigned char *)malloc((size_t)width * (size_t)height * 3);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int center_y = height / 2;
            int center_x = width / 2;
            int is_cross = (y == center_y || x == center_x);
            img->rgb[(y * width + x) * 3 + 0] = is_cross ? 255 : 0;
            img->rgb[(y * width + x) * 3 + 1] = 0;
            img->rgb[(y * width + x) * 3 + 2] = 0;
        }
    }
    return img;
}

int main(void) {
    abs_init();

    printf("=== Abscom Ultimate Layer ===\n\n");

    /* 1. Backend selection */
    printf("--- 1. Backend selection ---\n");
    var A = abs_matrix_new(2, 3);
    var B = abs_matrix_new(3, 2);
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 3; j++)
            abs_matrix_set(A, i, j, (double)(i * 3 + j + 1));
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 2; j++)
            abs_matrix_set(B, i, j, (double)(i * 2 + j + 1));

    abs_set_backend(ABS_CPU_AVX);
    var C = abs_matrix_mul(A, B);
    print_mat(C);
    printf("\n");
    abs_set_backend(ABS_GPU_CUDA);
    C = abs_matrix_mul(A, B); /* stub: warns once, falls back to CPU */
    print_mat(C);
    printf("\n");
    abs_set_backend(ABS_CPU);

    /* 2. Autograd */
    printf("\n--- 2. Autograd ---\n");
    AbsScalar *a = abs_scalar_new(2.0);
    AbsScalar *b = abs_scalar_new(3.0);
    AbsScalar *c = abs_scalar_new(-5.0);
    AbsScalar *t = abs_scalar_mul(a, b);
    AbsScalar *r = abs_scalar_relu(c);
    AbsScalar *f = abs_scalar_add(t, r);
    printf("f = (a * b) + relu(c),  a=2, b=3, c=-5\n");
    printf("f = %.2f\n", abs_scalar_val(f));
    abs_scalar_backward(f);
    printf("df/da = %.2f\n", abs_scalar_grad(a));
    printf("df/db = %.2f\n", abs_scalar_grad(b));
    printf("df/dc = %.2f\n", abs_scalar_grad(c));

    /* 3. Computer vision */
    printf("\n--- 3. Computer vision ---\n");
    AbsImg *img = make_dummy_ppm(10, 10);
    abs_img_save_ppm(img, "test.ppm");
    printf("Saved test.ppm (%d x %d)\n", img->width, img->height);
    AbsImg *loaded = abs_img_load_ppm("test.ppm");
    printf("Loaded PPM: %d x %d\n", loaded->width, loaded->height);

    double blur[9];
    for (int i = 0; i < 9; i++) blur[i] = 1.0 / 9.0; /* box blur */
    AbsImg *blurred = abs_img_conv2d(loaded, 3, blur);
    abs_img_save_ppm(blurred, "test_blur.ppm");
    printf("Convolved (3x3 box blur) and saved test_blur.ppm\n");
    abs_img_free(blurred);
    abs_img_free(loaded);
    abs_img_free(img);

    /* 4. Plotting */
    printf("\n--- 4. Plotting ---\n");
    int n = 30;
    double *xs = (double *)malloc((size_t)n * sizeof(double));
    double *ys = (double *)malloc((size_t)n * sizeof(double));
    for (int i = 0; i < n; i++) {
        xs[i] = (double)i * 0.1;
        ys[i] = sin(xs[i]);
    }
    printf("ASCII plot of sin(x):\n");
    abs_plot_ascii(ys, n, 8);
    abs_plot_svg(xs, ys, n, "plot.svg");
    printf("Saved plot.svg\n");
    free(ys);
    free(xs);

    /* 5. DataFrame */
    printf("\n--- 5. DataFrame ---\n");
    AbsDF *df = abs_df_create(3);
    double ages[3] = {25.0, 30.0, 22.0};
    const char *names[3] = {"Alice", "Bob", "Carol"};
    double scores[3] = {88.5, 91.0, 79.25};
    abs_df_add_col_double(df, "Age", ages);
    abs_df_add_col_string(df, "Name", names);
    abs_df_add_col_double(df, "Score", scores);
    abs_df_print(df);

    abs_scalar_free(f);
    abs_df_free(df);

    /* Tidy up the demo artifacts. */
    remove("test.ppm");
    remove("test_blur.ppm");
    remove("plot.svg");

    abs_cleanup();
    return 0;
}
