#include "abscom/abs.h"

#include <math.h>
#include <stdio.h>

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);  \
            return 1;                                                        \
        }                                                                    \
    } while (0)

static int close_to(double a, double b) {
    return fabs(a - b) < 1e-4;
}

static int v2_close(vec2 a, vec2 b) {
    return close_to(a.x, b.x) && close_to(a.y, b.y);
}

static int v3_close(vec3 a, vec3 b) {
    return close_to(a.x, b.x) && close_to(a.y, b.y) && close_to(a.z, b.z);
}

static int v4_close(vec4 a, vec4 b) {
    return close_to(a.x, b.x) && close_to(a.y, b.y) &&
           close_to(a.z, b.z) && close_to(a.w, b.w);
}

int main(void) {
    abs_init();

    /* --- Type aliases --- */
    CHECK(sizeof(i8) == 1 && sizeof(u16) == 2 && sizeof(f32) == 4 && sizeof(f64) == 8);
    CHECK(sizeof(byte) == 1 && sizeof(b8) == 1 && sizeof(b32) == 4);
    CHECK(sizeof(i64) == 8 && sizeof(u64) == 8);
    CHECK(sizeof(isize) == sizeof(ptrdiff_t) && sizeof(usize) == sizeof(size_t));

    /* --- Sizes --- */
    CHECK(sizeof(vec2) == 2 * sizeof(f32));
    CHECK(sizeof(vec3) == 3 * sizeof(f32));
    CHECK(sizeof(vec4) == 4 * sizeof(f32));
    CHECK(sizeof(quat) == 4 * sizeof(f32));
    CHECK(sizeof(mat2) == 4 * sizeof(f32));
    CHECK(sizeof(mat3) == 9 * sizeof(f32));
    CHECK(sizeof(mat4) == 16 * sizeof(f32));

    /* --- Anonymous-union component aliasing --- */
    {
        vec2 p = v2(1.0f, 2.0f);
        CHECK(p.x == 1.0f && p.y == 2.0f);
        CHECK(p.u == 1.0f && p.v == 2.0f);
        CHECK(p.raw[0] == 1.0f && p.raw[1] == 2.0f);

        vec3 c = v3(0.2f, 0.4f, 0.6f);
        CHECK(c.r == 0.2f && c.g == 0.4f && c.b == 0.6f);
        CHECK(c.raw[2] == 0.6f);

        vec4 rgba = v4(1.0f, 0.5f, 0.25f, 0.1f);
        CHECK(rgba.r == 1.0f && rgba.g == 0.5f && rgba.b == 0.25f && rgba.a == 0.1f);
        CHECK(rgba.raw[3] == 0.1f);

        ivec2 ip = iv2(3, 4);
        CHECK(ip.x == 3 && ip.y == 4 && ip.raw[0] == 3);
        ivec3 ip3 = iv3(1, 2, 3);
        CHECK(ip3.z == 3);
        ivec4 ip4 = iv4(1, 2, 3, 4);
        CHECK(ip4.w == 4);

        vec2d d = { .x = 1.5, .y = 2.5 };
        CHECK(d.x == 1.5 && d.raw[1] == 2.5);
        vec3d d3 = { .x = 1, .y = 2, .z = 3 };
        CHECK(d3.z == 3);
        vec4d d4 = { .x = 1, .y = 2, .z = 3, .w = 4 };
        CHECK(d4.w == 4);
    }

    /* --- vec2 --- */
    {
        CHECK(v2_close(abs_v2_add(v2(1, 2), v2(3, 4)), v2(4, 6)));
        CHECK(v2_close(abs_v2_sub(v2(5, 7), v2(1, 2)), v2(4, 5)));
        CHECK(v2_close(abs_v2_scale(v2(3, 4), 2), v2(6, 8)));
        CHECK(close_to(abs_v2_dot(v2(1, 2), v2(3, 4)), 11));
        CHECK(close_to(abs_v2_cross(v2(1, 2), v2(3, 4)), -2));
        CHECK(close_to(abs_v2_cross(v2(3, 4), v2(1, 2)), 2));
        CHECK(close_to(abs_v2_len(v2(3, 4)), 5));
        CHECK(close_to(abs_v2_dist(v2(0, 0), v2(3, 4)), 5));
        CHECK(v2_close(abs_v2_norm(v2(3, 4)), v2(0.6f, 0.8f)));
        CHECK(v2_close(abs_v2_lerp(v2(0, 0), v2(10, 10), 0.5f), v2(5, 5)));
        CHECK(v2_close(abs_v2_reflect(v2(1, -1), v2(0, 1)), v2(1, 1)));
        CHECK(v2_close(abs_v2_reflect(v2(5, -3), v2(0, 1)), v2(5, 3)));
        CHECK(v2_close(abs_v2_norm(v2(0, 0)), v2(0, 0)));
    }

    /* --- vec3 --- */
    {
        CHECK(v3_close(abs_v3_add(v3(1, 2, 3), v3(4, 5, 6)), v3(5, 7, 9)));
        CHECK(v3_close(abs_v3_sub(v3(5, 7, 9), v3(1, 2, 3)), v3(4, 5, 6)));
        CHECK(v3_close(abs_v3_scale(v3(1, 2, 3), 2), v3(2, 4, 6)));
        CHECK(close_to(abs_v3_dot(v3(1, 2, 3), v3(4, 5, 6)), 32));
        CHECK(v3_close(abs_v3_cross(v3(1, 0, 0), v3(0, 1, 0)), v3(0, 0, 1)));
        CHECK(v3_close(abs_v3_cross(v3(0, 1, 0), v3(1, 0, 0)), v3(0, 0, -1)));
        CHECK(close_to(abs_v3_len(v3(1, 2, 2)), 3));
        CHECK(close_to(abs_v3_dist(v3(0, 0, 0), v3(1, 2, 2)), 3));
        CHECK(v3_close(abs_v3_norm(v3(1, 2, 2)), v3(1.0f / 3, 2.0f / 3, 2.0f / 3)));
        CHECK(v3_close(abs_v3_lerp(v3(0, 0, 0), v3(3, 6, 9), 1.0f / 3), v3(1, 2, 3)));
        CHECK(v3_close(abs_v3_reflect(v3(1, -1, 0), v3(0, 1, 0)), v3(1, 1, 0)));
        CHECK(v3_close(abs_v3_norm(v3(0, 0, 0)), v3(0, 0, 0)));
    }

    /* --- vec4 --- */
    {
        CHECK(v4_close(abs_v4_add(v4(1, 2, 3, 4), v4(1, 1, 1, 1)), v4(2, 3, 4, 5)));
        CHECK(v4_close(abs_v4_sub(v4(5, 6, 7, 8), v4(1, 2, 3, 4)), v4(4, 4, 4, 4)));
        CHECK(v4_close(abs_v4_scale(v4(1, 2, 3, 4), 2), v4(2, 4, 6, 8)));
        CHECK(close_to(abs_v4_dot(v4(1, 2, 3, 4), v4(1, 1, 1, 1)), 10));
        CHECK(close_to(abs_v4_len(v4(1, 2, 2, 4)), 5));
        CHECK(close_to(abs_v4_dist(v4(0, 0, 0, 0), v4(1, 2, 2, 4)), 5));
        CHECK(v4_close(abs_v4_norm(v4(1, 2, 2, 4)), v4(0.2f, 0.4f, 0.4f, 0.8f)));
        CHECK(v4_close(abs_v4_lerp(v4(0, 0, 0, 0), v4(4, 8, 12, 16), 0.25f), v4(1, 2, 3, 4)));
        CHECK(v4_close(abs_v4_norm(v4(0, 0, 0, 0)), v4(0, 0, 0, 0)));
    }

    /* --- Quaternions --- */
    {
        quat id = abs_quat_ident();
        CHECK(id.x == 0 && id.y == 0 && id.z == 0 && id.w == 1);

        quat q = abs_quat_from_axis_angle(0, 0, 1, (f32)ABS_PI / 2);
        CHECK(close_to(q.z, sin(ABS_PI / 4)) && close_to(q.w, cos(ABS_PI / 4)));
        CHECK(v3_close(abs_quat_rotate_vec3(q, v3(1, 0, 0)), v3(0, 1, 0)));

        quat q2 = abs_quat_mul(q, q);
        CHECK(close_to(q2.w, 0) && close_to(q2.z, 1));
        CHECK(v3_close(abs_quat_rotate_vec3(q2, v3(1, 0, 0)), v3(-1, 0, 0)));

        CHECK(v3_close(abs_quat_rotate_vec3(id, v3(1, 2, 3)), v3(1, 2, 3)));

        vec3 p = v3(1, 2, 3);
        quat qa = abs_quat_norm(q4(0.1f, -0.2f, 0.3f, 0.9f));
        CHECK(close_to(abs_v3_len(abs_quat_rotate_vec3(qa, p)), abs_v3_len(p)));

        quat u = abs_quat_norm(q4(2, 0, 0, 2));
        CHECK(close_to(u.x, 1 / sqrt(2.0)) && close_to(u.w, 1 / sqrt(2.0)));

        quat z = abs_quat_norm(q4(0, 0, 0, 0));
        CHECK(z.w == 1);
        quat zz = abs_quat_from_axis_angle(0, 0, 0, 1);
        CHECK(zz.w == 1);
    }

    /* --- mat4 --- */
    {
        mat4 I = abs_mat4_identity();
        CHECK(I.m[0][0] == 1 && I.m[1][1] == 1 && I.m[2][2] == 1 && I.m[3][3] == 1);
        CHECK(I.m[0][1] == 0 && I.m[2][0] == 0 && I.m[1][3] == 0);
        CHECK(v4_close(abs_mat4_mul_vec4(I, v4(1, 2, 3, 4)), v4(1, 2, 3, 4)));

        mat4 M = abs_mat4_mul(I, abs_mat4_identity());
        CHECK(M.m[0][0] == 1 && M.m[3][3] == 1 && M.m[0][1] == 0);

        CHECK(v4_close(abs_mat4_mul_vec4(abs_mat4_translate(1, 2, 3), v4(0, 0, 0, 1)),
                       v4(1, 2, 3, 1)));
        CHECK(v4_close(abs_mat4_mul_vec4(abs_mat4_translate(10, 20, 30), v4(1, 1, 1, 1)),
                       v4(11, 21, 31, 1)));

        CHECK(v4_close(abs_mat4_mul_vec4(abs_mat4_scale(2, 3, 4), v4(1, 1, 1, 1)),
                       v4(2, 3, 4, 1)));

        CHECK(v4_close(abs_mat4_mul_vec4(abs_mat4_rotate_z((f32)ABS_PI / 2), v4(1, 0, 0, 1)),
                       v4(0, 1, 0, 1)));
        CHECK(v4_close(abs_mat4_mul_vec4(abs_mat4_rotate_z((f32)ABS_PI / 2), v4(0, 1, 0, 1)),
                       v4(-1, 0, 0, 1)));
        CHECK(v4_close(abs_mat4_mul_vec4(abs_mat4_rotate_x((f32)ABS_PI / 2), v4(0, 1, 0, 1)),
                       v4(0, 0, 1, 1)));
        CHECK(v4_close(abs_mat4_mul_vec4(abs_mat4_rotate_y((f32)ABS_PI / 2), v4(0, 0, 1, 1)),
                       v4(1, 0, 0, 1)));

        mat4 model = abs_mat4_mul(abs_mat4_translate(10, 0, 0), abs_mat4_scale(2, 2, 2));
        CHECK(v4_close(abs_mat4_mul_vec4(model, v4(1, 0, 0, 1)), v4(12, 0, 0, 1)));

        mat4 P = abs_mat4_perspective((f32)ABS_PI / 2, 1.0f, 0.1f, 100.0f);
        CHECK(close_to(P.m[0][0], 1.0));
        CHECK(close_to(P.m[1][1], 1.0));
        CHECK(P.m[2][3] == -1.0f && P.m[3][3] == 0.0f);
        CHECK(close_to(abs_mat4_mul_vec4(P, v4(0, 0, -0.1f, 1)).w, 0.1));

        mat4 V = abs_mat4_look_at(v3(0, 0, 5), v3(0, 0, 0), v3(0, 1, 0));
        CHECK(v4_close(abs_mat4_mul_vec4(V, v4(0, 0, 0, 1)), v4(0, 0, -5, 1)));
        CHECK(v4_close(abs_mat4_mul_vec4(V, v4(0, 0, 5, 1)), v4(0, 0, 0, 1)));
        CHECK(v4_close(abs_mat4_mul_vec4(V, v4(1, 0, 0, 1)), v4(1, 0, -5, 1)));

        abs_v2_print(v2(1, 2), "v2");
        abs_v3_print(v3(1, 2, 3), "v3");
        abs_v4_print(v4(1, 2, 3, 4), "v4");
        abs_quat_print(abs_quat_ident(), "quat");
        abs_mat4_print(abs_mat4_identity(), "mat4");
    }

    abs_cleanup();
    printf("test_geom: OK\n");
    return 0;
}
