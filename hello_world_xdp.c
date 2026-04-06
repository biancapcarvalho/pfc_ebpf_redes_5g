#include <linux/bpf.h> // header do bpf linux
#include <bpf/bpf_helpers.h> // header do libbpf com as funções auxiliares

SEC("xdp") // indica o hook
int hello_world_xdp(struct xdp_md *ctx) {
    bpf_printk("Hello World XDP!\n"); // escreve no buffer trace_pipe
    return XDP_PASS; // só envia o pacote
}

char LICENSE[] SEC("license") = "Dual BSD/GPL";