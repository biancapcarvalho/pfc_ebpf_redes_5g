#ifndef __MAP_STRUCT_H
#define __MAP_STRUCT_H

// estrutura pro ring buffer
struct pacote_evento {
    unsigned int tamanho_pacote;
    unsigned char is_ipv6;
    unsigned char protocolo;  // TCP -> 6, UDP -> 17
    unsigned short porta_origem;
    unsigned short porta_destino;
    unsigned long long timestamp_ns;
    unsigned char tcp_flags;

    union {
        unsigned int ipv4_origem;
        unsigned char ipv6_origem[16];
    };
    union {
        unsigned int ipv4_destino;
        unsigned char ipv6_destino[16];
    };
};

#endif