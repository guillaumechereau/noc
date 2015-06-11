#include "noc_turtle.h"
#define NOC_TURTLE_DEFINE_NAMES
#include "noc_turtle.h"

#include <math.h>
#include <stdio.h>

void font_draw_text(float x, float y, const char *str);
// Defines a custom directive to render text.
#define TEXT(msg, ...) TRANSFORM(__VA_ARGS__) { \
    noctt_vec3_t pos = noctt_get_pos(ctx); \
    font_draw_text(pos.x, pos.y, msg); \
}

enum {
    FLAG_STENCIL_WRITE  = 1 << 0,
    FLAG_STENCIL_FILTER = 1 << 1,
    FLAG_EFFECT_LIGHT   = 1 << 2,
};

static const float SPEED = 0.1;

static void square(noctt_turtle_t *ctx)
{
    float k;
    const float time = SPEED * sqrtf(ctx->scale[1]);
    const int nb = time * 50;
    START
    FOR(nb) {
        k = ctx->i / (ctx->n - 1.0);
        SQUARE(Y, -0.5, S, 1, k, Y, 0.5);
        YIELD(1);
    }
    SQUARE();
    END
}

static void cloud(noctt_turtle_t *ctx)
{
    START
    TR(A, -0.5, LIGHT, 1);
    TR(SY, 1.0 / 3, Y, -1, X, -0.5, SN, X, 0.5);
    TR(LIGHT, 1);
    FOR(4, X, 2.0 / 3) {
        CIRCLE(S, FRAND(0.75, 1));
    }
    TR(Y, 0.5, X, 1.0 / 3);
    FOR(3, X, 2.0 / 3) {
        CIRCLE(S, FRAND(0.75, 1));
    }
    TR(Y, 0.25, X, 2.0 / 3);
    FOR(1, X, 2.0 / 3) {
        CIRCLE(S, FRAND(0.75, 1));
    }
    END
}

static void sky_line(noctt_turtle_t *ctx)
{
    int i;
    const float sx = 0.02;
    const float dx = 0.9;
    START
    TR(X, -0.5, S, sx, 0.2, LIGHT, 0.2);
    for (i = 0; i < 3; i++) ctx->vars[i] = PM(0, 0.5);
    FOR(1.1 / sx / dx, X, dx, R, PM(0, 0.1), SY, PM(1, 0.01))
    {
        SQUARE();
        for (i = 0; i < 3; i++)
            SQUARE(Y, ctx->vars[i], S, 1.2, 0.02, LIGHT, -0.05);
    }
    END
}

static void noise(noctt_turtle_t *ctx)
{
    START
    TR(FLAG, FLAG_EFFECT_LIGHT, SAT, -1, LIGHT, 1, 0.5);
    FOR(1000) {
        TR(X, PM(0, 0.5), PM(0, 0.5));
        TR(S, PM(0.02, 0.02), SN, R, FRAND(0, 360));
        SQUARE(LIGHT, PM(0, 0.04));
    }
    END
}

static void sky(noctt_turtle_t *ctx)
{
    START
    SQUARE();
    TR(SN);
    TR(Y, -0.3);
    if (BRAND(0.5)) {
        FOR(3, Y, 0.3) {
            CALL(sky_line);
        }
    }
    END
}

static void antenna(noctt_turtle_t *ctx)
{
    START
    CALL(square, S, 0.02, FRAND(1.5, 2) / 6, Y, 0.5);
    END
}

static void tower(noctt_turtle_t *ctx)
{
    const int n = 4;
    START
    CALL(square);

    // Sides
    if (BRAND(0.5)) {
        FOR(n) {
            SPAWN(square, Y, (float)ctx->i / n - 0.4, S, 1.1, 0.1);
        }
    }

    if (BRAND(0.5)) SPAWN(antenna, X, PM(0, 0.5), 0.5, S, 3, 0.5);

    // Top
    TRANSFORM(Y, 0.5, S, 0.9, 0.02, Y, 0.5) {
        ctx->vars[0] = FRAND(0, 3);
        FOR(ctx->vars[0], Y, 1, S, 0.9, 1)
            CALL(square);
    }

    END
}

static void building(noctt_turtle_t *ctx)
{
    START
    CALL(square);

    // Top
    TRANSFORM(Y, 0.5, S, 0.9, 0.05, Y, 0.5) {
        FOR(FRAND(0, 3), Y, 1, S, 0.9, 1) {
            CALL(square);
        }
    }

    if (BRAND(0.5)) SPAWN(antenna, X, PM(0, 0.5), 0.5);

    // Chimneys
    if (BRAND(0.5)) {
        TRANSFORM_SPAWN(X, 0, 0.5, S, 0.1, 0.5, Y, 0.5) {
            FOR(3, X, 1.5, -0.2) {
                CALL(square);
                CALL(square, Y, 0.4, S, 1.2, 0.2, Y, 0.5);
            }
        }
    }
    END
}

static void structure(noctt_turtle_t *ctx)
{
    START
    if (BRAND(0.5))
        CALL(tower, S, FRAND(1, 3), FRAND(5, 10), Y, 0.5);
    else
        CALL(building, S, FRAND(4, 10), FRAND(2, 4), Y, 0.5);
    END
}

void blowfish_city_rule(noctt_turtle_t *ctx)
{
    START
    TR(HSL, 1, 0, 0.3, 0.5);
    CALL(sky, Z, -0.5);
    CALL(noise);
    TR(SN);
    TR(HSL, 1, 180, 0.1, 0.1);
    SQUARE(X, -1);
    SQUARE(X, +1);
    TEXT("Background from the game Blowish Rescue", X, -0.48, Y, 0.47);

    SPAWN(cloud, X, 0.25, 0.25, S, 0.1, SN);

    TR(Y, -0.05);
    SQUARE(Y, -0.7, S, 1, 0.4);
    TR(Y, -0.5);
    FOR(20) {
        SPAWN(structure, X, FRAND(-0.45, 0.45), S, 1.0 / 30);
        YIELD(1);
    }
    END
}
