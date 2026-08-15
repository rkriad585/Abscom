/* Mixed-type DataFrame: named columns of doubles or strings over a shared row
 * count. All column data is copied in, so the caller keeps ownership of its
 * input arrays; release the whole frame with abs_df_free(). */

#include "abscom/abs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *abs_df_strdup(const char *s) {
    if (!s) s = "";
    size_t len = strlen(s) + 1;
    char *copy = (char *)malloc(len);
    if (copy) memcpy(copy, s, len);
    return copy;
}

AbsDF *abs_df_create(int rows) {
    if (rows < 0) return NULL;
    AbsDF *df = (AbsDF *)malloc(sizeof(AbsDF));
    if (!df) return NULL;
    df->rows = rows;
    df->col_count = 0;
    df->cols = NULL;
    return df;
}

static AbsCol *abs_df_new_col(const char *name, AbsColType type, int rows) {
    AbsCol *col = (AbsCol *)malloc(sizeof(AbsCol));
    if (!col) return NULL;
    col->name = abs_df_strdup(name);
    col->type = type;
    col->rows = rows;
    col->capacity = rows > 0 ? rows : 1;
    col->doubles = NULL;
    col->strings = NULL;
    if (type == ABS_COL_DOUBLE) {
        col->doubles = (double *)calloc((size_t)col->capacity, sizeof(double));
        if (!col->doubles) {
            free(col->name);
            free(col);
            return NULL;
        }
    } else {
        col->strings = (char **)calloc((size_t)col->capacity, sizeof(char *));
        if (!col->strings) {
            free(col->name);
            free(col);
            return NULL;
        }
    }
    return col;
}

static void abs_df_push_col(AbsDF *df, AbsCol *col) {
    AbsCol **cols = (AbsCol **)realloc(df->cols,
                                       (size_t)(df->col_count + 1) *
                                           sizeof(AbsCol *));
    if (!cols) {
        /* Cannot grow the column list: leak the column rather than crash. */
        return;
    }
    df->cols = cols;
    df->cols[df->col_count++] = col;
}

void abs_df_add_col_double(AbsDF *df, const char *name, const double *values) {
    if (!df || !name || !values) return;
    AbsCol *col = abs_df_new_col(name, ABS_COL_DOUBLE, df->rows);
    if (!col) return;
    for (int i = 0; i < df->rows; i++)
        col->doubles[i] = values[i];
    abs_df_push_col(df, col);
}

void abs_df_add_col_string(AbsDF *df, const char *name,
                           const char *const *values) {
    if (!df || !name || !values) return;
    AbsCol *col = abs_df_new_col(name, ABS_COL_STRING, df->rows);
    if (!col) return;
    for (int i = 0; i < df->rows; i++)
        col->strings[i] = abs_df_strdup(values[i]);
    abs_df_push_col(df, col);
}

static void abs_df_cell(const AbsCol *col, int row, char *buf, size_t cap) {
    if (col->type == ABS_COL_DOUBLE) {
        snprintf(buf, cap, "%.2f", col->doubles[row]);
    } else {
        snprintf(buf, cap, "%s", col->strings[row] ? col->strings[row] : "");
    }
}

void abs_df_print(const AbsDF *df) {
    if (!df) return;
    printf("Abscom DataFrame (%d rows, %d cols):\n", df->rows, df->col_count);
    if (df->col_count == 0) return;

    int *widths = (int *)calloc((size_t)df->col_count, sizeof(int));
    if (!widths) return;
    for (int c = 0; c < df->col_count; c++) {
        int w = (int)strlen(df->cols[c]->name);
        if (df->cols[c]->type == ABS_COL_DOUBLE && w < 7) w = 7; /* "%.2f" */
        for (int r = 0; r < df->rows; r++) {
            char cell[256];
            abs_df_cell(df->cols[c], r, cell, sizeof(cell));
            int l = (int)strlen(cell);
            if (l > w) w = l;
        }
        widths[c] = w;
    }

    printf("| ");
    for (int c = 0; c < df->col_count; c++)
        printf("%-*s | ", widths[c], df->cols[c]->name);
    printf("\n");

    for (int c = 0; c < df->col_count; c++) {
        printf("+");
        for (int i = 0; i < widths[c] + 2; i++) putchar('-');
    }
    printf("+\n");

    for (int r = 0; r < df->rows; r++) {
        printf("| ");
        for (int c = 0; c < df->col_count; c++) {
            char cell[256];
            abs_df_cell(df->cols[c], r, cell, sizeof(cell));
            printf("%-*s | ", widths[c], cell);
        }
        printf("\n");
    }
    free(widths);
}

void abs_df_free(AbsDF *df) {
    if (!df) return;
    for (int c = 0; c < df->col_count; c++) {
        AbsCol *col = df->cols[c];
        if (col->type == ABS_COL_STRING) {
            for (int r = 0; r < col->rows; r++) free(col->strings[r]);
        }
        free(col->doubles);
        free(col->strings);
        free(col->name);
        free(col);
    }
    free(df->cols);
    free(df);
}
