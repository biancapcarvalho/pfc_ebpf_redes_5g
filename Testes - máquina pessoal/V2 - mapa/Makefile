FILE_EBPF ?= xdp_program.c
FILE_USER?= ringbuf_reader.c
OBJ_EBPF = $(FILE_EBPF:.c=.o)
PROG_USER = reader

all: build load

build_bpf:
	clang -O2 -g -Wall -target bpf -I/usr/include/x86_64-linux-gnu -c $(FILE_EBPF) -o $(OBJ_EBPF)

build_user:
	gcc -Wall -o $(PROG_USER) $(FILE_USER) -lbpf -lelf

build: build_bpf build_user

load:
	sudo ./$(PROG_USER)

clean:
	rm -f *.o $(PROG_USER)