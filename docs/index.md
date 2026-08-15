# Abscom

<p align="center">
  <img src="https://raw.githubusercontent.com/rkriad585/Abscom/main/logo/logo.svg" alt="Abscom logo" width="150" height="150">
</p>

**Abscom** is a C11 library of reusable data structures and platform utilities, plus a Python-inspired dynamic runtime for scripting-style C programs.

- Zero third-party C dependencies — just the standard library, plus `ws2_32` on Windows.
- Dynamic array, growable string, hash functions, and an open-addressing hash map.
- Monotonic/wall-clock time and simple file I/O helpers.
- **Runtime**: `var` values, lists, dictionaries, sets, JSON, random utilities, functional helpers, and a light object system.
- **Scientific & AI/ML**: matrices, statistics, general math (number theory, geometry, complex numbers), CSV, threading, plus activations, softmax, MSE loss, and numerical gradients.
- **Data science**: NumPy-style reshaping and generators, Pandas-style CSV, functional utils, one-hot encoding, and train/test splits.
- **Ultimate**: computational backends (AVX/GPU), scalar autograd, PPM vision and convolution, plotting, and a mixed-type DataFrame.
- **Spatial math**: modern type aliases, anonymous-union 2D/3D/4D vectors, fixed-size matrices, and quaternions.
- **Macro utilities**: min/max/clamp, interpolation and smoothstep curves, generic swap, array/struct introspection, bitwise and alignment helpers, and AI activation macros.

## Get started

| | |
| --- | --- |
| [Getting Started](getting-started.md) | First steps and a walkthrough. |
| [Installation](installation.md) | Install via the one-line installer or build from source. |
| [Core library](common-macros.md) | Common macros, dynamic arrays, strings, hashing, hash maps, time, file I/O. |
| [Runtime](lifecycle.md) | Lifecycle, literals, lists and ranges, dictionaries, sets, JSON, and more. |

## Quick look

```c
#include "abscom/abs.h"

int main(void) {
    abs_init();
    var nums = List();
    append(nums, v(10));
    append(nums, v(20));
    append(nums, v(30));
    print(v("Sum:"), sum_val(nums));
    abs_cleanup();
    return 0;
}
```

```sh
cc -std=c11 hello.c -Iinclude build/libabscom.a -o hello
./hello
```

Expected output: `Sum: 60`.

## One-line install

Windows (PowerShell):

```powershell
irm https://raw.githubusercontent.com/rkriad585/Abscom/main/installer.ps1 | iex
```

Linux and macOS:

```sh
curl -fsSL https://raw.githubusercontent.com/rkriad585/Abscom/main/installer.sh | sh
```

See the [full README](https://github.com/rkriad585/Abscom/blob/main/README.md) on GitHub for the complete feature list and examples.
