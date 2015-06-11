#include "noc_turtle.h"
#include "font.h"

#include <math.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>


enum {
    FLAG_STENCIL_WRITE  = 1 << 0,
    FLAG_STENCIL_FILTER = 1 << 1,
    FLAG_EFFECT_LIGHT   = 1 << 2,
};

typedef struct {
    GLuint prog;
    GLuint u_proj_l;
    GLuint u_color_l;
    GLuint a_pos_l;
} gl_prog_t;

static gl_prog_t gl_prog;

#define NOC_TURTLE_DEFINE_NAMES
#include "noc_turtle.h"

// Defines a custom directive to render text.
#define TEXT(msg, ...) TRANSFORM(__VA_ARGS__) { \
    noctt_vec3_t pos = noctt_get_pos(ctx); \
    font_draw_text(pos.x, pos.y, msg); \
}

// Defined in blowfish_city.c
void blowfish_city_rule(noctt_turtle_t *ctx);
// Defined in blowfish_objs.c
void blowfish_objs(noctt_turtle_t *ctx);

static void spiral_node(noctt_turtle_t *ctx)
{
    START
    SQUARE();
    TR(HUE, PM(0, 5));
    YIELD();
    if (BRAND(0.01)) {
        TR(FLIP, 0);
        SPAWN(spiral_node, R, -90);
    }
    SPAWN(spiral_node, X, 0.4, R, 3, X, 0.4, S, 0.99);
    END
}

static void spiral(noctt_turtle_t *ctx)
{
    START
    TR(HSL, 1, 0, 1, 0.5);
    CALL(spiral_node);
    CALL(spiral_node, FLIP, 90);
    END
}

static void test_with_stencil(noctt_turtle_t *ctx)
{
    START
    TRANSFORM(FLAG, FLAG_STENCIL_WRITE) {
        SQUARE(LIGHT, -0.5);
        CIRCLE(LIGHT, -0.5, X, 0.5, 0.5, S, 0.5);
    }
    TRANSFORM(FLAG, FLAG, FLAG_STENCIL_FILTER) {
        CIRCLE(X, 0.5);
    }
    END
}

static void test(noctt_turtle_t *ctx)
{
    START

    TEXT("Press any key to see other demos", X, -0.47, Y, 0.47);

    SQUARE(S, 0.9, LIGHT, 0.2);
    SQUARE(S, 0.9, G, -1, LIGHT, 0.1);
    TR(SN, LIGHT, 1);

    TRANSFORM_SPAWN(X, -0.25, 0.25, S, 0.5) {
        FOR(64, G, -2, LIGHT, -0.02) {
            RSQUARE(60, S, 0.5);
            YIELD(4);
        }
    }

    SQUARE(S, 0.1);
    SQUARE(S, 0.1, X, 2);
    SQUARE(S, 0.1, X, 4, R, 45, LIGHT, -0.5);
    SQUARE(S, 0.1, X, 6, R, 45, LIGHT, -0.5, SAT, 1, HUE, 180);
    CIRCLE(S, 0.1, Y, 2);
    TRIANGLE(S, 0.1, Y, 4);

    CALL(test_with_stencil, S, 0.2, X, -2, -1);

    CALL(spiral, Y, -0.5, S, 0.02);
    END
}

// Defined in tree.c
void tree_rule(noctt_turtle_t *ctx);
// Defined in modern.c
void modern_rule(noctt_turtle_t *ctx);

static void shapes_rule(noctt_turtle_t *ctx)
{
    const noctt_vec3_t poly[] = {
        {-0.5, -0.5}, {0, -0.5}, {0.5, 0.5}, {-0.5, 0.5}};
    START
    TR(LIGHT, 1, S, 1.0 / 3, SN);
    SQUARE(             X, -1,  1, S, 0.9);
    CIRCLE(             X,  0,  1, S, 0.9);
    RSQUARE(80,         X,  1,  1, S, 0.9);
    TRIANGLE(           X, -1,  0, S, 0.9);
    STAR(5, 0.3, 0,     X,  0,  0, S, 0.9);
    STAR(8, 0.2, 0.9,   X,  1,  0, S, 0.9);
    POLY(4, poly,       X, -1, -1, S, 0.9);
    END
}

static void stencil_rule(noctt_turtle_t *ctx)
{
    START
    TR(SN, S, 0.5, LIGHT, 0.5);
    TRANSFORM(FLAG, FLAG_STENCIL_WRITE) {
        SQUARE();
        CIRCLE(X, 0.5, 0.5, S, 0.5);
    }
    TRANSFORM(FLAG, FLAG_STENCIL_FILTER) {
        CIRCLE(X, 0.5, LIGHT, 0.5);
    }
    END
}

static void colors_rule(noctt_turtle_t *ctx)
{
    START
    TR(SN, X, -0.5, -0.5, S, 1.0 / 64, SAT, 0.5);
    FOR(64, Y, 1, HUE, 360.0 / 64) {
        FOR(64, X, 1) {
            SQUARE(LIGHT, 1, (float)ctx->i / (ctx->n - 1));
        }
    }
    END
}

#define NOC_TURTLE_UNDEF_NAMES
#include "noc_turtle.h"

static void mat_ortho(float m[16], float left, float right, float bottom,
                      float top, float near, float far)
{
    float tx = -(right + left) / (right - left);
    float ty = -(top + bottom) / (top - bottom);
    float tz = -(far + near) / (far - near);
    float tmp[16] = {
        2 / (right - left), 0, 0, 0,
        0, 2 / (top - bottom), 0, 0,
        0, 0, -2 / (far - near), 0,
        tx, ty, tz, 1
    };
    memcpy(m, tmp, sizeof(tmp));
}

static void mat_scale(float m[16], float x, float y, float z)
{
    m[0] *= x;   m[4] *= y;   m[8]  *= z;
    m[1] *= x;   m[5] *= y;   m[9]  *= z;
    m[2] *= x;   m[6] *= y;   m[10] *= z;
    m[3] *= x;   m[7] *= y;   m[11] *= z;
}

static void init_opengl(int w, int h)
{
    const char *vshader_src =
        "uniform     mat4 u_proj;                       \n"
        "attribute   vec3 a_pos;                        \n"
        "void main()                                    \n"
        "{                                              \n"
        "   gl_Position = u_proj * vec4(a_pos, 1.0);    \n"
        "}                                              \n"
    ;

    const char *fshader_src =
        "uniform     vec4 u_color;                      \n"
        "void main()                                    \n"
        "{                                              \n"
        "   gl_FragColor = u_color;                     \n"
        "}                                              \n"
    ;

    float mat[16];
    GLuint vshader, fshader;

    vshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vshader, 1, &vshader_src, NULL);
    glCompileShader(vshader);

    fshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fshader, 1, &fshader_src, NULL);
    glCompileShader(fshader);

    gl_prog.prog = glCreateProgram();
    glAttachShader(gl_prog.prog, vshader);
    glAttachShader(gl_prog.prog, fshader);
    glLinkProgram(gl_prog.prog);

    gl_prog.u_proj_l = glGetUniformLocation(gl_prog.prog, "u_proj");
    gl_prog.u_color_l = glGetUniformLocation(gl_prog.prog, "u_color");
    gl_prog.a_pos_l = glGetAttribLocation(gl_prog.prog, "a_pos");
    glEnableVertexAttribArray(gl_prog.a_pos_l);

    glViewport(0, 0, w, h);
    glUseProgram(gl_prog.prog);
    mat_ortho(mat, -w / 2, +w / 2, -h / 2, h / 2, -1, +1);
    glUniformMatrix4fv(gl_prog.u_proj_l, 1, 0, mat);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
}

static void hsl_to_rgb(const float hsl[3], float rgb[3])
{
    float r = 0, g = 0, b = 0, c, x, m;
    const float h = hsl[0] / 60, s = hsl[1], l = hsl[2];
    c = (1 - fabs(2 * l - 1)) * s;
    x = c * (1 - fabs(fmod(h, 2) - 1));
    if      (h < 1) {r = c; g = x; b = 0;}
    else if (h < 2) {r = x; g = c; b = 0;}
    else if (h < 3) {r = 0; g = c; b = x;}
    else if (h < 4) {r = 0; g = x; b = c;}
    else if (h < 5) {r = x; g = 0; b = c;}
    else if (h < 6) {r = c; g = 0; b = x;}
    m = l - 0.5 * c;
    rgb[0] = r + m;
    rgb[1] = g + m;
    rgb[2] = b + m;
}

static void render_callback(int n, const noctt_vec3_t *poly, const float color[4],
                            unsigned int flags, void *user_data)
{
    float rgba[4];
    static unsigned int current_flags = 0;
    assert(sizeof(float) == sizeof(GL_FLOAT));
    if (current_flags != flags) {
        if (flags & (FLAG_STENCIL_WRITE | FLAG_STENCIL_FILTER)) {
            glEnable(GL_STENCIL_TEST);
            glStencilFunc(GL_LEQUAL, 0x1,
                          flags & FLAG_STENCIL_WRITE ? 0x0 : 0x1);
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        } else {
            glDisable(GL_STENCIL_TEST);
        }

        if (flags & FLAG_EFFECT_LIGHT) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
        } else {
            glDisable(GL_BLEND);
        }
        current_flags = flags;
    }
    hsl_to_rgb(color, rgba);
    rgba[3] = color[3];
    glUniform4fv(gl_prog.u_color_l, 1, rgba);
    glVertexAttribPointer(gl_prog.a_pos_l, 3, GL_FLOAT, false, 0, (void*)poly);
    glDrawArrays(GL_TRIANGLE_FAN, 0, n);
}

static struct {
    const char          *name;
    noctt_rule_func_t   rule;
} RULES[] = {
    {"press key to see more",    test},
    {"blowfish objs", blowfish_objs},
    {"tree" , tree_rule},
    {"modern", modern_rule},
    {"shapes",  shapes_rule},
    {"stencil", stencil_rule},
    {"colors", colors_rule},
    {"blowfish", blowfish_city_rule},
};

static const nb_rules = sizeof(RULES) / sizeof(RULES[0]);
static int rule_index = 0;
static noctt_prog_t *prog = NULL;
static GLFWwindow *window;

static void start_demo(int index)
{
    float mat[16] = {1, 0, 0, 0,
                     0, 1, 0, 0,
                     0, 0, 1, 0,
                     0, 0, 0, 1};
    int w, h;
    if (prog) noctt_prog_delete(prog);
    glfwGetWindowSize(window, &w, &h);
    mat_scale(mat, w, h, 1);
    prog = noctt_prog_create(RULES[index].rule, 256, 0, mat, 1);
    prog->render_callback = render_callback;
    prog->render_callback_data = NULL;
    glfwSetWindowTitle(window, RULES[index].name);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static void key_callback(GLFWwindow* window,
                         int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS) return;
    rule_index = (rule_index + 1) % nb_rules;
    start_demo(rule_index);
}

int main()
{
    int i;
    int r;
    int w = 640, h = 480;
    float proj_mat[16];

    glfwInit();

    glfwWindowHint(GLFW_SAMPLES, 2);
    window = glfwCreateWindow(w, h, "test", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glewInit();

    mat_ortho(proj_mat, -w / 2, +w / 2, -h / 2, h / 2, -1, +1);
    font_init(proj_mat);
    init_opengl(w, h);
    start_demo(0);

    while (!glfwWindowShouldClose(window)) {
        glUseProgram(gl_prog.prog);

        noctt_prog_iter(prog);
        font_flush();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    noctt_prog_delete(prog);
    glfwTerminate();
}
