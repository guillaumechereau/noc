# For the moment I just use this simple Makefile.
# If you don't want to use libasan, just remove -lasan and -fsanitize=address

all: linear turtle

turtle:
	g++ -o test_turtle \
	    tests/turtle.c noc_turtle.c \
	    -Wall \
	    -O0 -fsanitize=address -g \
	    -I ./ -lglfw -lGLEW -lGL -lm -lasan

linear:
	g++ -o test_vec \
	    tests/vec.cpp \
	    -Wall \
	    -O0 -g \
	    -I ./ -lm
