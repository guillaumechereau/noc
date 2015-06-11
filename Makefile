
all:
	gcc -o test_turtle \
	    tests/turtle.c tests/blowfish_city.c tests/tree.c \
	    tests/modern.c tests/blowfish_objs.c noc_turtle.c \
	    -O0 -fsanitize=address -g \
	    -I ./ -lglfw -lGLEW -lGL -lm -lasan
