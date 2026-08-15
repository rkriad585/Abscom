#include "abscom/abs.h"

#include <stdio.h>

static var heavy_calculation(var input) {
    printf("  [Thread] Calculating factorial of %ld...\n", input->val.i);
    sleep_sec(0.5);
    return factorial(input);
}

int main(void) {
    abs_init();

    print(v("--- 1. Matrix Operations ---"));
    var A = abs_matrix_new(2, 2);
    abs_matrix_set(A, 0, 0, 1.0);
    abs_matrix_set(A, 0, 1, 2.0);
    abs_matrix_set(A, 1, 0, 3.0);
    abs_matrix_set(A, 1, 1, 4.0);
    var I = abs_matrix_eye(2);

    print(v("Matrix A:"), A);
    print(v("A * Identity:"), abs_matrix_mul(A, I));
    print(v("Determinant(A):"), abs_matrix_det(A));

    print(v("--- 2. Statistics ---"));
    var data = List();
    append(data, v(10));
    append(data, v(20));
    append(data, v(20));
    append(data, v(40));
    print(v("Data:"), data);
    print(v("Mean:"), abs_stats_mean(data));
    print(v("Median:"), abs_stats_median(data));
    print(v("Mode:"), abs_stats_mode(data));
    print(v("Variance:"), abs_stats_variance(data));
    print(v("StDev:"), abs_stats_stdev(data));

    print(v("--- 3. Advanced Math ---"));
    print(v("Sin(45):"), sin_val(deg2rad(v(45))));
    print(v("Log10(100):"), log10_val(v(100)));
    print(v("Sqrt(9):"), sqrt_val(v(9)));

    print(v("--- 4. Combinatorics ---"));
    print(v("5! :"), factorial(v(5)));
    print(v("5 Choose 2:"), nCr(v(5), v(2)));
    print(v("5 Permute 2:"), nPr(v(5), v(2)));

    print(v("--- 5. CSV & Path ---"));
    var cwd = getcwd_val();
    var path = path_join(cwd, v("sci_output.csv"));
    print(v("Saving to:"), path);

    var csv_rows = List();
    append(csv_rows, data);
    csv_write(path->val.s, csv_rows);
    print(v("Read back:"), csv_read(path->val.s));
    remove(path->val.s);

    print(v("--- 6. Threading ---"));
    print(v("Main thread running... starting background task."));
    var t = thread_start(heavy_calculation, v(5));
    print(v("Main thread continuing work..."));
    var result = thread_join(t);
    print(v("Thread finished. Result:"), result);

    abs_cleanup();
    return 0;
}
