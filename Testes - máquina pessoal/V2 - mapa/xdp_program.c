#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>
#include <linux/if_ether.h>
#include <bpf/bpf_endian.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/in.h>
#include <linux/tcp.h>
#include <linux/udp.h>

#include "map_struct.h"

// definir o mapa ring buffer
struct {
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 256 * 1024);
} eventos_ringbuf SEC(".maps");

SEC("xdp")
int extrair_metricas(struct xdp_md *ctx) {
    void *data_end = (void *)(long)ctx->data_end;
    void *data_start = (void *)(long)ctx->data;

    __u32 tamanho_pacote = data_end - data_start;

    struct ethhdr* eth = data_start;

    if ((void *)(eth+1) > data_end) return XDP_PASS;


    // pede espaço pro ring buffer, se não tiver o pacote passa
    // pacote_evento é a estrutura do ring buffer definida no common.h
    struct pacote_evento *evento = bpf_ringbuf_reserve(&eventos_ringbuf, sizeof(*evento), 0);
    if (!evento) return XDP_PASS;

    evento->tamanho_pacote = tamanho_pacote; // escreve o tamanho do pacote
    evento->timestamp_ns = bpf_ktime_get_ns(); // validar se não deveria ser capturado antes
    evento->tcp_flags = 0;

    if (eth->h_proto == bpf_htons(ETH_P_IP)) {

        struct iphdr* ip = (struct iphdr *)(eth+1);

        if ((void *)(ip+1) > data_end) {
            bpf_ringbuf_discard(evento, 0);
            return XDP_PASS;
        }

        evento->is_ipv6 = 0;
        evento->protocolo = ip->protocol;
        evento->ipv4_origem = ip->saddr;
        evento->ipv4_destino = ip->daddr;
            
        if (ip->protocol == IPPROTO_TCP) {
            struct tcphdr* tcp = (struct tcphdr *)(ip+1);

            if ((void *)(tcp+1) > data_end) {
                bpf_ringbuf_discard(evento, 0);
                return XDP_PASS;
            }

            evento->porta_origem = bpf_ntohs(tcp->source);
            evento->porta_destino = bpf_ntohs(tcp->dest);
            __u8 *tcp_bytes = (__u8 *)tcp; // ler as flags
            evento->tcp_flags = tcp_bytes[13];
        } else if (ip->protocol == IPPROTO_UDP) {
            struct udphdr* udp = (struct udphdr *)(ip+1);

            if ((void *)(udp+1) > data_end) {
                bpf_ringbuf_discard(evento, 0);
                return XDP_PASS;
            }

            evento->porta_origem = bpf_ntohs(udp->source);
            evento->porta_destino = bpf_ntohs(udp->dest);
        }
    } else if (eth->h_proto == bpf_htons(ETH_P_IPV6)) {
        struct ipv6hdr* ipv6 = (struct ipv6hdr *)(eth+1);

        if ((void *)(ipv6+1) > data_end) {
            bpf_ringbuf_discard(evento, 0);
            return XDP_PASS;
        }

        evento->is_ipv6 = 1;
        evento->protocolo = ipv6->nexthdr;

        // não entendi
        for (int i = 0; i < 16; i++) {
            evento->ipv6_origem[i] = ipv6->saddr.in6_u.u6_addr8[i];
            evento->ipv6_destino[i] = ipv6->daddr.in6_u.u6_addr8[i];
        }

        if (ipv6->nexthdr == IPPROTO_TCP) {
            struct tcphdr* tcp = (struct tcphdr *)(ipv6+1);

            if ((void *)(tcp+1) > data_end) {
                bpf_ringbuf_discard(evento, 0);
                return XDP_PASS;
            }

            evento->porta_origem = bpf_ntohs(tcp->source);
            evento->porta_destino = bpf_ntohs(tcp->dest);
            __u8 *tcp_bytes = (__u8 *)tcp; // ler as flags
            evento->tcp_flags = tcp_bytes[13];
        } else if (ipv6->nexthdr == IPPROTO_UDP) {
            struct udphdr* udp = (struct udphdr *)(ipv6+1);

            if ((void *)(udp+1) > data_end) {
                bpf_ringbuf_discard(evento, 0);
                return XDP_PASS;
            }

            evento->porta_origem = bpf_ntohs(udp->source);
            evento->porta_destino = bpf_ntohs(udp->dest);
        }
    }

    bpf_ringbuf_submit(evento, 0);

    return XDP_PASS; 
}

char LICENSE[] SEC("license") = "Dual BSD/GPL";