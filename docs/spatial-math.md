# Spatial Math Layer

A lightweight spatial-math module: modern type aliases (Rust/NumPy style), 2D/3D/4D vectors with anonymous-union component aliasing, fixed-size matrices, and quaternions. Everything is a plain value type — no allocation, no GC tracking, no `free` needed.

## Type aliases

`abs.h` defines the modern fixed-width aliases used throughout this layer:

| Alias | C type | Alias | C type |
| --- | --- | --- | --- |
| `i8` / `i16` / `i32` / `i64` | `int8_t` / `int16_t` / `int32_t` / `int64_t` | `u8` / `u16` / `u32` / `u64` | `uint8_t` / `uint16_t` / `uint32_t` / `uint64_t` |
| `isize` | `ptrdiff_t` | `usize` | `size_t` |
| `f32` | `float` | `f64` | `double` |
| `b8` | `bool` | `b32` | `uint32_t` |
| `byte` | `unsigned char` | | |

```c
i32 score = 100;
u64 big = 1ULL << 40;
f32 pi_f = (f32)ABS_PI;
f64 pi = ABS_PI;
byte raw = 0xAB;
```

## Vectors

Anonymous-union vectors overlap their components, so `v.x`, `v.u`, and `v.raw[0]` all read the same memory. `vec3` also exposes color aliases (`r`/`g`/`b`) and `vec4` adds alpha (`a`).

| Type | Components | Aliases |
| --- | --- | --- |
| `vec2` / `vec2d` / `ivec2` | `x`, `y` | `u`, `v` (and `raw[]`) |
| `vec3` / `vec3d` / `ivec3` | `x`, `y`, `z` | `r`, `g`, `b` |
| `vec4` / `vec4d` / `ivec4` | `x`, `y`, `z`, `w` | `r`, `g`, `b`, `a` |
| `quat` | `x`, `y`, `z` (vector), `w` (scalar) | `raw[]` |

Construct with the static inline helpers `v2`, `v3`, `v4`, `iv2`, `iv3`, `iv4`, and `q4`.

```c
vec2 p = v2(1.0f, 2.0f);
vec3 c = v3(0.2f, 0.4f, 0.6f);
printf("%f %f %f\n", (double)c.r, (double)c.g, (double)c.b);
vec4 rgba = v4(1.0f, 0.5f, 0.25f, 0.1f);
printf("%f\n", (double)rgba.a);          /* 0.10 */
```

### vec2 operations

| Function | Description |
| --- | --- |
| `vec2 abs_v2_add(vec2 a, vec2 b)` | Component-wise addition. |
| `vec2 abs_v2_sub(vec2 a, vec2 b)` | Component-wise subtraction. |
| `vec2 abs_v2_scale(vec2 a, f32 s)` | Multiply by a scalar. |
| `f32 abs_v2_dot(vec2 a, vec2 b)` | Dot product. |
| `f32 abs_v2_cross(vec2 a, vec2 b)` | 2D cross product (`a.x*b.y - a.y*b.x`). |
| `f32 abs_v2_len(vec2 v)` | Length (Euclidean norm). |
| `f32 abs_v2_dist(vec2 a, vec2 b)` | Distance between two points. |
| `vec2 abs_v2_norm(vec2 v)` | Unit vector (zero for the zero vector). |
| `vec2 abs_v2_lerp(vec2 a, vec2 b, f32 t)` | Linear interpolation. |
| `vec2 abs_v2_reflect(vec2 v, vec2 n)` | Reflect `v` about a unit normal `n`: `v - 2*dot(v,n)*n`. |
| `void abs_v2_print(vec2 v, const char *name)` | Print `name = (x, y)`. |

### vec3 operations

`abs_v3_add`, `abs_v3_sub`, `abs_v3_scale`, `abs_v3_dot`, `abs_v3_cross`, `abs_v3_len`, `abs_v3_dist`, `abs_v3_norm`, `abs_v3_lerp`, `abs_v3_reflect`, and `abs_v3_print` mirror the 2D set, with `abs_v3_cross` returning the proper 3D cross product.

```c
vec3 n = abs_v3_norm(abs_v3_cross(v3(1, 0, 0), v3(0, 1, 0)));
/* n == (0, 0, 1) */
```

### vec4 operations

`abs_v4_add`, `abs_v4_sub`, `abs_v4_scale`, `abs_v4_dot`, `abs_v4_len`, `abs_v4_dist`, `abs_v4_norm`, `abs_v4_lerp`, and `abs_v4_print`.

## Quaternions

| Function | Description |
| --- | --- |
| `quat abs_quat_ident(void)` | The identity quaternion `(0, 0, 0, 1)`. |
| `quat abs_quat_mul(quat a, quat b)` | Hamilton product; composes rotation `b` (applied first), then `a`. |
| `quat abs_quat_norm(quat q)` | Unit quaternion (identity for a zero quaternion). |
| `quat abs_quat_from_axis_angle(f32 ax, f32 ay, f32 az, f32 angle)` | Rotation about an axis by `angle` radians. |
| `vec3 abs_quat_rotate_vec3(quat q, vec3 v)` | Rotate `v` by `q` (via `v + 2w(r×v) + 2(r×(r×v))`). |
| `void abs_quat_print(quat q, const char *name)` | Print `name = (x, y, z, w)`. |

```c
quat q = abs_quat_from_axis_angle(0, 0, 1, ABS_PI / 2);
vec3 p = abs_quat_rotate_vec3(q, v3(1, 0, 0));
/* p == (0, 1, 0): a 90-degree turn about +Z */

quat twice = abs_quat_mul(q, q);        /* 180 degrees about +Z */
vec3 p2 = abs_quat_rotate_vec3(twice, v3(1, 0, 0));
/* p2 == (-1, 0, 0) */
```

## mat4

Column-major 4×4 matrices, so `m.m[col][row]` addresses a column first and the columns are readable directly as `vec4`s via `m.cols[i]`.

| Function | Description |
| --- | --- |
| `mat4 abs_mat4_identity(void)` | Identity matrix. |
| `mat4 abs_mat4_mul(mat4 a, mat4 b)` | Matrix product (`a` applied after `b`). |
| `vec4 abs_mat4_mul_vec4(mat4 m, vec4 v)` | Transform a point/homogeneous vector. |
| `mat4 abs_mat4_translate(f32 x, f32 y, f32 z)` | Translation matrix. |
| `mat4 abs_mat4_scale(f32 x, f32 y, f32 z)` | Scaling matrix. |
| `mat4 abs_mat4_rotate_x(f32 angle)` | Rotation about the X axis. |
| `mat4 abs_mat4_rotate_y(f32 angle)` | Rotation about the Y axis. |
| `mat4 abs_mat4_rotate_z(f32 angle)` | Rotation about the Z axis. |
| `mat4 abs_mat4_perspective(f32 fov_y, f32 aspect, f32 near, f32 far)` | OpenGL-style perspective projection. |
| `mat4 abs_mat4_look_at(vec3 eye, vec3 center, vec3 up)` | Right-handed view matrix. |
| `void abs_mat4_print(mat4 m, const char *name)` | Print the matrix row by row. |

```c
mat4 model = abs_mat4_translate(10.0f, 0.0f, 5.0f);
model = abs_mat4_mul(model, abs_mat4_scale(2.0f, 2.0f, 2.0f));
model = abs_mat4_mul(model, abs_mat4_rotate_y(ABS_PI / 4.0f));

vec4 world = abs_mat4_mul_vec4(model, v4(1.0f, 1.0f, 1.0f, 1.0f));
/* world == (12.83, 2.00, 5.00, 1.00) */

mat4 view = abs_mat4_look_at(v3(0, 0, 5), v3(0, 0, 0), v3(0, 1, 0));
mat4 proj = abs_mat4_perspective((f32)(60 * ABS_PI / 180), 16.0f / 9.0f, 0.1f, 100.0f);
```

## Example

See `examples/geom_demo.c` (`build/examples/geom_demo`), which prints the type aliases, bounces a 2D ball off the ground, computes a cross-product normal, rotates a point with a quaternion, builds a model matrix, and projects through a camera.

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).

For the matrix types and operations this layer's `mat4` complements, see [Matrices, Statistics, and More](scientific.md).
