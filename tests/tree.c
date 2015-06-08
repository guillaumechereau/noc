
#define NOC_TURTLE_DEFINE_NAMES
#include "noc_turtle.h"

void font_draw_text(float x, float y, const char *str);
// Defines a custom directive to render text.
#define TEXT(msg, ...) TRANSFORM(__VA_ARGS__) { \
    noctt_vec3_t pos = noctt_get_pos(ctx); \
    font_draw_text(pos.x, pos.y, msg); \
}

static void moon(noctt_turtle_t *ctx)
{
    START
    CIRCLE(LIGHT, -0.5, G, 2);
    FOR(32, S, 0.95, LIGHT, 0.01) {
        CIRCLE();
        YIELD();
    }
    END
}

static void part(noctt_turtle_t *ctx)
{
    START
    RSQUARE(0, SX, 0.2, HUE, PM(0, 15));
    RSQUARE(0, SX, 0.2, LIGHT, -0.4, G, 2, Z, -0.5);
    ctx->vars[0] += 1;
    if (ctx->vars[0] == 15) {
        TR(S, PM(1, 0.4));
        CIRCLE(HUE, PM(0, 45));
        CIRCLE(HUE, PM(0, 45), LIGHT, -0.4, G, 2, Z, -0.5);
        KILL();
    }
    if (BRAND(0.3)) {
        SPAWN(part, R, PM(0, 90), Y, 0.5);
    }
    YIELD(4);
    CALL(part, Y, 0.45, R, PM(0, 45), Y, 0.45, S, 0.9);
    END
}

void tree_rule(noctt_turtle_t *ctx)
{
    START
    TR(HSL, 180, 0.5, 0.5);
    SQUARE(LIGHT, 0.1, SAT, -0.5, Z, -1);
    TEXT("Example of using depth buffer for border effects",
         X, -0.48, Y, 0.47);
    SPAWN(moon, X, 0.3, 0.3, SN, S, 0.2);
    TR(Y, -0.5, SN, S, 0.1);
    SPAWN(part);
    END
}

