
/* Some doc about the operations:
 *
 * Since I try to follow the same conventions as ContextFree when possible, we
 * can also refer to the doc at:
 * http://www.contextfreeart.org/mediawiki/index.php/Shape_adjustment
 *
 * Transformation operations
 * -------------------------
 *
 *  S, x, [y], [z]
 *      Scale in x, y, and z.  If z is not specified, it is set to 1.  If y is
 *      not specified it is set to x.
 *
 *  SN
 *      Scale so that the x and y ratios are the same. Useful when we want
 *      to specify the rendered shape regardless of the current scale.
 *
 *  SX, x
 *      Scale in x only (same as S, x, 1, 1).
 *
 *  SY, y
 *      Scale in y only (same as S, 1, y, 1).
 *
 *  SZ, z
 *      Scale in z only (same as S, 1, 1, z).
 *
 *
 *  X, x, [y], [z]
 *      Translate in x, y and z.  If y or z are not specified, they are set
 *      to 0.
 *
 *  Y, y
 *      Translate in y only (same as X, 0, y).
 *
 *  Z, z
 *      Translate in z only (same as X, 0, 0, z).
 *
 *  R, a
 *      Rotate along the z axis of an angle of a (in degree).
 *
 *  FLIP, a
 *      Flip along the angle a (in degree).
 *
 *  G, x, [y]
 *      Scale along x and y, such that the size of square grows a fixed amount.
 *      This is useful for border effects.
 *
 *
 * Color operations
 * ----------------
 *
 *  HUE, SAT, LIGHT and A can be used to modify the color (A is for alpha).
 *
 *  SAT, LIGHT, and A can be used with a single argument, in that case it
 *  moves the value toward 1 if the argument is positifs and toward 0 if the
 *  argument is negative.  The ratio of change is the absolute value of the
 *  argument.  So an argument of 0 don't do anything, an argument of -1 set
 *  the value to 0, and an argument of +1 set the value to 1.
 *
 *  With two arguments, the function mix the current value and the second
 *  argument according to the first argument value, for example:
 *
 *      LIGHT, t, x   =>   light = mix(light, x, t);
 *
 *
 *  HUE, [t], x
 *      Add x to the hue.
 *
 *  SAT, [t], s,
 *      Adjust the saturation
 *
 *  LIGHT, [t], l
 *      Adjust the lightness
 *
 *  A, [t], a
 *      Adjust the alpha value
 *
 *  HSL, [t], h, s, l
 *      Same as HUE, h, SAT, s, LIGHT, l
 *
 *
 * Other operations
 * ----------------
 *
 *  VAR, i0, v0, [i1, v1], ..., [in, vn]
 *      Set the context variables value.
 */

#include <float.h>
#include <stdbool.h>

#ifndef NOCTT_NB_VARS
#   define NOCTT_NB_VARS 0
#endif


typedef struct noctt_turtle noctt_turtle_t;
typedef void (*noctt_rule_func_t)(noctt_turtle_t*);

struct noctt_turtle {
    float               mat[16];
    float               scale[2]; // Should we compute it from the matrix?
    float               color[4];
    noctt_turtle_t      *wait;
    noctt_rule_func_t   func;
    int                 iflags;  // Internal flags.
    int                 step;
    int                 time;
    int                 n, i;
    float               vars[NOCTT_NB_VARS];
};

enum {
    NOCTT_OP_END = 0,
    NOCTT_OP_S,       // Scale, with no arg normalize x/y scale.
    NOCTT_OP_SN,      // Normalize x/y scale.
    NOCTT_OP_SAXIS,   // Scale in a single axis. (For SX, SY and SZ).
    NOCTT_OP_X,       // Translate.
    NOCTT_OP_R,       // Rotate.
    NOCTT_OP_G,       // Grow
    NOCTT_OP_FLIP,

    NOCTT_OP_HSL,
    NOCTT_OP_HUE,
    NOCTT_OP_SAT,
    NOCTT_OP_LIGHT,
    NOCTT_OP_A,       // Alpha

    NOCTT_OP_VAR,

    NOCTT_OP_COUNT,
};

// Internal flags.
enum {
    NOCTT_FLAG_DONE         = 1 << 0,
    NOCTT_FLAG_JUST_CLONED  = 1 << 1,
    NOCTT_FLAG_BLOCK_DONE   = 1 << 2,
};

#define NOCTT_OP_START FLT_MAX

#define S       NOCTT_OP_START, NOCTT_OP_S
#define SN      NOCTT_OP_START, NOCTT_OP_SN
#define SX      NOCTT_OP_START, NOCTT_OP_SAXIS, 0
#define SY      NOCTT_OP_START, NOCTT_OP_SAXIS, 1
#define SZ      NOCTT_OP_START, NOCTT_OP_SAXIS, 2
#define X       NOCTT_OP_START, NOCTT_OP_X
#define Y       X, 0
#define Z       X, 0, 0
#define R       NOCTT_OP_START, NOCTT_OP_R
#define FLIP    NOCTT_OP_START, NOCTT_OP_FLIP

#define HUE     NOCTT_OP_START, NOCTT_OP_HUE
#define SAT     NOCTT_OP_START, NOCTT_OP_SAT
#define LIGHT   NOCTT_OP_START, NOCTT_OP_LIGHT
#define HSL     NOCTT_OP_START, NOCTT_OP_HSL
#define A       NOCTT_OP_START, NOCTT_OP_A

#define G       NOCTT_OP_START, NOCTT_OP_G
#define VAR     NOCTT_OP_START, NOCTT_OP_VAR

#define T_TR(...) do { \
        const float ops_[] = {__VA_ARGS__}; \
        noctt_tr(ctx, sizeof(ops_) / sizeof(float), ops_); \
    } while (0)
#define T_RULE(rule) void rule(noctt_turtle_t *ctx)

#define T_START \
    noctt_turtle_t *new_ = 0; \
    (void) new_; \
    switch (ctx->step) {          \
        case 0:;

#define T_END   \
        T_KILL; \
    }

#define T_PRIMITIVE_(func, ...) do { \
    const float ops_[] = {__VA_ARGS__}; \
    noctt_turtle_t ctx_ = *ctx; \
    noctt_tr(&ctx_, sizeof(ops_) / sizeof(float), ops_); \
    func; \
} while (0)

#define T_SQUARE(...)      T_PRIMITIVE_(noctt_square(&ctx_), __VA_ARGS__)
#define T_RSQUARE(r, ...)  T_PRIMITIVE_(noctt_rsquare(&ctx_, r), __VA_ARGS__)
#define T_CIRCLE(...)      T_PRIMITIVE_(noctt_circle(&ctx_), __VA_ARGS__)
#define T_STAR(n, t, c, ...) \
                    T_PRIMITIVE_(noctt_star(&ctx_, n, t, c), __VA_ARGS__)
#define T_TRIANGLE(...)    T_STAR(3, 0, 0, ##__VA_ARGS__)

#define T_YIELD do { \
    ctx->step = __LINE__ * 8; \
    ctx->iflags |= NOCTT_FLAG_DONE; \
    return; \
    case __LINE__ * 8:; \
} while (0)

#define T_CLONE(mode, ...) do { \
    ctx->step = __LINE__ * 8; \
    const float ops_[] = {__VA_ARGS__}; \
    noctt_clone(ctx, mode, sizeof(ops_) / sizeof(float), ops_); \
    if (mode == 1) return; \
    } while (0); \
    case __LINE__ * 8:; \

#define T_CALL(rule, ...) do { \
    T_CLONE(1, ##__VA_ARGS__); \
    if (ctx->iflags & NOCTT_FLAG_JUST_CLONED) { \
        ctx->iflags &= ~NOCTT_FLAG_JUST_CLONED; \
        ctx->func = rule; \
        ctx->step = 0; \
        return; \
    } \
} while (0)

#define T_SPAWN(rule, ...) do { \
    T_CLONE(0, ##__VA_ARGS__); \
    if (ctx->iflags & NOCTT_FLAG_JUST_CLONED) { \
        ctx->iflags &= ~NOCTT_FLAG_JUST_CLONED; \
        ctx->func = rule; \
        ctx->step = 0; \
        return; \
    } \
} while (0)

#define T_RUN_BLOCK_AND_KILL_ \
    for (ctx->iflags &= ~NOCTT_FLAG_BLOCK_DONE; ; \
         ctx->iflags |= NOCTT_FLAG_BLOCK_DONE) \
        if (ctx->iflags & NOCTT_FLAG_BLOCK_DONE) { \
            noctt_kill(ctx); \
            return; \
        } \
        else

#define T_TRANSFORM_SPAWN(...) \
    T_CLONE(0, ##__VA_ARGS__); \
    if (ctx->iflags & NOCTT_FLAG_JUST_CLONED) \
        if (ctx->iflags |= ~NOCTT_FLAG_JUST_CLONED, 1) \
            T_RUN_BLOCK_AND_KILL_

#define T_TRANSFORM(...) \
    T_CLONE(1, ##__VA_ARGS__); \
    if (ctx->iflags & NOCTT_FLAG_JUST_CLONED) \
        if (ctx->iflags |= ~NOCTT_FLAG_JUST_CLONED, 1) \
            T_RUN_BLOCK_AND_KILL_

#define T_FOR(n_, ...) \
    T_CLONE(1); \
    if (ctx->iflags & NOCTT_FLAG_JUST_CLONED) { \
        ctx->step = __LINE__ * 8 + 1; case __LINE__ * 8 + 1:; \
        ctx->n = n_; \
        for (ctx->i = 0; ctx->i < ctx->n; ctx->i++) { \
            ctx->step = __LINE__ * 8 + 2; \
            noctt_clone(ctx, 1, 0, NULL); \
            T_TR(__VA_ARGS__); \
            return; \
            case __LINE__ * 8 + 2:; \
            if (ctx->iflags & NOCTT_FLAG_JUST_CLONED) { \
                ctx->step = __LINE__ * 8 + 3; \
                return; \
            } \
        } \
        noctt_kill(ctx); \
        return; \
    } \
    case __LINE__ * 8 + 3:; \
    if (ctx->iflags & NOCTT_FLAG_JUST_CLONED) \
        T_RUN_BLOCK_AND_KILL_


#define T_KILL do { noctt_kill(ctx); return; } while(0)

void noctt_square(const noctt_turtle_t *ctx);
void noctt_rsquare(const noctt_turtle_t *ctx, float r);
void noctt_circle(const noctt_turtle_t *ctx);
void noctt_star(const noctt_turtle_t *ctx, int n, int t, int c);

void noctt_kill(noctt_turtle_t *ctx);
void noctt_tr(noctt_turtle_t *ctx, int n, const float *ops);
void noctt_clone(noctt_turtle_t *ctx, int mode, int n, const float *ops);

typedef void (*noctt_render_func_t)(int n, float (*poly)[3], float color[4],
                                    void *user_data);

typedef struct noctt_prog {
    int                 nb;         // total number of contexts.
    // int             nb_active;  // number of active contexts.
    unsigned long       rand_next;
    noctt_render_func_t render_callback;
    void                *render_callback_data;
    // Kill context if x or y scale get below this value.
    float               min_scale;
    noctt_turtle_t      turtles[];
} noctt_prog_t;

bool brand(float x);
float pm(float x, float a);

noctt_prog_t *noctt_prog_create(noctt_rule_func_t rule, int nb,
                                int seed, float rect[4]);
void noctt_prog_delete(noctt_prog_t *prog);
void noctt_prog_iter(noctt_prog_t *prog);
