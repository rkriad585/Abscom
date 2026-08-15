#include "abscom/abs.h"

int main(void) {
    abs_init();

    printf("=== Modern type aliases ===\n");
    i32 score = 100;
    u64 big = 1ULL << 40;
    f32 pi_f = (f32)ABS_PI;
    f64 pi = ABS_PI;
    byte raw = 0xAB;
    b8 flag = true;
    printf("score=%d  big=%llu  pi_f=%.4f  pi=%.6f  raw=0x%02X  flag=%d\n",
           (int)score, (unsigned long long)big, (double)pi_f, pi, (unsigned)raw,
           (int)flag);

    printf("\n=== vec2: 2D ground bounce ===\n");
    vec2 vel = v2(5.0f, -3.0f);
    vec2 normal = v2(0.0f, 1.0f);
    vec2 bounce = abs_v2_reflect(vel, normal);
    abs_v2_print(vel, "velocity");
    abs_v2_print(normal, "normal  ");
    abs_v2_print(bounce, "bounced ");
    printf("dot=%f  len=%f  dist=%f\n", (double)abs_v2_dot(vel, normal),
           (double)abs_v2_len(vel), (double)abs_v2_dist(vel, bounce));
    abs_v2_print(abs_v2_lerp(v2(0.0f, 0.0f), v2(10.0f, 10.0f), 0.5f), "lerp mid");

    printf("\n=== vec3: cross product normal ===\n");
    vec3 a = v3(1.0f, 0.0f, 0.0f);
    vec3 b = v3(0.0f, 1.0f, 0.0f);
    vec3 n = abs_v3_norm(abs_v3_cross(a, b));
    abs_v3_print(a, "a       ");
    abs_v3_print(b, "b       ");
    abs_v3_print(n, "a x b   ");
    printf("rgb aliases: r=%f g=%f b=%f\n", (double)n.r, (double)n.g, (double)n.b);

    printf("\n=== quaternion rotation ===\n");
    quat q = abs_quat_from_axis_angle(0.0f, 0.0f, 1.0f, (f32)ABS_PI / 2.0f);
    vec3 p = v3(1.0f, 0.0f, 0.0f);
    abs_quat_print(q, "q(90,Z) ");
    abs_v3_print(p, "point   ");
    abs_v3_print(abs_quat_rotate_vec3(q, p), "rotated ");
    abs_v3_print(abs_quat_rotate_vec3(abs_quat_mul(q, q), p), "twice   ");

    printf("\n=== mat4: model matrix ===\n");
    mat4 model = abs_mat4_translate(10.0f, 0.0f, 5.0f);
    model = abs_mat4_mul(model, abs_mat4_scale(2.0f, 2.0f, 2.0f));
    model = abs_mat4_mul(model, abs_mat4_rotate_y((f32)ABS_PI / 4.0f));
    abs_mat4_print(model, "model");
    vec4 local = v4(1.0f, 1.0f, 1.0f, 1.0f);
    vec4 world = abs_mat4_mul_vec4(model, local);
    abs_v4_print(local, "local ");
    abs_v4_print(world, "world ");

    printf("\n=== mat4: camera ===\n");
    mat4 view = abs_mat4_look_at(v3(0.0f, 0.0f, 5.0f), v3(0.0f, 0.0f, 0.0f),
                                 v3(0.0f, 1.0f, 0.0f));
    mat4 proj = abs_mat4_perspective((f32)ABS_PI / 3.0f, 16.0f / 9.0f, 0.1f, 100.0f);
    vec4 clip = abs_mat4_mul_vec4(proj, abs_mat4_mul_vec4(view, world));
    abs_v4_print(clip, "clip   ");

    abs_cleanup();
    return 0;
}
