# Dynamic Arrays

`abs_dynarray` is a generic, byte-memory dynamic array. You tell it the element size once; it stores raw bytes and grows automatically. It can hold any POD type, and is the workhorse data structure used internally by the runtime.

```c
#include "abscom/abs.h"
```

## Types

```c
typedef struct abs_dynarray {
    void  *data;
    size_t elem_size;
    size_t len;
    size_t cap;
} abs_dynarray_t;
```

| Field | Meaning |
| --- | --- |
| `data` | The backing buffer (`NULL` when empty). |
| `elem_size` | Size of one element in bytes. |
| `len` | Number of elements currently stored. |
| `cap` | Number of elements the buffer can hold before growing. |

## API

| Function | Description |
| --- | --- |
| `int abs_dynarray_init(abs_dynarray_t *arr, size_t elem_size)` | Initialize an array with a fixed element size. |
| `void abs_dynarray_destroy(abs_dynarray_t *arr)` | Free the backing buffer. |
| `void abs_dynarray_clear(abs_dynarray_t *arr)` | Set the length to zero, keeping capacity. |
| `int abs_dynarray_reserve(abs_dynarray_t *arr, size_t cap)` | Ensure capacity for at least `cap` elements. |
| `int abs_dynarray_push(abs_dynarray_t *arr, const void *elem)` | Append a copy of `elem`. |
| `void abs_dynarray_pop(abs_dynarray_t *arr)` | Remove the last element. |
| `int abs_dynarray_resize(abs_dynarray_t *arr, size_t new_len)` | Resize; newly added slots are zero-filled. |
| `void *abs_dynarray_at(abs_dynarray_t *arr, size_t index)` | Pointer to element `index`, or `NULL` if out of range. |
| `const void *abs_dynarray_at_const(const abs_dynarray_t *arr, size_t index)` | Const variant of `abs_dynarray_at`. |
| `void *abs_dynarray_data(abs_dynarray_t *arr)` | The raw backing buffer. |
| `size_t abs_dynarray_len(const abs_dynarray_t *arr)` | Number of elements. |
| `size_t abs_dynarray_cap(const abs_dynarray_t *arr)` | Allocated capacity in elements. |

Functions that can fail return `0` on success and `-1` on error.

## Example

```c
#include "abscom/abs.h"
#include <stdio.h>

int main(void) {
    abs_dynarray_t arr;
    abs_dynarray_init(&arr, sizeof(int));

    for (int i = 0; i < 100; i++) abs_dynarray_push(&arr, &i);

    printf("len=%zu cap=%zu\n", abs_dynarray_len(&arr), abs_dynarray_cap(&arr));
    int *first = (int *)abs_dynarray_at(&arr, 0);
    int *last  = (int *)abs_dynarray_at(&arr, 99);
    printf("first=%d last=%d\n", *first, *last);   /* 0 99 */

    abs_dynarray_destroy(&arr);
    return 0;
}
```

Memory note: the array copies `elem_size` bytes per `push`, so it is your job to manage any heap memory the elements point to. `abs_dynarray_destroy` frees only the buffer itself.

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
