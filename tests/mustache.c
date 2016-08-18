/* noc mustache test code.
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

#define NOC_MUSTACHE_IMPLEMENTATION

#include "noc_mustache.h"

static const struct {
    const char *name;
    int         age;
} PEOPLES[] = {
    {"Guillaume", 32},
    {"Chiling", 27},
};

int main()
{
    const char *templ;
    char out[1024];
    int i;
    noc_mustache_t *m, *m_people, *m_person;
    m = noc_mustache_create();
    noc_mustache_add_str(m, "x", "%d", 10);

    templ = "Hello {{x}}";
    noc_mustache_render(m, templ, out);
    printf("%s\n", out);

    m_people = noc_mustache_add_list(m, "people");
    for (i = 0; i < (int)(sizeof(PEOPLES) / sizeof(PEOPLES[0])); i++) {
        m_person = noc_mustache_add_dict(m_people, NULL);
        noc_mustache_add_str(m_person, "name", PEOPLES[i].name);
        noc_mustache_add_str(m_person, "age", "%d", PEOPLES[i].age);
    }

    templ = "{{#people}}name:{{name}}, age:{{age}}\n{{/people}}";
    noc_mustache_render(m, templ, out);
    printf("\n%s\n", out);

    noc_mustache_free(m);
    return 0;
}
