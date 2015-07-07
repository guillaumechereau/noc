/* noc_vec.h test code.
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

#include "noc_vec.h"
#include <stdio.h>
#include <assert.h>

int main()
{
    vec2_t a2, b2, c2;
    vec3_t a3, b3, c3;
    vec4_t a4;
    float n;
    a2 = vec2(1, 2);
    b2 = vec2(3, 4);
    a3 = vec3(1, 2, 3);
    b3 = vec3(4, 5, 6);

    // Create a vector.
    a4 = vec4_zero;
    a4 = vec4(1, 2, 3, 4);
    // Access members.
    a4.x   = 0;
    a4.xy  = vec2(1, 2);
    a4.xyz = vec3(1, 2, 3);
    printf("%g, %g, %g, %g\n", VEC4_SPLIT(a4));
    // Add vectors.
    a4 = vec4_add(vec4(1, 2, 3, 4), vec4(10, 20, 30, 40));
    // in place add.
    vec4_iadd(&a4, vec4(100, 100, 100, 100));
    // Other operations.
    vec4_iaddk(&a4, vec4(4, 6, 8, 10), 0.5);
    vec4_imul(&a4, 10);

    // Cross product.
    c3 = vec3_cross(a3, b3);
    printf("(%g, %g, %g) x (%g, %g, %g) = (%g, %g, %g)\n",
            VEC3_SPLIT(a3), VEC3_SPLIT(b3), VEC3_SPLIT(c3));
    // Norm.
    n = vec3_norm(a3);
    printf("norm(%g, %g, %g) = %g\n", VEC3_SPLIT(a3), n);

    // Dot product.
    n = vec3_dot(a3, b3);
    printf("(%g, %g, %g) . (%g, %g, %g) = %g\n",
            VEC3_SPLIT(a3), VEC3_SPLIT(b3), n);

    // lerp.
    c3 = vec3_lerp(a3, b3, 0.1);
    printf("lerp((%g, %g, %g), (%g, %g, %g), 0.1) = (%g, %g, %g)\n",
            VEC3_SPLIT(a3), VEC3_SPLIT(b3), VEC3_SPLIT(c3));

    // 2d rotation.
    c2 = vec2_rot(a2, M_PI / 8);
    printf("rot((%g, %g), pi/8) = (%g, %g)\n",
            VEC2_SPLIT(a2), VEC2_SPLIT(c2));
    // 90 deg rotations.
    c2 = vec2_perp(a2);
    printf("perp(%g, %g) = (%g, %g)\n", VEC2_SPLIT(a2), VEC2_SPLIT(c2));

    // Matrix.
    mat4_t mat4 = mat4_translate(mat4_identity, 1, 0, 0);
    mat4_itranslate(&mat4, 10, 10, 10);
    mat4_iscale(&mat4, 1, 1, 2);
    a4 = mat4_mul_vec(mat4, a4);
    printf("%g, %g, %g, %g\n", a4.x, a4.y, a4.z, a4.w);

    // Quaternion.
    quat_t quat = quat_from_axis(M_PI / 4, 0, 0, 1);
    c3 = quat_mul_vec3(quat, a3);


    // c++ operators.
    c3 = a3 + b3 * 2;
    c3 += a3;
    c3 = -a3;
    n = a3 * b3;
    assert(n == vec3_dot(a3, b3));
    assert(a3 == a3);
    n = a2 ^ b2;
    assert(n == vec2_cross(a2, b2));
    c3 = a3 ^ b3;
    assert(c3 == vec3_cross(a3, b3));
    a4 = a4 * mat4;
    a4 = a4 * quat;
}
