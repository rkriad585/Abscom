#include "abscom/abs.h"

typedef struct {
    u32 id;
    f32 health;
    vec3 position;
} Entity;

int main(void) {
    abs_init();

    printf("=== ABSCOM MACRO UTILITIES DEMO ===\n\n");

    /* 1. Min / Max / Clamp / Multi-arg Comparisons */
    printf("--- 1. Min, Max, and Multi-Arg Comparisons ---\n");
    f32 sensor_a = 12.5f, sensor_b = 48.2f, sensor_c = -3.4f, sensor_d = 27.9f;
    printf("MIN(%.1f, %.1f)             = %.1f\n", sensor_a, sensor_b, MIN(sensor_a, sensor_b));
    printf("MAX(%.1f, %.1f)             = %.1f\n", sensor_a, sensor_b, MAX(sensor_a, sensor_b));
    printf("ABS_MIN4(a, b, c, d)        = %.1f\n", ABS_MIN4(sensor_a, sensor_b, sensor_c, sensor_d));
    printf("ABS_MAX4(a, b, c, d)        = %.1f\n", ABS_MAX4(sensor_a, sensor_b, sensor_c, sensor_d));

    f32 raw_voltage = 14.8f;
    f32 clamped_voltage = CLAMP(raw_voltage, 0.0f, 12.0f);
    printf("CLAMP(14.8, 0.0, 12.0)      = %.1f\n\n", clamped_voltage);

    /* 2. Interpolation & Curves (Graphics / AI / Physics) */
    printf("--- 2. LERP, UNLERP, REMAP, and Smoothstep ---\n");
    f32 start_pos = 100.0f, end_pos = 200.0f;
    f32 halfway = LERP(start_pos, end_pos, 0.5f);
    printf("LERP(100, 200, t=0.5)       = %.1f\n", halfway);

    /* Remap a temperature sensor: [0.0 - 5.0] Volts to [-40.0 - 125.0] Celsius */
    f32 input_volts = 2.5f;
    f32 temp_celsius = REMAP(input_volts, 0.0f, 5.0f, -40.0f, 125.0f);
    printf("REMAP(2.5V [0..5V] to [-40..125C]) = %.2f C\n", temp_celsius);

    f32 smooth_val = ABS_SMOOTHSTEP(0.0, 1.0, 0.75);
    printf("SMOOTHSTEP(0.0, 1.0, x=0.75) = %.4f\n\n", smooth_val);

    /* 3. Array Count & Type-Safe Swap */
    printf("--- 3. Static Array Length & Generic Swap ---\n");
    f64 dataset[] = { 1.2, 4.5, -9.1, 14.3, 0.0, 8.8, -3.3 };
    printf("ARRAY_LEN(dataset)          = %zu elements\n", ARRAY_LEN(dataset));

    vec2 point_p = v2(10.0f, 20.0f);
    vec2 point_q = v2(99.0f, 88.0f);
    printf("Before SWAP: P=(%.1f, %.1f), Q=(%.1f, %.1f)\n", point_p.x, point_p.y, point_q.x, point_q.y);
    SWAP(point_p, point_q);
    printf("After  SWAP: P=(%.1f, %.1f), Q=(%.1f, %.1f)\n\n", point_p.x, point_p.y, point_q.x, point_q.y);

    /* 4. Bitwise Operations & Power-of-2 Memory Alignment */
    printf("--- 4. Bitwise & Memory Alignment Helpers ---\n");
    u32 flags = 0;
    ABS_BIT_SET(flags, 3);
    ABS_BIT_SET(flags, 5);
    printf("Flags after setting bit 3 & 5: %u (Bit 3 set? %s)\n",
           flags, ABS_BIT_CHECK(flags, 3) ? "YES" : "NO");

    u32 buffer_size = 1040;
    u32 aligned_512 = ABS_ALIGN_UP(buffer_size, 512);
    printf("ALIGN_UP(1040, 512)         = %u bytes\n", aligned_512);
    printf("IS_POW2(512)?               = %s\n", IS_POW2(512) ? "TRUE" : "FALSE");
    printf("IS_POW2(1040)?              = %s\n\n", IS_POW2(1040) ? "TRUE" : "FALSE");

    /* 5. Structure Introspection (Container Of) */
    printf("--- 5. Struct Member Offset & Container Pointer ---\n");
    printf("Offset of 'position' in Entity: %zu bytes\n", ABS_OFFSETOF(Entity, position));

    Entity player = { .id = 101, .health = 100.0f, .position = v3(0.0f, 5.0f, -12.0f) };
    vec3 *pos_ptr = &player.position;
    Entity *recovered_player = ABS_CONTAINER_OF(pos_ptr, Entity, position);
    printf("Recovered Entity ID via pos_ptr: %u (Health: %.1f)\n\n",
           recovered_player->id, recovered_player->health);

    /* 6. Neural Network Activation Macros */
    printf("--- 6. Machine Learning Activation Macros ---\n");
    f32 neuron_input = -4.5f;
    printf("Leaky ReLU(-4.5, alpha=0.01) = %.4f\n", ABS_LEAKY_RELU_M(neuron_input, 0.01f));
    printf("Standard ReLU(-4.5)          = %.4f\n", ABS_RELU_M(neuron_input));
    printf("Heaviside Step(-4.5)         = %.1f\n", ABS_HEAVISIDE_M(neuron_input));

    abs_cleanup();
    return 0;
}
