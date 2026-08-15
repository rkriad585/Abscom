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
    return fabs(a - b) < 1e-12;
}

typedef struct {
    u32 id;
    f32 health;
    vec3 position;
} AbsTestEntity;

int main(void) {
    abs_init();

    /* --- Basic arithmetic and comparison --- */
    CHECK(ABS_MIN(3, 5) == 3);
    CHECK(ABS_MAX(3, 5) == 5);
    CHECK(ABS_MIN(-1.5, 2.5) == -1.5);
    CHECK(ABS_MIN3(3, 5, 1) == 1);
    CHECK(ABS_MAX3(3, 5, 1) == 5);
    CHECK(ABS_MIN4(4, 2, 9, 3) == 2);
    CHECK(ABS_MAX4(4, 2, 9, 3) == 9);
    CHECK(ABS_MIN4(1, 1, 1, 1) == 1);
    CHECK(ABS_ABS(-7) == 7);
    CHECK(ABS_ABS(7) == 7);
    CHECK(ABS_SIGN(-3.2) == -1);
    CHECK(ABS_SIGN(0.0) == 0);
    CHECK(ABS_SIGN(4.9) == 1);
    CHECK(ABS_CLAMP(10, 0, 5) == 5);
    CHECK(ABS_CLAMP(-10, 0, 5) == 0);
    CHECK(ABS_CLAMP(3, 0, 5) == 3);
    CHECK(ABS_CLAMP01(1.5) == 1.0);
    CHECK(ABS_CLAMP01(-0.5) == 0.0);
    CHECK(ABS_CLAMP01(0.25) == 0.25);
    CHECK(ABS_IN_RANGE(3, 1, 5));
    CHECK(!ABS_IN_RANGE(6, 1, 5));
    CHECK(ABS_IN_RANGE(1, 1, 5));
    CHECK(ABS_IN_RANGE(5, 1, 5));

    /* --- Powers, differences, approximate equality --- */
    CHECK(ABS_SQR(6) == 36);
    CHECK(ABS_SQR(-6) == 36);
    CHECK(ABS_CUBE(3) == 27);
    CHECK(ABS_CUBE(-2) == -8);
    CHECK(ABS_DIFF(10, 4) == 6);
    CHECK(ABS_DIFF(4, 10) == 6);
    CHECK(ABS_APPROX_EQ(1.0, 1.0 + 1e-12, 1e-9));
    CHECK(!ABS_APPROX_EQ(1.0, 1.1, 1e-9));

    /* --- Interpolation and shading math --- */
    CHECK(ABS_LERP(0.0, 10.0, 0.5) == 5.0);
    CHECK(ABS_LERP(0.0, 10.0, 0.0) == 0.0);
    CHECK(ABS_LERP(0.0, 10.0, 1.0) == 10.0);
    CHECK(ABS_UNLERP(0.0, 10.0, 5.0) == 0.5);
    CHECK(ABS_REMAP(2.5, 0.0, 5.0, -40.0, 125.0) == 42.5);
    CHECK(ABS_REMAP(0.0, 0.0, 5.0, -40.0, 125.0) == -40.0);
    CHECK(ABS_REMAP(5.0, 0.0, 5.0, -40.0, 125.0) == 125.0);
    CHECK(ABS_STEP(5.0, 4.9) == 0.0);
    CHECK(ABS_STEP(5.0, 5.0) == 1.0);
    CHECK(close_to(ABS_SMOOTHSTEP(0.0, 1.0, 0.75), 0.84375));
    CHECK(close_to(ABS_SMOOTHSTEP(0.0, 1.0, 0.0), 0.0));
    CHECK(close_to(ABS_SMOOTHSTEP(0.0, 1.0, 1.0), 1.0));
    CHECK(close_to(ABS_SMOOTHSTEP(0.0, 1.0, -1.0), 0.0));
    CHECK(close_to(ABS_SMOOTHSTEP(0.0, 1.0, 2.0), 1.0));
    CHECK(close_to(ABS_SMOOTHSTEP(0.0, 2.0, 1.0), 0.5));

    /* --- Angle conversions --- */
    CHECK(close_to(ABS_DEG2RAD_M(180.0), ABS_PI));
    CHECK(close_to(ABS_DEG2RAD_M(0.0), 0.0));
    CHECK(close_to(ABS_RAD2DEG_M(ABS_PI), 180.0));
    CHECK(close_to(ABS_RAD2DEG_M(0.0), 0.0));

    /* --- Array length --- */
    {
        int ints[7] = {0};
        double dbls[3] = {0};
        char c[1] = {0};
        CHECK(ABS_ARRAY_LEN(ints) == 7);
        CHECK(ABS_ARRAY_LEN(dbls) == 3);
        CHECK(ABS_ARRAY_LEN(c) == 1);
    }

    /* --- Struct offset and container_of --- */
    CHECK(ABS_OFFSETOF(AbsTestEntity, id) == 0);
    CHECK(ABS_OFFSETOF(AbsTestEntity, health) == 4);
    CHECK(ABS_OFFSETOF(AbsTestEntity, position) == 8);
    {
        AbsTestEntity e = { .id = 101, .health = 100.0f, .position = v3(0, 5, -12) };
        vec3 *pos = &e.position;
        AbsTestEntity *back = ABS_CONTAINER_OF(pos, AbsTestEntity, position);
        CHECK(back == &e);
        CHECK(back->id == 101);
        CHECK(back->health == 100.0f);
        CHECK(back->position.z == -12.0f);
    }

    /* --- Generic swap --- */
    {
        int x = 1, y = 2;
        ABS_SWAP(x, y);
        CHECK(x == 2 && y == 1);
        double f = 1.5, g = 2.5;
        ABS_SWAP(f, g);
        CHECK(f == 2.5 && g == 1.5);
        vec2 p = v2(1, 2), q = v2(3, 4);
        ABS_SWAP(p, q);
        CHECK(p.x == 3 && p.y == 4 && q.x == 1 && q.y == 2);
    }

    /* --- Bitwise operations --- */
    CHECK(ABS_BIT(0) == 1);
    CHECK(ABS_BIT(3) == 8);
    CHECK(ABS_BIT(5) == 32);
    {
        u32 flags = 0;
        ABS_BIT_SET(flags, 3);
        ABS_BIT_SET(flags, 5);
        CHECK(flags == 40);
        CHECK(ABS_BIT_CHECK(flags, 3));
        CHECK(ABS_BIT_CHECK(flags, 5));
        CHECK(!ABS_BIT_CHECK(flags, 4));
        ABS_BIT_CLEAR(flags, 3);
        CHECK(flags == 32);
        ABS_BIT_TOGGLE(flags, 5);
        CHECK(flags == 0);
        ABS_BIT_TOGGLE(flags, 0);
        CHECK(flags == 1);
    }

    /* --- Power-of-two and alignment --- */
    CHECK(ABS_IS_POW2(1));
    CHECK(ABS_IS_POW2(2));
    CHECK(ABS_IS_POW2(512));
    CHECK(!ABS_IS_POW2(0));
    CHECK(!ABS_IS_POW2(3));
    CHECK(!ABS_IS_POW2(1040));
    CHECK(ABS_ALIGN_UP(1040, 512) == 1536);
    CHECK(ABS_ALIGN_UP(512, 512) == 512);
    CHECK(ABS_ALIGN_UP(513, 512) == 1024);
    CHECK(ABS_ALIGN_UP(0, 512) == 0);
    CHECK(ABS_ALIGN_DOWN(1040, 512) == 1024);
    CHECK(ABS_ALIGN_DOWN(512, 512) == 512);
    CHECK(ABS_ALIGN_DOWN(511, 512) == 0);

    /* --- AI activation helpers --- */
    CHECK(ABS_RELU_M(2.5) == 2.5);
    CHECK(ABS_RELU_M(-2.5) == 0.0);
    CHECK(ABS_LEAKY_RELU_M(-4.5, 0.01) == -0.045);
    CHECK(ABS_LEAKY_RELU_M(3.0, 0.01) == 3.0);
    CHECK(ABS_LEAKY_RELU_M(-4.5, 0.1) == -0.45);
    CHECK(ABS_HEAVISIDE_M(-0.5) == 0.0);
    CHECK(ABS_HEAVISIDE_M(0.0) == 1.0);
    CHECK(ABS_HEAVISIDE_M(0.5) == 1.0);

    /* --- Unprefixed aliases --- */
    CHECK(MIN(3, 5) == 3);
    CHECK(MAX(3, 5) == 5);
    CHECK(CLAMP(10, 0, 5) == 5);
    CHECK(CLAMP01(1.5) == 1.0);
    CHECK(LERP(0.0, 10.0, 0.5) == 5.0);
    CHECK(REMAP(2.5, 0.0, 5.0, -40.0, 125.0) == 42.5);
    CHECK(SIGN(-3.2) == -1);
    CHECK(SQR(6) == 36);
    {
        int ints[4] = {0};
        CHECK(ARRAY_LEN(ints) == 4);
    }
    {
        int x = 1, y = 2;
        SWAP(x, y);
        CHECK(x == 2 && y == 1);
    }
    CHECK(close_to(DEG2RAD(180.0), ABS_DEG2RAD_M(180.0)));
    CHECK(close_to(RAD2DEG(ABS_PI), ABS_RAD2DEG_M(ABS_PI)));
    CHECK(BIT(4) == 16);
    CHECK(IS_POW2(8));
    CHECK(!IS_POW2(12));

    abs_cleanup();
    printf("test_macros: OK\n");
    return 0;
}
