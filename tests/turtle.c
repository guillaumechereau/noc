#include "noc_turtle.h"
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

static const int W = 640;
static const int H = 480;

typedef struct {
    GLuint prog;
    GLuint u_proj_l;
    GLuint u_color_l;
    GLuint a_pos_l;
} gl_prog_t;

static gl_prog_t gl_prog;

static T_RULE(spiral_node)
{
    T_START
    T_SQUARE();
    T_TR(HUE, pm(0, 5));
    T_YIELD;
    if (brand(0.01)) {
        T_TR(FLIP, 0);
        T_SPAWN(spiral_node, R, -90);
    }
    T_SPAWN(spiral_node, X, 0.4, R, 3, X, 0.4, S, 0.99);
    T_END
}

T_RULE(spiral)
{
    T_START
    T_TR(HSL, 1, 0, 1, 0.5);
    T_CALL(spiral_node);
    T_CALL(spiral_node, FLIP, 90);
    T_END
}

static T_RULE(test)
{
    T_START

    T_SQUARE(S, 0.9, LIGHT, 0.2);
    T_SQUARE(S, 0.9, G, -1, LIGHT, 0.1);
    T_TR(SN, LIGHT, 1);

    T_TRANSFORM(X, -0.25, 0.25, S, 0.5) {
        T_FOR(10, S, 0.8, LIGHT, -0.2) {
            T_RSQUARE(60, S, 0.5, SAT, 1);
        }
    }

    T_SQUARE(S, 0.1);
    T_SQUARE(S, 0.1, X, 2);
    T_SQUARE(S, 0.1, X, 4, R, 45, LIGHT, -0.5);
    T_SQUARE(S, 0.1, X, 6, R, 45, LIGHT, -0.5, SAT, 1, HUE, 180);
    T_CIRCLE(S, 0.1, Y, 2);
    T_TRIANGLE(S, 0.1, Y, 4);
    T_CALL(spiral, Y, -0.5, S, 0.02);
    T_END
}

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
}

static void render_callback(int n, float (*poly)[3], float color[4],
                            void *user_data)
{
    assert(sizeof(float) == sizeof(GL_FLOAT));
    glUniform4fv(gl_prog.u_color_l, 1, color);
    glVertexAttribPointer(gl_prog.a_pos_l, 3, GL_FLOAT, false, 0, (void*)poly);
    glDrawArrays(GL_TRIANGLE_FAN, 0, n);
}


int main()
{
    int i;
    int r;
    noctt_prog_t *prog;
    float mat[16] = {1, 0, 0, 0,
                     0, 1, 0, 0,
                     0, 0, 1, 0,
                     0, 0, 0, 1};

    GLFWwindow *window;
    glfwInit();

    glfwWindowHint(GLFW_SAMPLES, 2);
    window = glfwCreateWindow(W, H, "test", NULL, NULL);
    glfwMakeContextCurrent(window);
    glewInit();

    init_opengl(W, H);

    mat_scale(mat, W, H, 1);

    prog = noctt_prog_create(test, 256, 0, mat);
    prog->render_callback = render_callback;
    prog->render_callback_data = NULL;

    while (!glfwWindowShouldClose(window)) {
        noctt_prog_iter(prog);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    noctt_prog_delete(prog);
    glfwTerminate();
}
