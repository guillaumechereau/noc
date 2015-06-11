#include "noc_turtle.h"
#define NOC_TURTLE_DEFINE_NAMES
#include "noc_turtle.h"

#include <math.h>
#include <stdio.h>

enum {
    FLAG_STENCIL_WRITE  = 1 << 0,
    FLAG_STENCIL_FILTER = 1 << 1,
    FLAG_EFFECT_LIGHT   = 1 << 2,
};

void font_draw_text(float x, float y, const char *str);
// Defines a custom directive to render text.
#define TEXT(msg, ...) TRANSFORM(__VA_ARGS__) { \
    noctt_vec3_t pos = noctt_get_pos(ctx); \
    font_draw_text(pos.x, pos.y, msg); \
}

static void noise1(noctt_turtle_t *ctx)
{
    START
    TR(SAT, -1, LIGHT, 1, 0.5, FLAG, FLAG_EFFECT_LIGHT);
    FOR(100) {
        SQUARE(X, PM(0, 0.5), PM(0, 0.5),
               SN, S, 0.2, S, PM(1, 1),
               R, FRAND(0, 360),
               LIGHT, PM(0, ctx->vars[0]));
    }
    END
}

static void block(noctt_turtle_t *ctx)
{
    START
    TR(HSL, 1, PM(200, 25), 0.5, 0.5);

    RSQUARE(64);
    RSQUARE(64, G, -0.5, FLAG, FLAG_STENCIL_WRITE);
    TR(FLAG, FLAG_STENCIL_FILTER);
    FOR(2, R, 90) {
        SQUARE(R, 45, S, 1.5, 0.2, LIGHT, 0.2);
    }
    CALL(noise1, VAR, 0, 0.05);
    END
}

static void saw(noctt_turtle_t *ctx)
{
    START
    TR(HSL, 1, 0, 0, 0.5);
    STAR(8, 0.2, -0.9, LIGHT, -0.25);
    STAR(8, 0.15, -0.9, R, -4, G, -8, FLAG, FLAG_STENCIL_WRITE);
    CALL(noise1, FLAG, FLAG_EFFECT_LIGHT, 1, FLAG_STENCIL_FILTER, 1,
                 VAR, 0, 0.1);
    CIRCLE(S, 0.3, LIGHT, -1);
    END
}

static void bomb(noctt_turtle_t *ctx)
{
    const int n = 8;
    START
    TR(HSL, 1, 0, 0.5, 0.5);
    TR(S, 0.8);
    CIRCLE();
    CIRCLE(G, -1, FLAG, FLAG_STENCIL_WRITE);

    TRANSFORM(FLAG, FLAG_STENCIL_FILTER) {
        CALL(noise1, FLAG, FLAG_EFFECT_LIGHT, VAR, 0, 0.1);
    }

    TRANSFORM(LIGHT, 1) {
        FOR(n, R, 360 / n) {
            TR(X, 0.5, S, 0.2);
            TRIANGLE(LIGHT, -0.3);
            TRIANGLE(G, -1);
        }
        FOR(6, R, 360 / 6) {
            TR(R, 180 / 6, X, 0.3, S, 0.2);
            TRIANGLE(LIGHT, -0.3);
            TRIANGLE(G, -1);
        }
        TRANSFORM(S, 0.15) {
            CIRCLE(LIGHT, -0.3);
            CIRCLE(G, -0.75);
        }
    }
    END
}

static void cannon(noctt_turtle_t *ctx)
{
    START
    TR(HSL, 1, 90, 0, 0.5);

    SQUARE(Z, -0.5, FLAG, FLAG_STENCIL_WRITE, X, -0.25, LIGHT, -1);
    TRANSFORM(FLAG, FLAG_STENCIL_FILTER) {
        CIRCLE(Z, -0.5);
        CIRCLE(G, -0.5);
    }

    RSQUARE(4, Z, -0.5, SX, 0.4, X, 0.2);
    RSQUARE(4, SX, 0.4, X, 0.2, G, -0.5);

    TRANSFORM(FLAG, FLAG_STENCIL_FILTER) {
        CIRCLE(S, 0.8, LIGHT, -0.5);
    }

    TRIANGLE(S, 0.4, LIGHT, 1);
    TRIANGLE(S, 0.4, LIGHT, 1, G, -0.5);
    TRIANGLE(X, -0.25, S, 0.2, LIGHT, 1);
    TRIANGLE(X, -0.25, S, 0.2, LIGHT, 1, G, -0.5);

    END
}

void blowfish_objs(noctt_turtle_t *ctx)
{
    START
    TEXT("Som objects from Blowish Rescue", X, -0.48, Y, 0.47);
    CALL(cannon, SN, S, 0.25);
    CALL(bomb, SN, S, 0.25, X, 1.5);
    CALL(block, SN, S, 0.25, X, -1.5);
    CALL(saw, SN, S, 0.25, X, -1.5, -1.25);
    END
}
