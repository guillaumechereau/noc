/* noc_turtle library
 *
 * Copyright (c) 2015 Guillaume Chereau <guillaume@noctua-software.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/*
 *
 * This library allows to create procedural graphics from a set of rules,
 * The syntax of the rules is somehow similar to the ContextFree project
 * (http://www.contextfreeart.org).
 *
 * I strongly suggest that you first check the sample code in tests/turtle.c
 * if you want to get an idea of what it can do.
 *
 * A rule is defined as a C function that takes a noctt_turtle_t object as
 * argument.  Since all the rules are actually coroutine relying on macros
 * to hide the dirty details, you have to enclose the code with a START and
 * END.  Here is a basic rule that just renders a square:
 *
 *     void my_rule(noctt_turtle_t *turtle)
 *     {
 *         START
 *         SQUARE();
 *         END
 *     }
 *
 * Turtles
 * -------
 *
 * Every rule is executed in the context of a turtle (I use this name since
 * this is similar to the turtle in the LOGO language).  A turtle possess a
 * 4x4 transformation matrix that represents its position, rotation and scale.
 * It also has a color and a set of flags and variables that can be set by the
 * user.
 *
 * You can change the turtle properties using operations and the TR macro.
 * For example to move the turtle a unit distance in the X direction, we
 * can write:
 *
 *     TR(X, 1);
 *
 * The macro can take an arbitrary number of arguments, so we can chain the
 * operations:  Here we move the turtle on 1 along X, then rotate it of
 * 45 degrees, and finally scale it to half its size:
 *
 *     SQUARE();   // First square at the current pos.
 *     TR(X, 1, R, 45, S, 0.5);
 *     SQUARE();   // Second square in the new position.
 *
 * If we just want to apply a transformation in the context of a rendering
 * operation, we can put it as argument of the operation, so the previous
 * example can be written as:
 *
 *     SQUARE();
 *     SQUARE(X, 1, R, 45, S, 0.5);
 *     // At the point the turtle is still in its original context.
 *
 * If you want to apply a transformation only for a block of code, you can
 * use the TRANSFORM macro:
 *
 *     TRANSFORM(LIGHT, 0.5) {  // Only affect the block.
 *         SQUARE();
 *         CIRCLE(X, 1);
 *     }
 *
 * You can have several turtles running at the same time.  In order to create
 * a new turtle, we can spawn a previously defined rule.  To control the
 * timing, we need to use the YIELD macro, that tell the turtle to wait one
 * iteration before proceeding with the rest of the rule.
 *
 *     void a_rule(noctt_turtle_t *turtle)
 *     {
 *         START
 *         SQUARE();
 *         YIELD();
 *         SQUARE(S, 0.8, LIGHT, -0.5);
 *         END
 *     }
 *
 *     void main_rule(noctt_turtle_t *turtle)
 *     {
  *        START
 *         // Create a new turtle from the current one, scale it, and make it
 *         // process the rule 'a_rule'.
 *         SPAWN(a_rule, S, 0.5);
 *         // The rest of the current rule will run in parallel to a_rule.
 *         ...
 *         END
 *     }
 *
 * An other way is to use the TRANSFORM_SPAWN macro, that runs a block in
 * the context of the new turtle.
 *
 * Looping
 * -------
 *
 * The LOOP macro allow to run a loop, with an optional transformation applied
 * at every iteration:
 *
 *     // Render 10 circles each one scaled by 0.9 from the previous one,
 *     // and 10/100 darker.
 *     LOOP(10, S, 0.9, LIGHT, -0.1) {
 *         CIRCLE();
 *     }
 *
 * Usage
 * -----
 * The library does not do the rendering, instead it relies on the client
 * to provide a callback that will be called each time a render operation
 * is executed.  The signature of the callback is:
 *
 * void *callback(int n, const noctt_vec3_t *poly,
 *                const float color[4],
 *                unsigned int flags, void *user_data);
 *
 * n          : number of vertices to renders.
 * poly       : pointer to the vertices as float[3] x,y,z values.
 * color      : the color, as (Hue, Sat, Value, Alpha) tuple.  It is up to the
 *              client to convert to RGBA if needed.
 * flags      : the current turtle user flag values.  This is up to the client
 *              to define what they mean.
 * user_data  : can be set by the user.
 *
 *
 * Typically the client would first create a program, with noctt_prog_create,
 * passing the pointer to the initial rule to be called:
 *
 *     noctt_prog_t *prog = noctt_prog_create(
 *         my_rule,  // The rule to call.
 *         256,      // Max number of turtles.
 *         0,        // Inital seed.
 *         NULL,     // Optional initial transformation matrix.
 *         1);       // Pixel logical size (used for the G operation).
 *
 * Then set the rendering callback:
 *
 *     prog->render_callback = my_render_callback;
 *     prog->render_callback_data = NULL;
 *
 * Then we can call noctt_prog_iter to step into the rendering, our callback
 * will be called appropriately.
 *
 *     while (proc->active) {
 *         noctt_prg_iter(prog);
 *     }
 *
 * Finally when we are done, we can delete the program:
 *
 *     noctt_prog_delete(prog);
 *
 *
 * Some doc about the operations
 * -----------------------------
 *
 * Since I try to follow the same conventions as ContextFree when possible, we
 * can also refer to the doc at:
 * http://www.contextfreeart.org/mediawiki/index.php/Shape_adjustment
 *
 * Rendering calls
 * ---------------
 *
 * All the calls accept extra arguments for the operations to apply before
 * the rendering is done.
 *
 * POLY(n, float (*verts)[3], ...)
 *     Render a polygon of n vertices.
 *     // XXX: to implement.
 *
 * SQUARE(...)
 *     Render a square
 *
 * CIRCLE(...)
 *     Render a circle
 *
 * RSQUARE(float r, ...)
 *     Render a rounded square.  r is the max size of the rectangle such
 *     that the rounded radius will be zero.
 *
 * STAR(int n, float t, float c, ...)
 *     Render a star with n branches, t is the flatness of the star,
 *     c define the center of the branch top.  Sorry this doc is
 *     no useful, need to add some examples.
 *
 * TRIANGLE(...)
 *     Conveniance for STAR(3, 0, 0)
 *
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
 *  VAR, i0, v0, [i1, v1, ..., in, vn]
 *      Set the context variables value.
 *
 *  FLAG f [v], [f2, v2, ..., fn, vn]
 *      Set the flag value.  If v is not defined, the default is 1.
 */

/* Notes about the implementation
 * ------------------------------
 *
 * Yes, the code is hard to follow, this is what we get from abusing the
 * C language to fake coroutines.
 *
 * The rules are all really based around a big switch that jumps to the
 * current state.  This has some important implications in what we can do
 * inside the rules:
 *
 *     - Since the code make heavy use of the __LINE__ macro, it is not
 *       possible to put two of them on the same line!  So do not write
 *       something like: LOOP(10) { SQUARE(); }.  This might or might not
 *       work.  In the best case the compiler will refuse to compile.
 *
 *     - It is not possible to use local variables.  In fact it is possible,
 *       but you have to be very careful about it, so don't do it if you
 *       don't know how this thing work, or until I write a doc about it.
 *
 *     - We cannot use switch blocks inside the rules.  This is unfortunate.
 *       Maybe I will try to find a way to fix this...
 *
 *     - However, it is perfectly fine to call any C functions, which is one
 *       of the advantage over using a script language.
 *
 */


/* If NOC_TURTLE_DEFINE_NAMES is defined, then we will create default
 * conveniance macros.  If you don't do it, then you have to use the names with
 * NOCTT_ prefix, or define the macros by yourself.
 *
 * You can also include this file with NOC_TURTLE_UNDEF_NAMES to undef all
 * the names.
 *
 * So a common use case would be to enclose your rules with two includes to
 * define and undefine the macros, like this:
 *
 *     #define NOC_TURTLE_DEFINE_NAMES
 *     #include "noc_turtle.h"
 *
 *     // All the code here here can use the default names...
 *
 *     #define NOC_TURTLE_UNDEF_NAME
 *     #include "noc_turtle.h"
 *
 *
 */
#if defined NOC_TURTLE_UNDEF_NAMES

#undef S
#undef SN
#undef SX
#undef SY
#undef SZ
#undef X
#undef Y
#undef Z
#undef R
#undef FLIP
#undef HUE
#undef SAT
#undef LIGHT
#undef HSL
#undef A
#undef G
#undef VAR
#undef FLAG

#undef TR
#undef START
#undef END
#undef YIELD
#undef CALL
#undef JUMP
#undef SPAWN
#undef KILL

#undef SQUARE
#undef RSQUARE
#undef CIRCLE
#undef STAR
#undef TRIANGLE
#undef POLY
#undef TRANSFORM_SPAWN
#undef TRANSFORM
#undef LOOP

#undef PM
#undef BRAND
#undef FRAND

#elif defined NOC_TURTLE_DEFINE_NAMES

#define S       NOCTT_S
#define SN      NOCTT_SN
#define SX      NOCTT_SX
#define SY      NOCTT_SY
#define SZ      NOCTT_SZ
#define X       NOCTT_X
#define Y       NOCTT_Y
#define Z       NOCTT_Z
#define R       NOCTT_R
#define FLIP    NOCTT_FLIP
#define HUE     NOCTT_HUE
#define SAT     NOCTT_SAT
#define LIGHT   NOCTT_LIGHT
#define HSL     NOCTT_HSL
#define A       NOCTT_A
#define G       NOCTT_G
#define VAR     NOCTT_VAR
#define FLAG    NOCTT_FLAG

#define TR(...)       NOCTT_TR(__VA_ARGS__)
#define START         NOCTT_START
#define END           NOCTT_END
#define YIELD(...)    NOCTT_YIELD(__VA_ARGS__)
#define CALL(...)     NOCTT_CALL(__VA_ARGS__)
#define JUMP(...)     NOCTT_JUMP(__VA_ARGS__)
#define SPAWN(...)    NOCTT_SPAWN(__VA_ARGS__)
#define KILL()        NOCTT_KILL()

#define SQUARE(...)            NOCTT_SQUARE(__VA_ARGS__)
#define RSQUARE(...)           NOCTT_RSQUARE(__VA_ARGS__)
#define CIRCLE(...)            NOCTT_CIRCLE(__VA_ARGS__)
#define STAR(...)              NOCTT_STAR(__VA_ARGS__)
#define TRIANGLE(...)          NOCTT_TRIANGLE(__VA_ARGS__)
#define POLY(...)              NOCTT_POLY(__VA_ARGS__)
#define TRANSFORM_SPAWN(...)   NOCTT_TRANSFORM_SPAWN(__VA_ARGS__)
#define TRANSFORM(...)         NOCTT_TRANSFORM(__VA_ARGS__)
#define LOOP(...)              NOCTT_LOOP(__VA_ARGS__)

#define PM(x_, y_)    noctt_pm(turtle, x_, y_)
#define BRAND(x_)     noctt_brand(turtle, x_)
#define FRAND(a_, b_) noctt_frand(turtle, a_, b_)

#endif

#ifndef _NOC_TURTLE_H_
#define _NOC_TURTLE_H_

#include <float.h>
#include <stdbool.h>

// Maybe I should remove this, and let the client pass a pointer to his own
// defined structure to hold turtle variables.
#ifndef NOCTT_NB_VARS
#   define NOCTT_NB_VARS 3
#endif

typedef struct {
    float x, y, z;
} noctt_vec3_t;

typedef struct noctt_turtle noctt_turtle_t;
typedef void (*noctt_rule_func_t)(noctt_turtle_t*);
typedef struct noctt_prog noctt_prog_t;

struct noctt_turtle {
    noctt_prog_t        *prog;
    float               mat[16];
    float               scale[2]; // Should we compute it from the matrix?
    float               color[4];
    int                 wait;    // Index of the turtle we wait for.
    noctt_rule_func_t   func;
    unsigned int        iflags;  // Internal flags.
    unsigned int        flags;   // User defined flags.
    int                 step;
    int                 time;
    int                 n, i, tmp;
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
    NOCTT_OP_FLAG,

    NOCTT_OP_COUNT,
};

// Internal flags.
enum {
    NOCTT_FLAG_DONE         = 1 << 0,
    NOCTT_FLAG_JUST_CLONED  = 1 << 1,
    NOCTT_FLAG_WAITING      = 1 << 2,
    NOCTT_FLAG_BLOCK_DONE   = 1 << 3,
};

#define NOCTT_OP_START FLT_MAX

#define NOCTT_S       NOCTT_OP_START, NOCTT_OP_S
#define NOCTT_SN      NOCTT_OP_START, NOCTT_OP_SN
#define NOCTT_SX      NOCTT_OP_START, NOCTT_OP_SAXIS, 0
#define NOCTT_SY      NOCTT_OP_START, NOCTT_OP_SAXIS, 1
#define NOCTT_SZ      NOCTT_OP_START, NOCTT_OP_SAXIS, 2
#define NOCTT_X       NOCTT_OP_START, NOCTT_OP_X
#define NOCTT_Y       NOCTT_X, 0
#define NOCTT_Z       NOCTT_X, 0, 0
#define NOCTT_R       NOCTT_OP_START, NOCTT_OP_R
#define NOCTT_FLIP    NOCTT_OP_START, NOCTT_OP_FLIP

#define NOCTT_HUE     NOCTT_OP_START, NOCTT_OP_HUE
#define NOCTT_SAT     NOCTT_OP_START, NOCTT_OP_SAT
#define NOCTT_LIGHT   NOCTT_OP_START, NOCTT_OP_LIGHT
#define NOCTT_HSL     NOCTT_OP_START, NOCTT_OP_HSL
#define NOCTT_A       NOCTT_OP_START, NOCTT_OP_A

#define NOCTT_G       NOCTT_OP_START, NOCTT_OP_G
#define NOCTT_VAR     NOCTT_OP_START, NOCTT_OP_VAR
#define NOCTT_FLAG    NOCTT_OP_START, NOCTT_OP_FLAG

// If __COUNTER__ is defined (gcc and clang), we use it, since it has the
// advantage of staying the same when we modify the code.  It also allows
// to put several on the same line.
// NOCTT_MARKER is used to generate a uniq int that can be used for labels
// If we want to put several on the same line, we have to use a different
// 'n' value for each of them.  If we want to get the same value for two
// markers, we need to put the same n, but also set the shift of one to the
// ofset between the two (so that it works with __COUNTER__ too).
#ifdef __COUNTER__
    #define NOCTT_MARKER(n, shift) (__COUNTER__ + shift)
#else
    #define NOCTT_MARKER(n, shift) (__LINE__ * 8 + n)
#endif

#define NOCTT_TR(...) do { \
        const float ops_[] = {__VA_ARGS__}; \
        noctt_tr(turtle, sizeof(ops_) / sizeof(float), ops_); \
    } while (0)

#define NOCTT_START \
    switch (turtle->step) {          \
        case 0:;

#define NOCTT_END   \
        NOCTT_KILL(); \
    }

#define NOCTT_PRIMITIVE_(func, ...) do { \
    const float ops_[] = {__VA_ARGS__}; \
    noctt_turtle_t turtle_ = *turtle; \
    noctt_tr(&turtle_, sizeof(ops_) / sizeof(float), ops_); \
    func; \
} while (0)

#define NOCTT_SQUARE(...)      \
    NOCTT_PRIMITIVE_(noctt_square(&turtle_), __VA_ARGS__)
#define NOCTT_RSQUARE(r, ...)  \
    NOCTT_PRIMITIVE_(noctt_rsquare(&turtle_, r), __VA_ARGS__)
#define NOCTT_CIRCLE(...)      \
    NOCTT_PRIMITIVE_(noctt_circle(&turtle_), __VA_ARGS__)
#define NOCTT_STAR(n, t, c, ...) \
    NOCTT_PRIMITIVE_(noctt_star(&turtle_, n, t, c), __VA_ARGS__)
#define NOCTT_POLY(n, p, ...) \
    NOCTT_PRIMITIVE_(noctt_poly(&turtle_, n, p), __VA_ARGS__)
#define NOCTT_TRIANGLE(...)    NOCTT_STAR(3, 0, 0, ##__VA_ARGS__)

#define NOCTT_COMMA_ ,
#define NOCTT_YIELD_(_, n_, ...) do { \
    turtle->tmp = n_; \
    turtle->step = NOCTT_MARKER(0, 1); \
    case NOCTT_MARKER(0, 0):; \
    if (turtle->tmp--) { \
        turtle->iflags |= NOCTT_FLAG_DONE; \
        return; \
    } \
} while (0)
#define NOCTT_YIELD(...) NOCTT_YIELD_(0, ##__VA_ARGS__, 1)


#define NOCTT_CLONE(mode, ...) do { \
    turtle->step = NOCTT_MARKER(0, 1); \
    const float ops_[] = {__VA_ARGS__}; \
    noctt_clone(turtle, mode, sizeof(ops_) / sizeof(float), ops_); \
    if (mode == 1) return; \
    } while (0); \
    case NOCTT_MARKER(0, 0):; \

#define NOCTT_CALL(rule, ...) do { \
    NOCTT_CLONE(1, ##__VA_ARGS__); \
    if (turtle->iflags & NOCTT_FLAG_JUST_CLONED) { \
        turtle->iflags &= ~NOCTT_FLAG_JUST_CLONED; \
        turtle->func = rule; \
        turtle->step = 0; \
        return; \
    } \
} while (0)

#define NOCTT_JUMP(rule, ...) do { \
    TR(__VA_ARGS__); \
    turtle->func = rule; \
    turtle->step = 0; \
    return; \
} while (0)

#define NOCTT_SPAWN(rule, ...) do { \
    NOCTT_CLONE(0, ##__VA_ARGS__); \
    if (turtle->iflags & NOCTT_FLAG_JUST_CLONED) { \
        turtle->iflags &= ~NOCTT_FLAG_JUST_CLONED; \
        turtle->func = rule; \
        turtle->step = 0; \
        return; \
    } \
} while (0)

#define NOCTT_UNIQ_LABEL__(n, line) label_ ## line ## _ ## n
#define NOCTT_UNIQ_LABEL_(n, line) NOCTT_UNIQ_LABEL__(n, line)
#define NOCTT_UNIQ_LABEL(n) NOCTT_UNIQ_LABEL_(n, __LINE__)

/*
#define NOCTT_RUN_BLOCK_AND_KILL_ \
    if (1) {goto NOCTT_UNIQ_LABEL(0);} \
    else \
        while (1) \
            if (1) { \
                noctt_kill(turtle); \
                return; \
            } \
            else \
                NOCTT_UNIQ_LABEL(0):
*/

#define NOCTT_RUN_BLOCK_AND_KILL_ \
    for (turtle->iflags &= ~NOCTT_FLAG_BLOCK_DONE; ; \
         turtle->iflags |= NOCTT_FLAG_BLOCK_DONE) \
        if (turtle->iflags & NOCTT_FLAG_BLOCK_DONE) { \
            noctt_kill(turtle); \
            return; \
        } else

#define NOCTT_TRANSFORM_SPAWN(...) \
    NOCTT_CLONE(0, ##__VA_ARGS__); \
    if (turtle->iflags & NOCTT_FLAG_JUST_CLONED) \
        if ((turtle->iflags &= ~NOCTT_FLAG_JUST_CLONED), 1) \
            NOCTT_RUN_BLOCK_AND_KILL_

#define NOCTT_TRANSFORM(...) \
    NOCTT_CLONE(1, ##__VA_ARGS__); \
    if (turtle->iflags & NOCTT_FLAG_JUST_CLONED) \
        if ((turtle->iflags &= ~NOCTT_FLAG_JUST_CLONED), 1) \
            NOCTT_RUN_BLOCK_AND_KILL_

#define NOCTT_LOOP(n_, ...) \
    turtle->tmp = n_; \
    NOCTT_CLONE(1); \
    if (turtle->iflags & NOCTT_FLAG_JUST_CLONED) { \
        turtle->step = NOCTT_MARKER(1, 1); case NOCTT_MARKER(1, 0):; \
        turtle->n = turtle->tmp; \
        for (turtle->i = 0; turtle->i < turtle->n; turtle->i++) { \
            turtle->step = NOCTT_MARKER(2, 1); \
            noctt_clone(turtle, 1, 0, 0); \
            NOCTT_TR(__VA_ARGS__); \
            return; \
            case NOCTT_MARKER(2, 0):; \
            if (turtle->iflags & NOCTT_FLAG_JUST_CLONED) { \
                turtle->step = NOCTT_MARKER(3, 1); \
                return; \
            } \
        } \
        noctt_kill(turtle); \
        return; \
    } \
    case NOCTT_MARKER(3, 0):; \
    if (turtle->iflags & NOCTT_FLAG_JUST_CLONED) \
        NOCTT_RUN_BLOCK_AND_KILL_


#define NOCTT_KILL() do { noctt_kill(turtle); return; } while(0)

noctt_vec3_t noctt_get_pos(const noctt_turtle_t *turtle);
void noctt_square(const noctt_turtle_t *turtle);
void noctt_rsquare(const noctt_turtle_t *turtle, float r);
void noctt_circle(const noctt_turtle_t *turtle);
void noctt_star(const noctt_turtle_t *turtle, int n, float t, float c);
void noctt_poly(const noctt_turtle_t *turtle, int n, const noctt_vec3_t *poly);

void noctt_kill(noctt_turtle_t *turtle);
void noctt_tr(noctt_turtle_t *turtle, int n, const float *ops);
void noctt_clone(noctt_turtle_t *turtle, int mode, int n, const float *ops);

typedef void (*noctt_render_func_t)(int n, const noctt_vec3_t *poly,
                                    const float color[4],
                                    unsigned int flags, void *user_data);

struct noctt_prog {
    int                 nb;         // total number of turtles.
    int                 active;     // number of active turtles.
    unsigned long       rand_next;
    float               pixel_size;
    noctt_render_func_t render_callback;
    void                *render_callback_data;
    // Kill context if x or y scale get below this value.
    float               min_scale;
    noctt_turtle_t      turtles[];
};

float noctt_frand(noctt_turtle_t *turtle, float a, float b);
bool noctt_brand(noctt_turtle_t *turtle, float x);
float noctt_pm(noctt_turtle_t *turtle, float x, float a);

noctt_prog_t *noctt_prog_create(noctt_rule_func_t rule, int nb,
                                int seed, float rect[16], float pixel_size);
void noctt_prog_delete(noctt_prog_t *prog);
void noctt_prog_iter(noctt_prog_t *prog);

#endif // _NOC_TURTLE_H_
