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
- Language features:
  - Exceptions and context managers: `try`/`catch`/`end_try` + `throw` with a `setjmp`/`longjmp` env stack (`abs_except.c`), plus the `with(VAR, INIT)` macro and `close_resource`.
  - A small regex engine (`abs_regex.c`): `re_match`, `re_findall`, and `re_sub` with `.`/`*`/`^`/`$` support.
  - Date/time (`abs_datetime.c`): `datetime_now`, `strftime_val`, and `timedelta` on a new `ABS_TIME` type.
  - Generators (`abs_gen.c`): `range_gen`/`next` on a new `ABS_GENERATOR` type.
  - Encoding (`abs_encode.c`): `base64_encode` and RFC 4122 version-4 `uuid4`.
  - Environment variables (`abs_env.c`): `os_getenv`/`os_setenv` with a safe Windows `putenv` implementation.
  - `abs_new_file` to wrap a raw `FILE*` as an `ABS_FILE` object.
- Framework layer:
  - Micro web server (`abs_server.c`): `Server`/`route`/`server_run` on a new `ABS_SERVER` type, plus a socket-free `server_handle` dispatcher that returns `NULL` for 404.
  - Event emitter (`abs_events.c`): `EventBus`/`on`/`emit` with per-event handler lists on a new `ABS_FUNC` representation.
  - Dynamic plugins (`abs_plugins.c`): `load_library`/`call_lib_func` via `LoadLibrary` on Windows and `dlopen`/`dlsym` on POSIX, on a new `ABS_LIB` type.
  - Function objects (`abs_func.c`): `make_func`, `call_func`, `memoize`/`call_memoized` with an argument-keyed cache, and `decorate`/`func_meta` decorators.
  - Introspection (`abs_introspect.c`): `id`, `repr`, and `dir`.
  - Itertools (`abs_itertools.c`): `chain` and `cycle` iterators plus `iter_next` on a new `ABS_ITERATOR` type.
- Algorithm suite:
  - Sorting (`abs_sort.c`): twelve algorithms on the runtime's lists — bubble, selection, insertion (O(n²)), shell, heap, merge, quick, and a C `qsort` wrapper (O(n log n)), plus counting, radix, and bucket sorts (O(n), integers only) and the joke `sort_bogo`.
  - Visualization: `sort_bubble_visual` calls an `AbsSortVis` hook on every swap.
  - Benchmarking: `timeit` times any sort function on a deep-copied list and returns seconds.
  - Search and helpers: `binary_search` for sorted lists, plus `is_sorted` and `list_copy`.
- Realtime and crypto layer:
  - WebSockets (`abs_ws.c`): RFC 6455 server handshake (`ws_accept`, with a real SHA-1 implementation), text-frame send/receive (`ws_send`/`ws_recv`, masked client frames unmasked on receipt), and socket-free `ws_compute_accept` / `ws_encode_frame` / `ws_decode_frame` helpers for tests. New `abs_socket` handle (SOCKET on Windows, `int` on POSIX).
  - Cryptography (`abs_crypto.c`): from-scratch `sha256` and `hmac_sha256` (FIPS 180-4 / RFC 2104), returned as lowercase hex strings and pinned to known-answer test vectors.
  - Shuffling (`abs.c`): `fisher_yates` (classic name for `shuffle`) and `riffle_shuffle`, which cuts a list in half and randomly interleaves the two piles.
  - Itertools (`abs_itertools.c`): `repeat(val, n)` iterator alongside `chain`/`cycle`, all driven by `iter_next`.
- Decorator layer:
  - Named function objects (`abs_func.c`): `def(f, name)` adds a display name to `make_func`; `func_name` reads it back, and `print`/`str` render it as `<function name>`.
  - Target-aware decorators (`abs_func.c`): `decorate_func(target, wrapper)` builds a wrapper that receives `(target, args)` and calls back into `call_func(target, args)`, so pre/post logic can run around the original (Python `@decorator` style). `call_func` dispatches plain, memoized, and decorated functions transparently; the original stays reachable via `func_meta`/`func_name`.
  - The WebSocket API (handshake and framing) from the 13.0-style announcement was already delivered by the realtime and crypto layer.
- Build system: Meson project with static + shared libraries, twenty-six tests, and nine example programs.
- Documentation: README, `docs/` (including the language-features, framework, algorithm-suite, and realtime-and-crypto pages), logo, and standard project files.
