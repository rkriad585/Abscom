/* Plotting: ASCII terminal charts and SVG line-chart export for a plain
 * double series. Both handle flat input and single-point series gracefully. */

#include "abscom/abs.h"

#include <stdio.h>
#include <stdlib.h>

void abs_plot_ascii(const double *y, int n, int height) {
    if (!y || n <= 0) return;
    if (height <= 0) height = 10;

    double ymin = y[0], ymax = y[0];
    for (int i = 1; i < n; i++) {
        if (y[i] < ymin) ymin = y[i];
        if (y[i] > ymax) ymax = y[i];
    }
    if (ymax - ymin < 1e-9) { /* flat line: give it some vertical room */
        ymin -= 1.0;
        ymax += 1.0;
    }
    double span = ymax - ymin;

    char *grid = (char *)malloc((size_t)height * (size_t)n);
    if (!grid) return;
    for (int r = 0; r < height; r++)
        for (int c = 0; c < n; c++)
            grid[r * n + c] = ' ';

    for (int i = 0; i < n; i++) {
        int row = (int)((y[i] - ymin) / span * (height - 1) + 0.5);
        if (row < 0) row = 0;
        if (row >= height) row = height - 1;
        grid[row * n + i] = '*';
    }

    for (int r = height - 1; r >= 0; r--) {
        printf("|");
        for (int c = 0; c < n; c++) putchar(grid[r * n + c]);
        printf("|\n");
    }
    printf("+");
    for (int c = 0; c < n; c++) putchar('-');
    printf("+\n");
    printf("ymin=%.3f ymax=%.3f\n", ymin, ymax);
    free(grid);
}

static void abs_svg_emit(const char *filename, const double *xs,
                         const double *ys, int n) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "abs_plot_svg: cannot open %s\n", filename);
        return;
    }

    const int W = 800, H = 400, PAD = 40;
    double xmin = xs[0], xmax = xs[0], ymin = ys[0], ymax = ys[0];
    for (int i = 1; i < n; i++) {
        if (xs[i] < xmin) xmin = xs[i];
        if (xs[i] > xmax) xmax = xs[i];
        if (ys[i] < ymin) ymin = ys[i];
        if (ys[i] > ymax) ymax = ys[i];
    }
    if (xmax - xmin < 1e-9) { xmin -= 1.0; xmax += 1.0; }
    if (ymax - ymin < 1e-9) { ymin -= 1.0; ymax += 1.0; }

    double px = (double)(W - 2 * PAD) / (xmax - xmin);
    double py = (double)(H - 2 * PAD) / (ymax - ymin);

    fprintf(fp,
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%d\" "
            "height=\"%d\" viewBox=\"0 0 %d %d\">\n",
            W, H, W, H);
    fprintf(fp, "<rect width=\"%d\" height=\"%d\" fill=\"white\"/>\n", W, H);
    fprintf(fp, "<polyline points=\"");
    for (int i = 0; i < n; i++) {
        double sx = PAD + (xs[i] - xmin) * px;
        double sy = H - PAD - (ys[i] - ymin) * py;
        fprintf(fp, "%s%.1f,%.1f", i ? " " : "", sx, sy);
    }
    fprintf(fp, "\" fill=\"none\" stroke=\"#2563eb\" stroke-width=\"2\" "
                "stroke-linejoin=\"round\"/>\n");

    if (n == 1) {
        double sx = PAD + (xs[0] - xmin) * px;
        double sy = H - PAD - (ys[0] - ymin) * py;
        fprintf(fp, "<circle cx=\"%.1f\" cy=\"%.1f\" r=\"3\" fill=\"#2563eb\"/>\n",
                sx, sy);
    }

    fprintf(fp, "</svg>\n");
    fclose(fp);
}

void abs_plot_svg(const double *x, const double *y, int n,
                  const char *filename) {
    if (!y || !filename || n <= 0) return;
    double *xs = (double *)malloc((size_t)n * sizeof(double));
    if (!xs) return;
    if (x) {
        for (int i = 0; i < n; i++) xs[i] = x[i];
    } else {
        for (int i = 0; i < n; i++) xs[i] = (double)i;
    }
    abs_svg_emit(filename, xs, y, n);
    free(xs);
}
