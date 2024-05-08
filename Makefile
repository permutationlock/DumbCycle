CC ?= gcc
CFLAGS ?= -fno-stack-protector
LD ?= ld
LDFLAGS ?=
AS ?= as
ASFLAGS ?=

all: dumb_cycle

clean: clean_dumb_cycle

dumb_cycle: src/main.o src/mem.o src/runtime.o
	$(LD) $(LDFLAGS) -o dumb_cycle src/main.o src/mem.o src/runtime.o

clean_dumb_cycle: clean_main clean_mem clean_runtime
	rm -f dumb_cycle

src/mem.o: src/mem.c
	$(CC) $(CFLAGS) -c -o src/mem.o src/mem.c

clean_mem:
	rm -f src/mem.o

src/main.o: src/main.c
	$(CC) $(CFLAGS) -c -o src/main.o src/main.c

clean_main:
	rm -f src/main.o

src/runtime.o: src/runtime.s
	$(AS) $(ASFLAGS) -o src/runtime.o src/runtime.s

clean_runtime:
	rm -f src/runtime.o
