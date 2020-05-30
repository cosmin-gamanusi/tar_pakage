# compiler setup
CC=gcc
CFLAGS=-Wall -Wextra -std=c99

# define targets
TARGETS=archiver

build: $(TARGETS)

archiver: archiver.c
	$(CC) $(CFLAGS) archiver.c -o archiver

pack:
	zip -FSr 3XYCA_FirstnameLastname_Tema3.zip README Makefile *.c *.h

clean:
	rm -f $(TARGETS)
