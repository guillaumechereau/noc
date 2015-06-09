
#define NOC_TURTLE_DEFINE_NAMES
#include "noc_turtle.h"

#include <stdio.h>

static void branch(noctt_turtle_t *ctx)
{
    START
    SQUARE(SY, 0.2);
    TR(LIGHT, -0.01);
    YIELD();
    if (BRAND(0.1)) {
        TR(FLIP, 0);
        SPAWN(branch, R, -90);
    }
    if (BRAND(0.01)) {
        CIRCLE(S, 2);
    }
    JUMP(branch, X, 0.4, R, PM(0, 1), X, 0.4, S, 0.99);
    END
}

void modern_rule(noctt_turtle_t *ctx)
{
    START
    TR(SN, S, 0.2, LIGHT, 1);
    CIRCLE();
    FOR(4) {
        SPAWN(branch, R, PM(0, 180), S, 0.1, X, 1);
    }
    END
}

