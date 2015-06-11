noc
===

A list of public domain libraries for C/C++, inspired by the stb libraries.

For the moment I just put one library: noc_turtle, that allows to create
procedural graphics using a syntax close to the one of [Context Free], but
directly in C code.  This is the code I used in my video game [Blowfish
Rescue].

To use the code, put noc_turtle.h and noc_turtle.c in your project file.  See
noc_turtle.h for some documentation about the usage.

To compile the demo, type 'make'.  You need to have OpenGL, GLFW, and GLEW.

[Context Free]: http://www.contextfreeart.org/
[Blowfish Rescue]: http://noctua-software.com/blowfish-rescue


FAQ
---

#### Does noc_turtle support all the features of Context Free?

Probably not.  I am not familiar enough with Context Free to tell.  I tried
my best to keep the syntax as close as possible. Since noc_turtle is not
interpreted but directly compiled by the C or C++ compiler, I had to change
the syntax to be compatible with C.  Where in Context Free you would write
this:

    shape blah {
        SQUARE [s 10 y 5]
    }

In noc_turtle you write:

    void blah(noctt_turtle_t *turtle) {
        START
        SQUARE(S, 10, Y, 5);
        END
    }

So it is a bit more verbose, I am afraid there is no way around it.

Also some of the adjustment functions have different names (for example A
instead of alpha).


#### What is the licence?

All the code is released under permissive free MIT licence.  This means it's
OK to use the code in your commercial project.


#### Why not making a single file library like stb?

I might do that at some point.  For the moment I keep the header and code
separated into two files.


#### Why is the code of noc_turtle full of macros?

This is because the code is using a C trick to make state machine look like
linear code.  For a good introduction to this, you can check this article from
Simon Tatham: http://www.chiark.greenend.org.uk/~sgtatham/coroutines.html


#### Does it work under Windows?

I cannot guaranty that.  I usually work on a unix environment.  The library
was originally created for a video game targeting only Android and iOS.
