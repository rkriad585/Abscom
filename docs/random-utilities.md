# Random Utilities

Deterministic random number generation with an explicit seed, plus convenience pickers and shuffles. The RNG is `rand()` based; call `seed` or `random_seed` to control it.

```c
#include "abscom/abs.h"
```

## Seeding

| Function | Description |
| --- | --- |
| `void seed(var obj)` | Seed the RNG from an int value or the hash of a string value. |
| `void random_seed(void)` | Seed from the current time. |

```c
seed(v(42));       /* reproducible sequence */
random_seed();     /* different every run  */
```

## Numbers

| Function | Description |
| --- | --- |
| `var randint(int min, int max)` | Uniform random int in `[min, max]` (bounds auto-swap). |
| `var random_float(void)` | Uniform float in `[0.0, 1.0)`. |
| `var uniform(double a, double b)` | Uniform float in `[a, b]`. |

## Collections

| Function | Description |
| --- | --- |
| `var choice(var seq)` | A random item from a non-empty list, or a random char from a string. |
| `var choices(var seq, int k)` | A new list of `k` random items/chars (repetition allowed). |
| `var sample(var seq, int k)` | A new list of `k` unique random items (lists only). |
| `void shuffle(var list)` | Randomize a list **in place** (returns nothing). |
| `void fisher_yates(var list)` | Same algorithm as `shuffle`, under its classic name. |
| `void riffle_shuffle(var list)` | In-place shuffle that splits the list in half and randomly interleaves the two piles (a card-deck riffle). |

Empty-sequence `choice`/`choices` and `k`-too-large `sample` produce `ABS_ERROR`. `sample`, `shuffle`, `fisher_yates`, and `riffle_shuffle` accept lists only; on non-lists they are no-ops.

## Example

```c
abs_init();
seed(v(42));

print(randint(1, 100));       /* deterministic given seed */
print(random_float());
print(uniform(10.0, 20.0));

var xs   = range(0, 10);      /* [0..9] */
print(choice(xs));
print(choices(xs, 5));

var uniq = sample(xs, 3);     /* new list, 3 unique items */
shuffle(xs);                  /* xs itself is permuted  */
print(uniq);
print(xs);

var deck = range(0, 13);      /* a hand of cards... */
riffle_shuffle(deck);         /* ...cut and interleaved */
fisher_yates(deck);           /* ...or fully randomized */
print(deck);
```

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
