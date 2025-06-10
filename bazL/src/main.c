#include <stdio.h>
#include <bazL.h>

void pausar(){
    while(getchar() != '\n');
}

void limpar_buffer(){
    while(getchar() != '\n');
}

void limpar_tela(){
    printf("\033c\033[H");
}

int main(){
    char nomeArquivo[256];
    int escopo_dinamico = 0;
    
    printf("\nbaZL Debugger\n-------------\npor João Vitor Assumpção Proença");
    
    printf("\n\n> Arquivo de código: ");
    scanf("%[^\n]", nomeArquivo);
    limpar_buffer();
    
    FILE* teste = fopen(nomeArquivo, "r");
    
    if(teste == NULL){
        printf("\n> Erro ao abrir o arquivo! Tente novamente!\n");
        return -1;
    }
    
    fclose(teste);
    
    // Infelizmente, a funcionalidade de escopo dinâmico não foi implementada.
    printf("\n> Deseja utilizar escopo dinâmico?"
           "\n> Digite 0 para NÃO ou outro número para SIM: ");
    scanf("%d", &escopo_dinamico);
    limpar_buffer();
    
    Programa* Prog = bazL_carregar_programa(nomeArquivo, escopo_dinamico);
    
    printf("\n\nCÓDIGO\n------\n");
    bazL_imprimir_codigo(Prog);
    pausar();
    limpar_tela();
    
    while(!(bazL_fim_programa(Prog))){
        bazL_executar_prox_linha(Prog);
        
        printf("\n\nCÓDIGO\n------\n");
        bazL_imprimir_codigo(Prog);
        
        printf("\n\nESTADO DA PILHA\n-----------------");
        bazL_imprimir_estado(Prog);
        
        pausar();
        limpar_tela();
    }
    
    bazL_descarregar_programa(Prog);
}
