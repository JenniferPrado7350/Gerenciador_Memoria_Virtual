#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


/** Sistema para gerenciar memória virtual
 *
 * Esse programa tem como objetivo traduzir dos lógicos para endereços físicos
 * para um espaço de endereço virtual de tamanho 2 16 = 65.536 bytes. Ele lê
 * o arquivo address.txt que tem varios endereços lógicos e, utilizando um TLB e uma
 * uma tabela de páginas, ele vai traduzir cada um dos endereços lógicos para seu endereço
 * físico correspondente, e também o valor do byte armazenado no físico traduzido 
 */


/** Criando a struct Par para construirmos a TLB
 */
typedef struct {
    uint32_t pagina, quadro;
    uint8_t valido;
} Par;


/** Definindo macros
 */
#define bitsPagina 8
#define bitsQuadro 7
#define bitsOffset 8
#define tamanhoTLB 16
#define tamanhoPagina (1 << bitsOffset)
#define tamanhoQuadro tamanhoPagina
#define numeroDePaginas (1 << bitsPagina)
#define numeroDeQuadros (1 << bitsQuadro)
#define tamanhoMemoria (tamanhoQuadro * numeroDeQuadros)
#define ArquivoBackingStorage "BACKING_STORE.bin"


/** Definindo funções macros
 */
#define retornaEnderecoFisico(quadro, offset) ((numeroDoQuadro << bitsOffset) | offset)
#define retornaNumeroPagina(endereco) ((endereco & ((1 << (bitsOffset + bitsPagina))- (1 << bitsOffset))) >> bitsOffset)
#define retornaOffset(endereco) (endereco & ((1 << bitsOffset) - 1))


/** Criando a TLB
 */
Par TLB[tamanhoTLB];


/** Criando vetores para a Memoria, Tabela de Paginas e Paginas Validas
 */
int8_t memoria[tamanhoMemoria];
uint32_t tabelaPaginas[numeroDePaginas], proximoQuadroLivre, proximaTLBLivre;
uint8_t paginasValidas[numeroDePaginas];


/** Criando as variaveis que irao guardar os arquivos de entrada backing storage e address
 */
FILE *arquivoEnderecos, *backing_storage;


/** Criando variaveis auxiliares
*/
uint32_t total_cnt, falhaPaginaCNT, perdaTLB_CNT;


/** pegaQuadroEscolhido e uma funcao que vai implementar o algoritmo FIFO
 */
uint32_t pegaQuadroEscolhido() {
    if(proximoQuadroLivre < numeroDeQuadros) 
        return proximoQuadroLivre++;
    
    uint32_t escolhido = (proximoQuadroLivre++) % numeroDeQuadros;

    for(size_t i = 0; i != numeroDePaginas; ++i){
        if(paginasValidas[i] && tabelaPaginas[i] == escolhido) {
            paginasValidas[i] = 0;
            break;

        }
    }

    return escolhido;
}


/** trataFalhaDePagina e uma funcao para lidar com as falhas que ocorrerao nas paginas, ela chama a funcao
 * pegaQuadroEscolhido para salvar o escolhido na tabela de paginas e no mesmo indice que na tabela de paginas, no vetor de paginas
 * validas ele seta com 1, representacao de boolean true, significa que a pagina é valida
 */
void trataFalhaDePagina(uint32_t numeroDaPagina) {

    tabelaPaginas[numeroDaPagina] = pegaQuadroEscolhido();

    fseek(backing_storage, numeroDaPagina * tamanhoPagina, SEEK_SET);

    fread(memoria + tabelaPaginas[numeroDaPagina] * tamanhoPagina, sizeof(int8_t), tamanhoPagina, backing_storage);

    paginasValidas[numeroDaPagina] = 1;

    ++falhaPaginaCNT;

}


/** esta funcao percorre toda a TLB e é responsavel por verificar na TLB se pelo menos um elemento é valido e se a pagina dele 
 * é igual ao numero da pagina indicado no parametro da funcao
 */
int verificaTLB(uint32_t numeroDaPagina, uint32_t *numeroDoQuadro) {

    for(size_t i = 0; i != tamanhoTLB; ++i) {
        if(TLB[i].valido && TLB[i].pagina == numeroDaPagina) {
            *numeroDoQuadro = TLB[i].quadro;
            return 1;

        }
    }

    return 0;
}


/** esta funcao é responsavel por atualizar a TLB, ela usa o algoritmo FIFO para isso
 * é igual ao numero da pagina indicado no parametro da funcao
 */
void atualizaTLB(uint32_t numeroDaPagina, uint32_t numeroDoQuadro) {
    size_t escolhido = proximaTLBLivre % tamanhoTLB;

    proximaTLBLivre = (proximaTLBLivre + 1) % tamanhoTLB;
    
    TLB[escolhido].valido = 1;
    TLB[escolhido].pagina = numeroDaPagina, TLB[escolhido].quadro = numeroDoQuadro;
}


/** esta funcao é responsavel por realizar o comando -1, ou seja, ela imprime o conteúdo da TLB
 */
void imprimeTLB() {
    
    for(int i=0; i < tamanhoTLB; i++){
        printf("TLB[%d] ->", i);
        printf("\tvalido: %d", TLB[i].valido);
        printf("\tpagina: %d", TLB[i].pagina);
        printf("\tquadro: %d\n", TLB[i].quadro);
    }

}

/** esta funcao é responsavel por fazer a traducao do endereco Logico para o endereco Fisico
 * ela incrementa total_cnt, pega o numero da pagina chamando a funcao retornaNumeroPagina,
 * pegar o offset com a funcao retornaOffset, verifica se nao ha nenhum elemento na TLB que seja 
 * valido e que tenha o numero da pagina indicado para incrementar a perda da TLB e lidar com a 
 * falha na pagina chamando a funcao trataFalhaDePagina, apos isso a TLB é atualizada e a funcao 
 * em questao retorna o endereco fisico encontrado
 */
uint32_t traduzirEndereco(uint32_t logica) {
    ++total_cnt;

    uint32_t numeroDaPagina, offset, numeroDoQuadro;

    numeroDaPagina = retornaNumeroPagina(logica);
    offset = retornaOffset(logica);

    if(!verificaTLB(numeroDaPagina, &numeroDoQuadro)) {   
        ++perdaTLB_CNT;

        if(paginasValidas[numeroDaPagina] == 0) 
            trataFalhaDePagina(numeroDaPagina);
        
        numeroDoQuadro = tabelaPaginas[numeroDaPagina];
        atualizaTLB(numeroDaPagina, numeroDoQuadro);

    }

    return retornaEnderecoFisico(numeroDoQuadro, offset);
}

/** esta funcao é responsavel por realizar o comando -2 e -3 escrevendo as entradas da tabela de páginas, ou seja:
 * caso o comando seja -2, vai escrever somente páginas com bit de presença ZERO => páginas não carregadas na memória principal
 * caso o comando seja -3, vai escrever somente páginas com bit de presença UM => páginas carregas na memória principal
 */
void imprimeTabelaDePaginas(int bit){
    
    for(int i=0; i < numeroDePaginas; i++){
        if((paginasValidas[i] == 0 && bit == 0) || (paginasValidas[i] == 1 && bit == 1))
            printf("TabelaDePagina[%d]:\tPagina %d\n", i, tabelaPaginas[i]); 
    }
}

/** esta funcao é responsavel por retornar o valor do endereco fisico indicado na memoria
 */
char acessarMemoriaFisica(uint32_t fisica) {
    return memoria[fisica];

}


/** esta funcao é responsavel por iniciar os arquivos utilizados na execucao do sistema, verificar qual algoritmo foi escolhido,
 * receber valor do quadro e iniciar o vetor paginasValidas com zero assim como proximoQuadroLivre e proximaTLBLivre
 */
int iniciarArquivos(int argc, char **argv) {

    backing_storage = fopen(ArquivoBackingStorage, "rb");

    if(backing_storage == NULL) {
        printf("Não é possível abrir o arquivo BackingStorage: %s\n", ArquivoBackingStorage);
        return -1;

    }


    arquivoEnderecos = fopen(argv[1], "r");

    if(arquivoEnderecos == NULL) {
        printf("Não é possível abrir o arquivo address: %s\n", argv[1]);
        return -2;

    }

    if(argv[2] == NULL || argv[3] == NULL){
        printf("\nErro!!! Quadro não informado ou Algoritmo não escolhido!\n");
        return -3;
    }

    if(strcmp(argv[3],"FIFO") == 0) 
        printf("\n\t\t\tAlgoritmo FIFO\n\n\n");
    else if(strcmp(argv[3],"LRU") == 0)
        printf("\n\t\t\tAlgoritmo LRU\n\n\n");
    else{
        printf("\nErro!!! Algoritmo Inválido\n");
        return -4;
    }

    memset(paginasValidas, 0, sizeof(uint8_t) * numeroDePaginas);
    proximoQuadroLivre = proximaTLBLivre = 0;

    return 0;
}


/** esta funcao é responsavel por fechar os arquivos que foram abertos para consulta
 */
void fecharArquivos() {
    if(arquivoEnderecos) 
        fclose(arquivoEnderecos);

    if(backing_storage) 
        fclose(backing_storage);

}


/** esta funcao é responsavel por imprimir os resultados finais calculados
 */
void Resultados() {
    printf("Porcentagem de falha de página: %.2f%%\n", (float)falhaPaginaCNT / total_cnt * 100);
    printf("Porcentagem de sucesso na TLB: %.2f%%\n", (float)(total_cnt - perdaTLB_CNT) / total_cnt * 100);
}

/** esta funcao é a funcao principal, caso haja os comandos negativos no arquivo address de entrada,
 * vai imprimir TLB se -1 e tabela de paginas se -2 ou -3, caso nao haja, vai chamar a funcao traduzirEndereco para iniciar a traducao do 
 * endereco logico para fisico, depois chama a acessarMemoriaFisica para pegar o valor na memoria do 
 * endereco fisico encontrado, a Resultados para mostrar os resultados encontrados e por fim a fecharArquivos 
 * para fechar os arquivos de consulta
 */
int main(int argc, char **argv) {
    char line[8];
    int valorEntrada;

    if(iniciarArquivos(argc, argv) != 0) {
        fecharArquivos();
        return 0;

    }

    while(fgets(line, 8, arquivoEnderecos)) {
        uint32_t logica, fisica;
        int8_t valor;

        if(line[0] != '\n'){
            printf("---------------------------------------------------------------------\n");
            
            sscanf(line, "%d", &valorEntrada);
            sscanf(line, "%u", &logica);

            printf("Entrada : %d\n\n", valorEntrada);
            
            if(valorEntrada == -1){
                imprimeTLB();
            }
            else if(valorEntrada == -2){
                imprimeTabelaDePaginas(0);
            }
            else if(valorEntrada == -3){
                imprimeTabelaDePaginas(1);
            }
            else{
                fisica = traduzirEndereco(logica);
                valor = acessarMemoriaFisica(fisica);

                printf("Endereco logico: %u \nEndereco fisico: %u \nValor do byte armazenado no endereço físico traduzido: %d\n", logica, fisica, valor);
            }
        }
    }
    printf("---------------------------------------------------------------------\n\t\t\t\tESTATISTICAS\n\n");
    Resultados();
    fecharArquivos();

    return 0;
}