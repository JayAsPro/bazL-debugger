#ifndef BAZL_H_INCLUDED
#define BAZL_H_INCLUDED

typedef struct Programa Programa;

Programa* bazL_carregar_programa(char nomeArquivo[], int escopo);

void bazL_imprimir_codigo(Programa* Prog);

void bazL_imprimir_estado(Programa* Prog);

void bazL_executar_prox_linha(Programa* Prog);

int bazL_fim_programa(Programa* Prog);

void bazL_descarregar_programa(Programa* Prog);

#endif // BAZL_H_INCLUDED
