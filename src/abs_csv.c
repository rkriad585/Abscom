#include "abscom/abs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Simple CSV: fields are split on commas; quoted fields are not supported. */
var csv_read(const char *filename) {
    if (!filename) return abs_new_error("csv_read expects a filename");
    FILE *f = fopen(filename, "r");
    if (!f) return abs_new_error("File not found");
    var rows = abs_new_list();
    char line[4096];
    while (fgets(line, (int)sizeof(line), f)) {
        line[strcspn(line, "\r\n")] = '\0';
        var row = abs_new_list();
        const char *p = line;
        char field[4096];
        for (;;) {
            const char *comma = strchr(p, ',');
            size_t n = comma ? (size_t)(comma - p) : strlen(p);
            if (n >= sizeof(field)) n = sizeof(field) - 1;
            memcpy(field, p, n);
            field[n] = '\0';
            char *end = NULL;
            long iv = strtol(field, &end, 10);
            if (end != field && *end == '\0') {
                append(row, abs_new_int(iv));
            } else {
                double dv = strtod(field, &end);
                if (end != field && *end == '\0')
                    append(row, abs_new_float(dv));
                else
                    append(row, abs_new_str(field));
            }
            if (!comma) break;
            p = comma + 1;
        }
        append(rows, row);
    }
    fclose(f);
    return rows;
}

void csv_write(const char *filename, var rows) {
    if (!filename || !rows || rows->type != ABS_LIST) return;
    FILE *f = fopen(filename, "w");
    if (!f) return;
    for (size_t i = 0; i < rows->val.list.size; i++) {
        var row = rows->val.list.items[i];
        if (!row || row->type != ABS_LIST) continue;
        for (size_t j = 0; j < row->val.list.size; j++) {
            var v = row->val.list.items[j];
            if (!v) continue;
            if (v->type == ABS_INT)
                fprintf(f, "%ld", v->val.i);
            else if (v->type == ABS_FLOAT)
                fprintf(f, "%g", v->val.f);
            else if (v->type == ABS_STR)
                fprintf(f, "%s", v->val.s);
            if (j + 1 < row->val.list.size) fputc(',', f);
        }
        fputc('\n', f);
    }
    fclose(f);
}
