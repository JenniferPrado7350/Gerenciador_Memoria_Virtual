# Programa de Gerenciamento de Memória Virtual

## Este projeto foi implemetado para a disciplina de Sistemas Operacionais, consiste em traduzir endereços lógicos para endereços físicos, usando um TLB, uma tabela de páginas e o algoritmo FIFO


## Instruções para execução:

Para gerar o executável digite no terminal:

    make

Em seguida, para executar digite:

    ./GerenciadorMemoriaVirtual address-exemplo.txt <quadro> <algoritmo>

sendo:

    quadro = 128 ou 256
    algoritmo = FIFO ou LRU

Caso queira. também pode escolher outro .txt de entrada de endereços
