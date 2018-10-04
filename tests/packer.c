/* noc packer test code.
 *
 * Copyright (c) 2018 Guillaume Chereau <guillaume@noctua-software.com>
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

#define NOC_PACKER_IMPLEMENTATION

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "noc_packer.h"

int main()
{
    int i, size;
    struct {
        float x;
        float y;
        char  s[8];
    } data[1024] = {}, *data2;
    char *buf;

    const struct noc_packer_column cols[] = {
        {"x", 'f', 0, 4},
        {"y", 'f', 4, 4, .precision=16},
        {"s", 's', 8, 8},
        {}
    };

    // Create some data.
    for (i = 0; i < 1024; i++) {
        data[i].x = i;
        data[i].y = 1.0 / (i + 1);
        sprintf(data[i].s, "%d", i);
    }

    // Compress it.
    size = noc_packer_compress(
            (char*)data, sizeof(data), sizeof(data[0]), cols, &buf);
    printf("size: %d -> %d\n", (int)sizeof(data), size);

    // Uncompress it.
    size = noc_packer_uncompress(buf, size, sizeof(data[0]),
                                 cols, (char**)&data2);
    assert(size == 1024 * sizeof(data[0]));
    // Check that the data is correct.
    for (i = 0; i < 1024; i++) {
        assert(data2[i].x == data[i].x);
        assert(fabs(data2[i].y - data[i].y) < 0.00001);
        assert(strcmp(data2[i].s, data[i].s) == 0);
    }

    free(buf);
    free(data2);

    return 0;
}
