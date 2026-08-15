/* Data science demo: NumPy-style shapes and generators, Pandas-style CSV,
 * functional utils, and SciKit-Learn-style preprocessing. */

#include "abscom/abs.h"

#include <stdio.h>

static double double_it(double x) { return x * 2.0; }
static int is_positive(double x) { return x > 0.0; }

int main(void) {
    abs_init();

    printf("=== Abscom Data Science Layer ===\n\n");

    /* 1. Generators & reshaping */
    printf("--- 1. Generators & reshaping ---\n");
    var data = abs_matrix_arange(0.0, 12.0, 1.0);   /* arange(0, 12) */
    var matrix = abs_matrix_reshape(data, 4, 3);    /* .reshape(4, 3) */
    print_mat(matrix);

    /* 2. Slicing (rows 1-3, cols 0-2) */
    printf("\n--- 2. Slicing (rows 1-3, cols 0-2) ---\n");
    var sub = abs_matrix_slice(matrix, 1, 3, 0, 2);
    print_mat(sub);

    /* 3. CSV I/O */
    printf("\n--- 3. CSV I/O ---\n");
    abs_matrix_write_csv(matrix, "test_data.csv");
    printf("Saved test_data.csv. Reading it back...\n");
    var loaded = abs_matrix_read_csv("test_data.csv");
    print_mat(loaded);

    /* 4. ML preprocessing */
    printf("\n--- 4. ML preprocessing ---\n");
    var labels = abs_matrix_new(4, 1);
    abs_matrix_set(labels, 0, 0, 0.0);
    abs_matrix_set(labels, 1, 0, 1.0);
    abs_matrix_set(labels, 2, 0, 2.0);
    abs_matrix_set(labels, 3, 0, 0.0);

    printf("One-hot encoding:\n");
    var y_oh = abs_matrix_one_hot_encode(labels, 3);
    print_mat(y_oh);

    printf("\nTrain/test split (75%% train):\n");
    var splits = abs_matrix_train_test_split(matrix, labels, 0.25);
    printf("X_train:\n");
    print_mat(get(splits, 0));
    printf("X_test:\n");
    print_mat(get(splits, 1));
    printf("Y_train:\n");
    print_mat(get(splits, 2));
    printf("Y_test:\n");
    print_mat(get(splits, 3));

    /* 5. Macros, map and filter */
    printf("\n--- 5. Macros, map and filter ---\n");
    printf("foreach_mat over sub: ");
    double val;
    foreach_mat(val, sub) printf("%.1f ", val);
    printf("\n");

    printf("map(matrix, double):\n");
    var doubled = abs_matrix_map(matrix, double_it);
    print_mat(doubled);

    var range_shifted = abs_matrix_arange(-5.0, 5.0, 1.0);
    var pos_only = abs_matrix_filter(range_shifted, is_positive);
    printf("filter(arange(-5, 5), x > 0):\n");
    print_mat(pos_only);

    abs_cleanup();
    return 0;
}
