# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.1.0] - Unreleased

### Added

- Core library (`abscom`):
  - Dynamic array (`ac_dynarray`) with reserve, push, pop, resize, and indexed access.
  - Growable string (`ac_string`) with append (C string, bytes, char, formatted), shrink-to-fit, and ownership transfer.
  - Hash functions (`ac_hash`): FNV-1a 32/64 and djb2.
  - Open-addressing string-keyed hash map (`ac_hashmap`) with tombstone deletion, resizing, and optional value free callbacks.
  - Platform helpers: monotonic/wall-clock time (`ac_time`) and file I/O (`ac_fs`).
  - Umbrella header `abscom/ac.h` and `AC_API` export macros (`ac_common.h`).
- Python-like runtime (`ac_py`):
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
- Build system: Meson project with static + shared libraries, six tests, and four example programs.
- Documentation: README, `docs/`, logo, and standard project files.
