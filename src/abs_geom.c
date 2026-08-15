/* Spatial math: modern type aliases, anonymous-union 2D/3D/4D vectors,
 * fixed-size matrices, and quaternions. All plain value types — passed by
 * value with no heap internals and no GC tracking. */

#include "abscom/abs.h"

#include <math.h>
#include <stdio.h>

/* --- vec2 --- */

vec2 abs_v2_add(vec2 a, vec2 b) {
    return v2(a.x + b.x, a.y + b.y);
}

vec2 abs_v2_sub(vec2 a, vec2 b) {
    return v2(a.x - b.x, a.y - b.y);
}

vec2 abs_v2_scale(vec2 a, f32 s) {
    return v2(a.x * s, a.y * s);
}

f32 abs_v2_dot(vec2 a, vec2 b) {
    return a.x * b.x + a.y * b.y;
}

f32 abs_v2_cross(vec2 a, vec2 b) {
    return a.x * b.y - a.y * b.x;
}

f32 abs_v2_len(vec2 v) {
    return sqrtf(v.x * v.x + v.y * v.y);
}

f32 abs_v2_dist(vec2 a, vec2 b) {
    return abs_v2_len(abs_v2_sub(a, b));
}

vec2 abs_v2_norm(vec2 v) {
    f32 l = abs_v2_len(v);
    if (l == 0.0f) return v2(0.0f, 0.0f);
    return v2(v.x / l, v.y / l);
}

vec2 abs_v2_lerp(vec2 a, vec2 b, f32 t) {
    return v2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t);
}

vec2 abs_v2_reflect(vec2 v, vec2 n) {
    f32 d = abs_v2_dot(v, n);
    return v2(v.x - 2.0f * d * n.x, v.y - 2.0f * d * n.y);
}

void abs_v2_print(vec2 v, const char *name) {
    printf("%s = (%.2f, %.2f)\n", name, (double)v.x, (double)v.y);
}

/* --- vec3 --- */

vec3 abs_v3_add(vec3 a, vec3 b) {
    return v3(a.x + b.x, a.y + b.y, a.z + b.z);
}

vec3 abs_v3_sub(vec3 a, vec3 b) {
    return v3(a.x - b.x, a.y - b.y, a.z - b.z);
}

vec3 abs_v3_scale(vec3 a, f32 s) {
    return v3(a.x * s, a.y * s, a.z * s);
}

f32 abs_v3_dot(vec3 a, vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

vec3 abs_v3_cross(vec3 a, vec3 b) {
    return v3(a.y * b.z - a.z * b.y,
              a.z * b.x - a.x * b.z,
              a.x * b.y - a.y * b.x);
}

f32 abs_v3_len(vec3 v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

f32 abs_v3_dist(vec3 a, vec3 b) {
    return abs_v3_len(abs_v3_sub(a, b));
}

vec3 abs_v3_norm(vec3 v) {
    f32 l = abs_v3_len(v);
    if (l == 0.0f) return v3(0.0f, 0.0f, 0.0f);
    return v3(v.x / l, v.y / l, v.z / l);
}

vec3 abs_v3_lerp(vec3 a, vec3 b, f32 t) {
    return v3(a.x + (b.x - a.x) * t,
              a.y + (b.y - a.y) * t,
              a.z + (b.z - a.z) * t);
}

vec3 abs_v3_reflect(vec3 v, vec3 n) {
    f32 d = abs_v3_dot(v, n);
    return v3(v.x - 2.0f * d * n.x,
              v.y - 2.0f * d * n.y,
              v.z - 2.0f * d * n.z);
}

void abs_v3_print(vec3 v, const char *name) {
    printf("%s = (%.2f, %.2f, %.2f)\n", name, (double)v.x, (double)v.y,
           (double)v.z);
}

/* --- vec4 --- */

vec4 abs_v4_add(vec4 a, vec4 b) {
    return v4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

vec4 abs_v4_sub(vec4 a, vec4 b) {
    return v4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

vec4 abs_v4_scale(vec4 a, f32 s) {
    return v4(a.x * s, a.y * s, a.z * s, a.w * s);
}

f32 abs_v4_dot(vec4 a, vec4 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

f32 abs_v4_len(vec4 v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
}

f32 abs_v4_dist(vec4 a, vec4 b) {
    return abs_v4_len(abs_v4_sub(a, b));
}

vec4 abs_v4_norm(vec4 v) {
    f32 l = abs_v4_len(v);
    if (l == 0.0f) return v4(0.0f, 0.0f, 0.0f, 0.0f);
    return v4(v.x / l, v.y / l, v.z / l, v.w / l);
}

vec4 abs_v4_lerp(vec4 a, vec4 b, f32 t) {
    return v4(a.x + (b.x - a.x) * t,
              a.y + (b.y - a.y) * t,
              a.z + (b.z - a.z) * t,
              a.w + (b.w - a.w) * t);
}

void abs_v4_print(vec4 v, const char *name) {
    printf("%s = (%.2f, %.2f, %.2f, %.2f)\n", name, (double)v.x, (double)v.y,
           (double)v.z, (double)v.w);
}

/* --- Quaternions --- */

quat abs_quat_ident(void) {
    return q4(0.0f, 0.0f, 0.0f, 1.0f);
}

quat abs_quat_mul(quat a, quat b) {
    quat r;
    r.w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
    r.x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
    r.y = a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x;
    r.z = a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w;
    return r;
}

quat abs_quat_norm(quat q) {
    f32 l = sqrtf(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
    if (l == 0.0f) return abs_quat_ident();
    return q4(q.x / l, q.y / l, q.z / l, q.w / l);
}

quat abs_quat_from_axis_angle(f32 ax, f32 ay, f32 az, f32 angle) {
    f32 half = angle * 0.5f;
    f32 s = sinf(half);
    f32 c = cosf(half);
    f32 len = sqrtf(ax * ax + ay * ay + az * az);
    if (len == 0.0f) return abs_quat_ident();
    return q4(ax / len * s, ay / len * s, az / len * s, c);
}

vec3 abs_quat_rotate_vec3(quat q, vec3 v) {
    vec3 qv = v3(q.x, q.y, q.z);
    vec3 t = abs_v3_scale(abs_v3_cross(qv, v), 2.0f);
    vec3 cross_term = abs_v3_cross(qv, t);
    vec3 scale_term = abs_v3_scale(t, q.w);
    return abs_v3_add(v, abs_v3_add(scale_term, cross_term));
}

void abs_quat_print(quat q, const char *name) {
    printf("%s = (%.2f, %.2f, %.2f, %.2f)\n", name, (double)q.x, (double)q.y,
           (double)q.z, (double)q.w);
}

/* --- mat4 (column-major) --- */

mat4 abs_mat4_identity(void) {
    mat4 m = {{{0.0f}}};
    m.m[0][0] = 1.0f;
    m.m[1][1] = 1.0f;
    m.m[2][2] = 1.0f;
    m.m[3][3] = 1.0f;
    return m;
}

mat4 abs_mat4_mul(mat4 a, mat4 b) {
    mat4 r;
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            f32 sum = 0.0f;
            for (int k = 0; k < 4; k++)
                sum += a.m[k][row] * b.m[col][k];
            r.m[col][row] = sum;
        }
    }
    return r;
}

vec4 abs_mat4_mul_vec4(mat4 m, vec4 v) {
    return v4(m.m[0][0] * v.x + m.m[1][0] * v.y + m.m[2][0] * v.z + m.m[3][0] * v.w,
              m.m[0][1] * v.x + m.m[1][1] * v.y + m.m[2][1] * v.z + m.m[3][1] * v.w,
              m.m[0][2] * v.x + m.m[1][2] * v.y + m.m[2][2] * v.z + m.m[3][2] * v.w,
              m.m[0][3] * v.x + m.m[1][3] * v.y + m.m[2][3] * v.z + m.m[3][3] * v.w);
}

mat4 abs_mat4_translate(f32 x, f32 y, f32 z) {
    mat4 m = abs_mat4_identity();
    m.m[3][0] = x;
    m.m[3][1] = y;
    m.m[3][2] = z;
    return m;
}

mat4 abs_mat4_scale(f32 x, f32 y, f32 z) {
    mat4 m = abs_mat4_identity();
    m.m[0][0] = x;
    m.m[1][1] = y;
    m.m[2][2] = z;
    return m;
}

mat4 abs_mat4_rotate_x(f32 angle) {
    mat4 m = abs_mat4_identity();
    f32 c = cosf(angle), s = sinf(angle);
    m.m[1][1] = c;
    m.m[1][2] = s;
    m.m[2][1] = -s;
    m.m[2][2] = c;
    return m;
}

mat4 abs_mat4_rotate_y(f32 angle) {
    mat4 m = abs_mat4_identity();
    f32 c = cosf(angle), s = sinf(angle);
    m.m[0][0] = c;
    m.m[0][2] = -s;
    m.m[2][0] = s;
    m.m[2][2] = c;
    return m;
}

mat4 abs_mat4_rotate_z(f32 angle) {
    mat4 m = abs_mat4_identity();
    f32 c = cosf(angle), s = sinf(angle);
    m.m[0][0] = c;
    m.m[0][1] = s;
    m.m[1][0] = -s;
    m.m[1][1] = c;
    return m;
}

mat4 abs_mat4_perspective(f32 fov_y, f32 aspect, f32 near, f32 far) {
    mat4 m = {{{0.0f}}};
    f32 f = 1.0f / tanf(fov_y * 0.5f);
    m.m[0][0] = f / aspect;
    m.m[1][1] = f;
    m.m[2][2] = (far + near) / (near - far);
    m.m[3][2] = (2.0f * far * near) / (near - far);
    m.m[2][3] = -1.0f;
    return m;
}

mat4 abs_mat4_look_at(vec3 eye, vec3 center, vec3 up) {
    mat4 m = {{{0.0f}}};
    vec3 f = abs_v3_norm(abs_v3_sub(center, eye));
    vec3 s = abs_v3_norm(abs_v3_cross(f, up));
    vec3 u = abs_v3_cross(s, f);
    m.m[0][0] = s.x;  m.m[1][0] = s.y;  m.m[2][0] = s.z;  m.m[3][0] = -abs_v3_dot(s, eye);
    m.m[0][1] = u.x;  m.m[1][1] = u.y;  m.m[2][1] = u.z;  m.m[3][1] = -abs_v3_dot(u, eye);
    m.m[0][2] = -f.x; m.m[1][2] = -f.y; m.m[2][2] = -f.z; m.m[3][2] = abs_v3_dot(f, eye);
    m.m[3][3] = 1.0f;
    return m;
}

void abs_mat4_print(mat4 m, const char *name) {
    printf("%s =\n", name);
    for (int row = 0; row < 4; row++)
        printf("  [ %6.2f %6.2f %6.2f %6.2f ]\n",
               (double)m.m[0][row], (double)m.m[1][row],
               (double)m.m[2][row], (double)m.m[3][row]);
}
