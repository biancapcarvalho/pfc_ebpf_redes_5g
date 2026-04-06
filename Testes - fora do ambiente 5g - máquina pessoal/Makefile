# Valores padrão caso não passe os argumentos no terminal
FILE ?= metricas_xdp.c
IFACE ?= wlp2s0
MODE ?= xdpgeneric

# Converte o nome do arquivo .c para .o
OBJ = $(FILE:.c=.o)

# O alvo principal que chama todos os passos
load: build attach show trace

build:
	clang -O2 -g -Wall -target bpf -I/usr/include/x86_64-linux-gnu -c $(FILE) -o $(OBJ)

attach:
	sudo ip link set dev $(IFACE) $(MODE) obj $(OBJ) sec xdp

show:
	ip link show dev $(IFACE)

trace:
	sudo cat /sys/kernel/debug/tracing/trace_pipe

# Alvo utilitário para limpar e desanexar o programa
clean:
	sudo ip link set dev $(IFACE) $(MODE) off
	rm -f *.o