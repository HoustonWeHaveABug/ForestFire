FORESTFIRE_C_FLAGS=-c -O2 -Wall -Wextra -Waggregate-return -Wcast-align -Wcast-qual -Wconversion -Wformat=2 -Winline -Wlong-long -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Wno-import -Wpointer-arith -Wredundant-decls -Wreturn-type -Wshadow -Wstrict-prototypes -Wswitch -Wwrite-strings

forestfire.exe: forestfire.o
	gcc -o forestfire.exe forestfire.o -lm

forestfire.o: forestfire.c forestfire.make
	gcc ${FORESTFIRE_C_FLAGS} -o forestfire.o forestfire.c
