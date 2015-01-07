.PHONY: all clean

CC = gcc -std=gnu99
CFLAGS = $(OPT) $(DEBUG) $(WARN)
OPT = -O2
DEBUG = -g
WARN = -Wall
LDLIBS = -lelf

all: nrpages

cpuid: nrpages.c

clean:
	-rm -f nrpages
