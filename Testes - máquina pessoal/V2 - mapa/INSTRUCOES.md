## COMO RODAR O PROGRAMA USANDO O MAKEFILE

Dentro do makefile voce deve definir o nome dos arquivos

No inicio do Makefile
```Makefile
FILE_EBPF ?= xdp_program.c
FILE_USER?= ringbuf_reader.c
```

Definidos os valores voce deve compilar os programas:
```bash
make build
```

E depois executar o programa de monitoramento (o do espaço do usuario)
Para anexar
```bash
make load
```