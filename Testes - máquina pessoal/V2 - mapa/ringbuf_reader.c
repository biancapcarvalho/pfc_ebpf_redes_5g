#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <bpf/libbpf.h>
#include <net/if.h>
#include "map_struct.h"

static volatile bool exiting = false;

// gemini - sair com ctrl C
static void sig_handler(int sig) {
    exiting = true;
}

// executa toda vez que o programa ebpf escrever no mapa ring buffer
static int tratar_evento(void *ctx, void *data, size_t data_sz) {
    const struct pacote_evento *evento = data;
    char ip_origem[INET6_ADDRSTRLEN];
    char ip_destino[INET6_ADDRSTRLEN];
    const char *protocolo_str = (evento->protocolo == 6) ? "TCP" : (evento->protocolo == 17) ? "UDP" : "OUTRO";

    // traduz as flags TCP
    char flags_str[8] = "-------";
    if (evento->protocolo == 6) { // 6 é TCP
        if (evento->tcp_flags & (1 << 0)) flags_str[6] = 'F'; // FIN
        if (evento->tcp_flags & (1 << 1)) flags_str[5] = 'S'; // SYN
        if (evento->tcp_flags & (1 << 2)) flags_str[4] = 'R'; // RST
        if (evento->tcp_flags & (1 << 3)) flags_str[3] = 'P'; // PSH
        if (evento->tcp_flags & (1 << 4)) flags_str[2] = 'A'; // ACK
        if (evento->tcp_flags & (1 << 5)) flags_str[1] = 'U'; // URG
    }

    double tempo_s = (double)evento->timestamp_ns / 1000000000.0; // ns para s

    // converte os IPs (binario -> string)
    if (evento->is_ipv6) {
        inet_ntop(AF_INET6, evento->ipv6_origem, ip_origem, sizeof(ip_origem));
        inet_ntop(AF_INET6, evento->ipv6_destino, ip_destino, sizeof(ip_destino));
        printf("[%.4f] [IPv6] [%s]:%u -> [%s]:%u | %s [%s] | %u bytes\n", 
               tempo_s, ip_origem, evento->porta_origem, ip_destino, evento->porta_destino, protocolo_str, flags_str, evento->tamanho_pacote);
    } else {
        inet_ntop(AF_INET, &evento->ipv4_origem, ip_origem, sizeof(ip_origem));
        inet_ntop(AF_INET, &evento->ipv4_destino, ip_destino, sizeof(ip_destino));
        printf("[%.4f] [IPv4] %s:%u -> %s:%u | %s [%s] | %u bytes\n", 
               tempo_s, ip_origem, evento->porta_origem, ip_destino, evento->porta_destino, protocolo_str, flags_str, evento->tamanho_pacote);
    }
    return 0;
}

int main(int argc, char **argv) {
    struct bpf_object *obj;
    struct ring_buffer *rb = NULL;
    int map_fd;
    int ifindex;

    const char *iface = "wlp2s0"; 
    ifindex = if_nametoindex(iface);
    if (!ifindex) {
        fprintf(stderr, "Falha ao encontrar interface %s\n", iface);
        return 1;
    }

    obj = bpf_object__open_file("xdp_program.o", NULL);
    if (libbpf_get_error(obj)) return 1;
    if (bpf_object__load(obj)) return 1;

    struct bpf_program *prog = bpf_object__find_program_by_name(obj, "extrair_metricas");
    struct bpf_link *link = bpf_program__attach_xdp(prog, ifindex);
    if (libbpf_get_error(link)) {
        fprintf(stderr, "Falha ao anexar XDP. Tente rodar com sudo.\n");
        return 1;
    }

    map_fd = bpf_object__find_map_fd_by_name(obj, "eventos_ringbuf");
    rb = ring_buffer__new(map_fd, tratar_evento, NULL, NULL);
    if (!rb) return 1;

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    printf("Monitorando pacotes na interface %s...\n", iface);

    while (!exiting) {
        ring_buffer__poll(rb, 100 /* timeout de 100ms */);
    }

    ring_buffer__free(rb);
    bpf_link__destroy(link);
    bpf_object__close(obj);
    printf("\nMonitoramento encerrado.\n");
    return 0;
}