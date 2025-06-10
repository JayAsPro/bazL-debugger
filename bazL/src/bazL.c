#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <bazL.h>

/* Definições privadas */

#define tam_nome 32
#define tam_ln 256
#define max_tokens 64
#define max_lns 256

#define separadores "=+(){},"

typedef struct Var{
    char nome[tam_nome];
    int info;
    
    struct Var* prox;
} Var;

typedef struct RegAtiv{
    char nomeFunc[tam_nome];
    Var* params;
    Var* varsLocais;
    int endRet;
    
    struct RegAtiv* prox;
} RegAtiv;

typedef struct Programa{
    char nomeArquivo[tam_nome];
    char codigo[max_lns][tam_ln];
    int nLinhas;
    int lnAtual;
    int escopo_dinamico;
    RegAtiv* Pilha;
} Programa;

/* Funções privadas */

int carregar_codigo(FILE* Arquivo, char codigo[][tam_ln]){
    int nLinhas = 0;
    
    rewind(Arquivo);
    
    while(!feof(Arquivo)){
        codigo[nLinhas][0] = '\0'; // Proteção para caso o fgets não escreva nada.
        fgets(codigo[nLinhas], tam_ln, Arquivo);
        nLinhas += 1;
    }
    
    return nLinhas;
}

int tokenizar_linha(char linha[], char tokens[][tam_nome]){
    int nTokens = 0;
    int posLinha = 0;
    int contChar;
    
    int tamLinha = strlen(linha);
    if(linha[tamLinha - 1] == '\n') // Proteção contra um loop infinito.
        tamLinha -= 1;
    
    int ver;
    
    while(posLinha < tamLinha){
        contChar = 0;
        ver = sscanf(&linha[posLinha], " %[^ \n" separadores "%]%n", tokens[nTokens], &contChar);
        posLinha += contChar;
        
        if(ver == 0){
            ver = sscanf(&linha[posLinha], " %1[" separadores "%]%n", tokens[nTokens], &contChar);
            posLinha += contChar;
        }
        
        if(ver > 0)
            nTokens += 1;
    }
    
    return nTokens;
}

Var* declarar_Var(Var* vars, char nome[]){
    Var* novaVar = (Var*) malloc(sizeof(Var));
    strcpy(novaVar->nome, nome);
    
    // Inicializar as variáveis por padrão não está definido na linguagem.
    novaVar->info = 0;
    
    novaVar->prox = vars;
    
    return novaVar;
}

Var* remover_Vars(Var* vars){
    while(vars != NULL){
        Var* varAtual = vars;
        vars = vars->prox;
        free(varAtual);
    }
    
    return vars;
}

void criar_RegAtiv(Programa* Prog, char nomeFunc[], long endRet){
    RegAtiv* regNovo = (RegAtiv*) malloc(sizeof(RegAtiv));
    
    strcpy(regNovo->nomeFunc, nomeFunc);
    regNovo->params = NULL;
    regNovo->varsLocais = NULL;
    regNovo->endRet = endRet;
    
    regNovo->prox = Prog->Pilha;
    
    Prog->Pilha = regNovo;
}

void remover_RegAtiv_atual(Programa* Prog){
    RegAtiv* regAtual = Prog->Pilha;
    Prog->Pilha = Prog->Pilha->prox;
    
    Prog->lnAtual = regAtual->endRet;
    
    regAtual->varsLocais = remover_Vars(regAtual->varsLocais);
    regAtual->params = remover_Vars(regAtual->params);
    free(regAtual);
}

Var* loop_encontrar_variavel(Var* p, char nome[]){
    while(p != NULL){
        if(!strcmp(p->nome, nome))
            return p;
        p = p->prox;
    }
    
    return NULL;
}

Var* encontrar_variavel(Programa* Prog, char nome[]){
    Var* p = loop_encontrar_variavel(Prog->Pilha->params, nome);
    
    if(p == NULL){
        p = loop_encontrar_variavel(Prog->Pilha->varsLocais, nome);
        
        if(p == NULL){
            RegAtiv* reg = Prog->Pilha;
            
            while(reg->prox != NULL)
                reg = reg->prox;
            
            p = loop_encontrar_variavel(reg->varsLocais, nome);
        }
    }
    
    return p;
}

int calcular_expressao(Programa* Prog, char tokens[][tam_nome], int nTokens){
    int i = 1;
    int acc = 0;
    
    int inteiro = 0;
    int ver;
    
    if(!strcmp(tokens[i], "(") || !strcmp(tokens[i], "=")){
        i += 1;
        while(i < nTokens){
            if(!strcmp(tokens[i], "+"))
                i += 1;
            
            ver = sscanf(tokens[i], "%d", &inteiro);
            
            if(ver == 0){
                Var* p = encontrar_variavel(Prog, tokens[i]);
                
                if(p != NULL)
                    inteiro = p->info;
            }
            
            acc += inteiro;
            i += 1;
        }
    }
    
    return acc;
}

void pular_para_funcao(Programa* Prog, char nomeFunc[]){
    char tokens[max_tokens][tam_nome];
    int nTokens = 0;
    
    for(int i = 0; i < Prog->nLinhas; i++){
        nTokens = tokenizar_linha(Prog->codigo[i], tokens);
        
        if(nTokens > 1){
            if(!strcmp(tokens[0], "func")){
                if(!strcmp(tokens[1], nomeFunc)){
                    criar_RegAtiv(Prog, nomeFunc, Prog->lnAtual);
                
                    // Se for a main, o endereço de retorno é após o fim do código.
                    if(!strcmp(Prog->Pilha->nomeFunc, "main"))
                        Prog->Pilha->endRet = Prog->nLinhas + 1;
                
                    Prog->lnAtual = i + 2;
                    return;
                }
            }
        }
    }
}

/* Funções públicas */

Programa* bazL_carregar_programa(char nomeArquivo[], int escopo_dinamico){
    FILE* Arquivo = fopen(nomeArquivo, "r");
    
    if(Arquivo == NULL)
        return NULL;
    
    Programa* Prog = (Programa*) malloc(sizeof(Programa));
    Prog->nLinhas = carregar_codigo(Arquivo, Prog->codigo);
    Prog->lnAtual = 1;
    Prog->escopo_dinamico = escopo_dinamico;
    Prog->Pilha = NULL;
    
    fclose(Arquivo);
    return Prog;
}

void bazL_imprimir_codigo(Programa* Prog){
    for(int i = 0; i < Prog->nLinhas; i++){
        if(i+1 == Prog->lnAtual)
            printf(">>%3u  %s", i+1, Prog->codigo[i]);
        else
            printf("  %3u  %s", i+1, Prog->codigo[i]);
    }
}

void bazL_imprimir_estado(Programa* Prog){
    RegAtiv* reg = Prog->Pilha;
    
    while(reg != NULL){
        printf("\n\n%s | End. ret.: %d", reg->nomeFunc, reg->endRet);
        
        printf("\nParâmetros: ");
        
        Var* p = reg->params;
        while(p != NULL){
            printf("%s = %d | ", p->nome, p->info);
            p = p->prox;
        }
        
        printf("\nVariáveis: ");
        
        p = reg->varsLocais;
        while(p != NULL){
            printf("%s = %d | ", p->nome, p->info);
            p = p->prox;
        }
        
        reg = reg->prox;
    }
}

void bazL_executar_prox_linha(Programa* Prog){
    char tokens[max_tokens][tam_nome];
    int nTokens = 0;
    char* linha;
    
    if(Prog->lnAtual > Prog->nLinhas)
        return;
    
    if(Prog->Pilha == NULL){
        // Isto faz "global" ser, indesejavelmente, palavra reservada.
        // O endereço de retorno é após o fim do código.
        criar_RegAtiv(Prog, "global", Prog->nLinhas + 1);
    }
    
    linha = Prog->codigo[Prog->lnAtual - 1];
    nTokens = tokenizar_linha(linha, tokens);
    
    if(nTokens > 0){
        if(!strcmp(tokens[0], "var"))
            Prog->Pilha->varsLocais = declarar_Var(Prog->Pilha->varsLocais, tokens[1]);
        
        else if(!strcmp(Prog->Pilha->nomeFunc, "global")){
            pular_para_funcao(Prog, "main");
            return;
        }
        else{
            if(!strcmp(tokens[0], "}"))
                remover_RegAtiv_atual(Prog);
            else if(!strcmp(tokens[1], "=")){
                Var* p = encontrar_variavel(Prog, tokens[0]);
                if(p != NULL){
                    p->info = calcular_expressao(Prog, tokens, nTokens);
                }
            }
        }
    }
    
    Prog->lnAtual += 1;
}

int bazL_fim_programa(Programa* Prog){
    if(Prog->lnAtual > Prog->nLinhas)
        return 1;
    return 0;
}

void bazL_descarregar_programa(Programa* Prog){
    while(Prog->Pilha != NULL)
        remover_RegAtiv_atual(Prog);
    
    free(Prog);
}
