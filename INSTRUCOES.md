## Compilar
clang -target bpf -O2 -c <nome_arquivo.c> -o <nome_arquivo.o>

#### clang é o compilador
#### -target bpf indica a estrutura de destino
#### -O2 define o nível de otimizaçao do compilador. Necessário por conta do verificador eBPF
#### -c informa que é somente compilaçao, para nao linkar

## OU Compilar
clang -O2 -g -Wall -target bpf -I/usr/include/x86_64-linux-gnu -c <nome_arquivo.c> -o <nome_arquivo.o>

#### -g permite o CO-RE
#### -Wall habilita todos os warnings do compilados
#### -I/usr/include/x86_64-linux-gnu informa o caminho para encontrar os includes, pois o programa vai rodar na maquina virtual do eBPF e pode nao encontrar os arquivos

## Anexar
ip -force link set dev [DEV] xdp obj <nome_arquivo.o> sec xdp

#### ip é a ferramenta de gerenciamento de rede do linux
#### link indica que quer operar na camada de link
#### set dev [DEV] configura o dispositivo de rede (no meu caso wlp2s0, deve ser colocado no lugar de [DEV])
#### xdpgeneric indica o hook que será utilizado. Como a conexao é wifi, deve usar o xdpgeneric (o ideal com Ethernet seria somente xdp)
#### obj <nome_arquivo.o> indica o arquivo com o programa que será anexado
#### sec xdp o clang divide o arquivo compilado em seçoes, esse comando indica a seçao xdp

## OU Anexar
ip link set dev wlp2s0 xdpgeneric obj <nome_arquivo.o> sec xdp
sudo ip link set dev wlp2s0 xdpgeneric obj metricas_xdp.o sec xdp


## Ver se foi anexado
ip link show dev wlp2s0

## Desanexar
ip link set dev wlp2s0 xdpgeneric off

## Monitorar o trace_pipe
sudo cat /sys/kernel/debug/tracing/trace_pipe