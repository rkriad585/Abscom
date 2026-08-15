# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.1.0] - Unreleased

### Added

- Core library (`abscom`):
  - Dynamic array (`abs_dynarray`) with reserve, push, pop, resize, and indexed access.
  - Growable string (`abs_string`) with append (C string, bytes, char, formatted), shrink-to-fit, and ownership transfer.
  - Hash functions (`abs_hash`): FNV-1a 32/64 and djb2.
  - Open-addressing string-keyed hash map (`abs_hashmap`) with tombstone deletion, resizing, and optional value free callbacks.
  - Platform helpers: monotonic/wall-clock time (`abs_time`) and file I/O (`abs_fs`).
  - Umbrella header `abscom/abs.h` and `ABS_API` export macros (`abs_common.h`).
- Python-like runtime:
  - `var` object model with `v()` literal macro, `None`/`True`/`False`, and `List()`/`Dict()`/`Set()` constructors.
  - Memory-pool allocator for `AbsObj` values.
  - Printing (`print`/`print_end`), input, and arithmetic (`add`/`sub`/`mul`/`eq`).
  - Conversions (`to_str`/`to_int`/`to_float`), dictionaries (`dset`/`dget`).
  - Lists, negative indexing, `range`, `range_step`, `slice`, and the `foreach` macro.
  - Strings: split/join, strip, case conversion, startswith/endswith, count.
  - Functional helpers: `map_func`, `filter_func`, `list_comp`.
  - Formatting via `fmt(...)` with `{}` placeholders.
  - File objects (`fopen_safe`/`read_file`/`write_file`/`close_file`).
  - JSON parsing (`json_parse`) and stringifying (`json_dump`).
  - Random utilities: `randint`, `random_float`, `uniform`, `choice`, `choices`, `sample`, `shuffle`, `seed`/`random_seed`.
  - Aggregates and math: `min_val`, `max_val`, `sum_val`, `abs_val`, `pow_val`, `round_val`, `not_`, `any`, `all`.
  - Sequences: `sorted`, `reversed_seq`, `zip_lists`.
  - Type introspection: `type`, `is_int`/`is_float`/`is_str`/`is_list`/`is_dict`/`is_set`/`is_none`, `is_err`, `is_true`.
  - Sets: `set_add`, `set_contains`, `set_union`, `set_diff`.
  - OOP-lite: `Class`/`New`/`set_attr`/`get_attr`.
  - System helpers: `sleep_sec`, `time_now`, `exec_cmd`, and HTTP/1.0 `http_get`.
- Scientific layer:
  - Matrices (`abs_matrix_*`): zero/identity constructors, get/set, multiplication, transpose, determinant (Laplace expansion), and printing.
  - Statistics (`abs_stats_*`): `mean`, `median`, `mode`, population `variance`, and population `stdev`.
  - Advanced math: `sin_val`, `cos_val`, `tan_val`, `log_val`, `log10_val`, `sqrt_val`, `deg2rad`.
  - Combinatorics: `factorial`, `nCr`, `nPr`.
  - Paths and OS helpers: `path_join`, `path_exists`, `getcwd_val`.
  - CSV: `csv_read` and `csv_write`.
  - Threading: `thread_start`/`thread_join` with a new `ABS_THREAD` type and lock-guarded memory pool for thread-safe allocation.
- Build system: Meson project with static + shared libraries, eleven tests, and five example programs.
- Documentation: README, `docs/`, logo, and standard project files.
