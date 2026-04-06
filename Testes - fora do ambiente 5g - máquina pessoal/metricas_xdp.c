#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>

#include <linux/if_ether.h>
#include <bpf/bpf_endian.h> // pro bpf_htons

#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/in.h> // para as constantes de protocolo

SEC("xdp") // indica o hook
int extrair_metricas(struct xdp_md *ctx) {
    // pela documentação, ponteiros de inicio e fim do pacote
    void *data_end = (void *)(long)ctx->data_end;
    void *data_start = (void *)(long)ctx->data;

    __u32 tamanho_pacote = data_end - data_start; // calcula o tamanho do pacote

    struct ethhdr* eth = data_start; // estrutura padrao do cabeçalho ethernet / camada de enlace

    if ((void *)(eth+1) > data_end) {
        // nao fazer nada se o cabeçalho for maior que o pacote
        // essa validaçao é necessária para nao acessar locais de memória além do pacote
        return XDP_PASS;
    }

    // o eth->h_proto é o campo que indica a proxima camada
    // h_proto é hardware protocol (EtherType)
    // ETH_P_IP 0x0800-> constante IP packet (IPv4)
    // ETH_P_IPV6 0x86DD-> constante IP packet (IPv6)

    if (eth->h_proto == bpf_htons(ETH_P_IP)) {
        // nao da pra comparar direto com ETH_P_IP por conta do tipo
        // usa o bpf_htons (host to network short) para converter pro formato de rede

        struct iphdr* ip = (struct iphdr *)(eth+1);

        if ((void *)(ip+1) > data_end) return XDP_PASS;

        if (ip->protocol == IPPROTO_TCP) {
            bpf_printk("\nPACOTE IPv4\nTAMANHO = %u bytes\nIP ORIGEM = %pI4\nIP DESTINO = %pI4\nPROTOCOLO TCP", tamanho_pacote, &ip->saddr, &ip->daddr);
        } else if (ip->protocol == IPPROTO_UDP) {
            bpf_printk("\nPACOTE IPv4\nTAMANHO = %u bytes\nIP ORIGEM = %pI4\nIP DESTINO = %pI4\nPROTOCOLO UDP", tamanho_pacote, &ip->saddr, &ip->daddr);
        }
    } else if (eth->h_proto == bpf_htons(ETH_P_IPV6)) {
        struct ipv6hdr* ipv6 = (struct ipv6hdr *)(eth+1);

        if ((void *)(ipv6+1) > data_end) return XDP_PASS;

        if (ipv6->nexthdr == IPPROTO_TCP) {
            bpf_printk("\nPACOTE IPv6\nTAMANHO = %u bytes\nIP ORIGEM = %pI4\nIP DESTINO = %pI4\nPROTOCOLO TCP", tamanho_pacote, &ipv6->saddr, &ipv6->daddr);
        } else if (ipv6->nexthdr == IPPROTO_UDP) {
            bpf_printk("\nPACOTE IPv6\nTAMANHO = %u bytes\nIP ORIGEM = %pI4\nIP DESTINO = %pI4\nPROTOCOLO UDP", tamanho_pacote, &ipv6->saddr, &ipv6->daddr);
        }
    }

    return XDP_PASS; // só envia o pacote
}

char LICENSE[] SEC("license") = "Dual BSD/GPL";