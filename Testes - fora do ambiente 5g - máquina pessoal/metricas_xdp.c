#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>

#include <linux/if_ether.h>
#include <bpf/bpf_endian.h> // pro bpf_htons

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
        bpf_printk("PACOTE IPv4 - TAMANHO = %u bytes\n", tamanho_pacote);
    } else if (eth->h_proto == bpf_htons(ETH_P_IPV6)) {
        bpf_printk("PACOTE IPv6 - TAMANHO = %u bytes\n", tamanho_pacote);
    }

    return XDP_PASS; // só envia o pacote
}

char LICENSE[] SEC("license") = "Dual BSD/GPL";