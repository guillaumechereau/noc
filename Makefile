# For the moment I just use this simple Makefile.
# If you don't want to use libasan, just remove -lasan and -fsanitize=address

all: packer

mustache:
	g++ -o test_mustache \
	    tests/mustache.c \
	    -Wall \
	    -O0 -fsanitize=address -g \
	    -I ./

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

file_dialog:
	g++ -o test_file_dialog \
	    tests/file_dialog.c \
	    `pkg-config --cflags --libs gtk+-2.0` \
	    -Wall \
	    -O0 \
	    -I ./

packer:
	g++ -o test_packer tests/packer.c \
	    -Wall -O0 -fsanitize=address -g -I ./ -lz
