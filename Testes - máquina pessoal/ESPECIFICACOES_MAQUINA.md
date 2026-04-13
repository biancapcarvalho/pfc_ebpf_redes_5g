## INFORMAÇOES DA MÁQUINA - BIANCA
Dados do sistema operacional:

```bash
bianca@bianca-Lenovo-IdeaPad-S145-15API:~$ hostnamectl
Static hostname: bianca-Lenovo-IdeaPad-S145-15API
Icon name: computer-laptop
Chassis: laptop
Machine ID: 5d8ae96f185a4aeca5d53dcb706bf8cb
Boot ID: 0a3bdec0b37d40cbaa93fae0a6924a4e
Operating System: Ubuntu 22.04.5 LTS
Kernel: Linux 6.8.0-106-generic
Architecture: x86-64
Hardware Vendor: Lenovo
Hardware Model: Lenovo IdeaPad S145-15API
```

Dados da rede:

```bash
bianca@bianca-Lenovo-IdeaPad-S145-15API:~$ ip -br link show
lo               UNKNOWN        00:00:00:00:00:00 <LOOPBACK,UP,LOWER_UP>
wlp2s0           UP             e4:aa:ea:d5:b9:2d <BROADCAST,MULTICAST,UP,LOWER_UP>

bianca@bianca-Lenovo-IdeaPad-S145-15API:~$ ethtool -i wlp2s0
driver: ath10k_pci
version: 6.8.0-106-generic
firmware-version: WLAN.TF.2.1-00021-QCARMSWP-1
expansion-rom-version:
bus-info: 0000:02:00.0
supports-statistics: yes
supports-test: no
supports-eeprom-access: no
supports-register-dump: no
supports-priv-flags: no
```

O driver de rede não suporta XDP nativo. Nesse caso, usei o modo genérico, que chama o programa em um nível mais alto da pilha. Para o projeto devo utilizar com Ethernet para usar o modo nativo e ter as vantagens do XDP.

A grande maioria dos drivers para rede sem fio não tem suporte ao XDP nativo. Isso porque o XDP não reconhece o formato 802.11, e é custoso para os desenvolvedores dos drivers fazerem a descodificação do 802.11 e depois converter para o 802.3. O xdp generico é executado depois de o kernel ter feito essa tradução, ou seja, depois de ele já ter alocado o buffer pro pacote
