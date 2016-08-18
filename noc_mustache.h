/* noc_mustache Mustache Templates library
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

/* Mustache is a simple logic-less template format
 *
 * This is a very crude implementation in plain C.  I am not trying to make
 * it fast, for example dicts are implemented as linked lists.
 * Also it doesn't support all the features of mustache.
 *
 * Usage:
 *
 * 0. define NOC_MUSTACHE_IMPLEMENTATION and include "noc_mustache.h"
 *    in your source file.
 * 1. Create a noc_mustache_t instance with `noc_mustache_create`.
 * 2. Fill the data using:
 *      `noc_mustache_add_str`
 *      `noc_mustache_add_list`
 *      `noc_mustache_add_dict`
 * 3. Render a template with `noc_template_render`.
 * 3. Free the data with `noc_template_free`.
 *
 * See tests/mustache.c for a concrete example.
 * 
 */

typedef struct noc_mustache noc_mustache_t;

// Context creation and destruction.
noc_mustache_t *noc_mustache_create(void);
void noc_mustache_free(noc_mustache_t *m);

/* Data generation
 *
 * All those functions take as first parameters a parent mustache object
 * and a key.  The key can be NULL if the parent is a list.
 */

// Create a new dictionary entry.
noc_mustache_t *noc_mustache_add_dict(noc_mustache_t *m, const char *key);
// Create a new list entry.
noc_mustache_t *noc_mustache_add_list(noc_mustache_t *m, const char *key);
// Create a new string entry.
void noc_mustache_add_str(noc_mustache_t *m, const char *key,
                          const char *fmt, ...);

// Render a template using a given data structure.
//  m       the data structure created with noc_mustache_create and
//          filled with the data.
//  templ   the mustache template.
//  out     buffer that will receive the generated text.  It can be set to
//          NULL if we just need to know the size of the text.
//  Return  the number of characters produced.
int noc_mustache_render(const noc_mustache_t *m, const char *templ,
                        char *out);

#ifdef NOC_MUSTACHE_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

enum {
    NOC_MUSTACHE_TYPE_DICT,
    NOC_MUSTACHE_TYPE_LIST,
    NOC_MUSTACHE_TYPE_STR,
};

struct noc_mustache
{
    char type;
    char *key;
    union {
        char *s;
    };
    noc_mustache_t *next, *prev, *children, *parent;
};

static noc_mustache_t *noc__mustache_create(
        noc_mustache_t *m, const char *key, int type)
{
    noc_mustache_t *ret = (noc_mustache_t*)calloc(1, sizeof(*ret));
    ret->parent = m;
    ret->key = key ? strdup(key) : NULL;
    ret->type = type;
    if (m) {
        if (m->children) {
            ret->prev = m->children->prev;
            m->children->prev->next = ret;
            m->children->prev = ret;
        } else {
            m->children = ret;
            ret->prev = ret;
        }
    }
    return ret;
}

noc_mustache_t *noc_mustache_create(void)
{
    noc_mustache_t *ret = (noc_mustache_t*)calloc(1, sizeof(*ret));
    ret->type = NOC_MUSTACHE_TYPE_DICT;
    return ret;
}

noc_mustache_t *noc_mustache_add_dict(noc_mustache_t *m, const char *key)
{
    return noc__mustache_create(m, key, NOC_MUSTACHE_TYPE_DICT);
}

noc_mustache_t *noc_mustache_add_list(noc_mustache_t *m, const char *key)
{
    return noc__mustache_create(m, key, NOC_MUSTACHE_TYPE_LIST);
}

void noc_mustache_add_str(
        noc_mustache_t *m, const char *key, const char *fmt, ...)
{
    va_list args;
    noc_mustache_t *ret;
    ret = noc__mustache_create(m, key, NOC_MUSTACHE_TYPE_STR);
    va_start(args, fmt);
    if (fmt) vasprintf(&ret->s, fmt, args);
    va_end(args);
}

void noc_mustache_free(noc_mustache_t *m)
{
    noc_mustache_t *c, *tmp;
    for (c = m->children; c && (tmp = c->next, 1); c = tmp)
        noc_mustache_free(c);
    if (m->type == NOC_MUSTACHE_TYPE_STR) free(m->s);
    free(m->key);
    free(m);
}

const noc_mustache_t *noc__mustache_get_elem(
        const noc_mustache_t *m, const char *key)
{
    noc_mustache_t *c;
    if (key[0] == '#') key++;
    for (c = m->children; c; c = c->next)
        if (c->key && strcmp(key, c->key) == 0) return c;
    return NULL;
}

static int noc__mustache_render(
        const noc_mustache_t *tree, const noc_mustache_t *m, char *out)
{
    int i = 0;
    noc_mustache_t tmp;
    const noc_mustache_t *c;
    const noc_mustache_t *elem = NULL;

    if (tree->key) elem = noc__mustache_get_elem(m, tree->key);

    // Some text.
    if (tree->type == NOC_MUSTACHE_TYPE_STR && !tree->key) {
        if (out)
            for (i = 0; i < (int)strlen(tree->s); i++) *out++ = tree->s[i];
        return strlen(tree->s);
    }

    if (tree->key && tree->key[0] == '/') return 0;

    // A list
    if (tree->key && tree->key[0] == '#') {
        if (!elem) return 0;
        tmp = *tree;
        tmp.type = NOC_MUSTACHE_TYPE_LIST;
        tmp.key = NULL;
        if (elem->type == NOC_MUSTACHE_TYPE_LIST) {
            for (c = elem->children; c; c = c->next) {
                i += noc__mustache_render(&tmp, c, out ? out + i : NULL);
            }
        }
        if (elem->type == NOC_MUSTACHE_TYPE_DICT) {
            i += noc__mustache_render(&tmp, elem, out);
        }
        return i;
    }

    // A variable
    if (tree->type == NOC_MUSTACHE_TYPE_STR && tree->key) {
        if (!elem) return 0;
        if (out)
            for (i = 0; i < (int)strlen(elem->s); i++) *out++ = elem->s[i];
        return strlen(elem->s);
    }

    if (tree->children) {
        for (c = tree->children; c; c = c->next) {
            i += noc__mustache_render(c, m, out ? out + i : NULL);
        }
        if (!tree->parent) {
            if (out) out[i] = '\0';
        }
        return i;
    }
    return 0;
}

static const char *noc__mustache_find_tag(const char *txt, int *size)
{
    char c;
    const char *ret = NULL;
    int state = 0;
    *size = 0;
    while ((c = *txt++)) {
        switch (state) {
        case 0: if (c == '{') state++; break;
        case 1:
            if (c == '{') {
                state++;
                ret = txt;
            }
            break;
        case 2:
            if (c == ' ') break;;
            if (    (c >= 'a' && c <= 'z') ||
                    (c >= 'A' && c <= 'Z') ||
                    (c >= '0' && c <= '9') ||
                    (c == '_' || c == '#' || c == '/')) {
                (*size)++;
            } else if (c == '}') {
                state++;
            } else {
                *size = 0;
                ret = NULL;
                state = 0;
            }
            break;
        case 3:
            if (c == '}') return ret;
            *size = 0;
            ret = NULL;
            break;
        default:
            break;
        }
    }
    return NULL;
}

int noc_mustache_render(
        const noc_mustache_t *m, const char *templ, char *out)
{
    char key[128];
    const char *tag;
    int r, len;
    noc_mustache_t *tree;

    // Create the template tree.
    tree = noc_mustache_add_list(NULL, NULL);
    while (templ) {
        tag = noc__mustache_find_tag(templ, &len);
        if (!tag) {
            noc_mustache_add_str(tree, NULL, templ);
            break;
        }
        if (tag > templ)
            noc_mustache_add_str(tree, NULL, "%.*s", tag - 2 - templ, templ);
        strncpy(key, tag, len);
        key[len] = '\0';
        noc_mustache_add_str(tree, key, NULL);
        if (key[0] == '/') {
            asprintf(&tree->s, "%.*s", (int)(templ - tree->s), tree->s);
            tree = tree->parent;
        }
        templ = tag + len + 2;
        if (key[0] == '#') {
            tree = tree->children->prev;
            tree->s = (char*)templ;
        }
    }
    r = noc__mustache_render(tree, m, out);
    noc_mustache_free(tree);
    return r;
}

#endif
