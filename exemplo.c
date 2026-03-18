#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ===================== ESTRUTURAS =====================

typedef struct No {
    char nome[50];
    int idoso;
    char senha[10];
    time_t horarioEntrada;
    struct No *prox;
} No;

typedef struct {
    No *inicio;
    No *fim;
    int tamanho;
    int proximaSenhaIdoso;
    int proximaSenhaNormal;
} Fila;

// ===================== FUNÇÕES BASE =====================

void inicializarFila(Fila *f) {
    f->inicio = NULL;
    f->fim = NULL;
    f->tamanho = 0;
    f->proximaSenhaIdoso = 1;
    f->proximaSenhaNormal = 1;
}

int filaVazia(Fila *f) {
    return f->inicio == NULL;
}

// ===================== GERAR SENHA =====================

void gerarSenha(Fila *f, int idoso, char *senha) {
    if (idoso) {
        sprintf(senha, "I-%02d", f->proximaSenhaIdoso++);
    } else {
        sprintf(senha, "N-%02d", f->proximaSenhaNormal++);
    }
}

// ===================== ENFILEIRAR =====================

void enfileirar(Fila *f, char *nome, int idoso) {
    No *novo = (No*) malloc(sizeof(No));
    if (novo == NULL) {
        printf("Erro ao alocar memoria!\n");
        return;
    }

    strcpy(novo->nome, nome);
    novo->idoso = idoso;
    gerarSenha(f, idoso, novo->senha);
    novo->horarioEntrada = time(NULL);
    novo->prox = NULL;

    if (idoso && !filaVazia(f)) {
        No *atual = f->inicio;
        No *anterior = NULL;

        while (atual != NULL && atual->idoso == 1) {
            anterior = atual;
            atual = atual->prox;
        }

        if (anterior == NULL) {
            novo->prox = f->inicio;
            f->inicio = novo;
        } else {
            novo->prox = anterior->prox;
            anterior->prox = novo;
        }

        if (novo->prox == NULL) {
            f->fim = novo;
        }

    } else {
        if (filaVazia(f)) {
            f->inicio = novo;
            f->fim = novo;
        } else {
            f->fim->prox = novo;
            f->fim = novo;
        }
    }

    f->tamanho++;
    printf("\n[OK] %s entrou na fila!\n", nome);
    printf("     Senha: %s | Tipo: %s\n",
           novo->senha, idoso ? "Idoso (Preferencial)" : "Normal");
}

// ===================== CHAMAR PROXIMO =====================

void chamarProximo(Fila *f) {
    if (filaVazia(f)) {
        printf("\n[!] A fila esta vazia!\n");
        return;
    }

    No *removido = f->inicio;
    f->inicio = f->inicio->prox;

    if (f->inicio == NULL) {
        f->fim = NULL;
    }

    time_t agora = time(NULL);
    double espera = difftime(agora, removido->horarioEntrada);

    printf("\n========================================\n");
    printf("  CHAMANDO: %s\n", removido->nome);
    printf("  Senha : %s | Tipo: %s\n",
           removido->senha, removido->idoso ? "Idoso" : "Normal");
    printf("  Tempo de espera: %.0f segundos\n", espera);
    printf("========================================\n");

    free(removido);
    f->tamanho--;
}

// ===================== BUSCAR NA FILA =====================

void buscarNaFila(Fila *f, char *nome) {
    if (filaVazia(f)) {
        printf("\n[!] A fila esta vazia!\n");
        return;
    }

    No *atual = f->inicio;
    int posicao = 1;
    int encontrado = 0;

    while (atual != NULL) {
        if (strcasecmp(atual->nome, nome) == 0) {
            time_t agora = time(NULL);
            double espera = difftime(agora, atual->horarioEntrada);

            printf("\n[OK] Cliente encontrado!\n");
            printf("  Nome    : %s\n", atual->nome);
            printf("  Senha   : %s\n", atual->senha);
            printf("  Posicao : %d na fila\n", posicao);
            printf("  Tipo    : %s\n", atual->idoso ? "Idoso" : "Normal");
            printf("  Aguardando: %.0f segundos\n", espera);
            encontrado = 1;
            break;
        }
        atual = atual->prox;
        posicao++;
    }

    if (!encontrado) {
        printf("\n[!] Cliente '%s' nao encontrado na fila.\n", nome);
    }
}

// ===================== DESISTIR DA FILA =====================

void desistir(Fila *f, char *nome) {
    if (filaVazia(f)) {
        printf("\n[!] A fila esta vazia!\n");
        return;
    }

    No *atual = f->inicio;
    No *anterior = NULL;

    while (atual != NULL) {
        if (strcasecmp(atual->nome, nome) == 0) {
            if (anterior == NULL) {
                f->inicio = atual->prox;
            } else {
                anterior->prox = atual->prox;
            }

            if (atual->prox == NULL) {
                f->fim = anterior;
            }

            printf("\n[OK] %s (Senha: %s) desistiu da fila. Ate a proxima!\n",
                   atual->nome, atual->senha);
            free(atual);
            f->tamanho--;
            return;
        }
        anterior = atual;
        atual = atual->prox;
    }

    printf("\n[!] Cliente '%s' nao encontrado na fila.\n", nome);
}

// ===================== MOSTRAR FILA =====================

void mostrarFila(Fila *f) {
    if (filaVazia(f)) {
        printf("\n[!] A fila esta vazia!\n");
        return;
    }

    printf("\n========================================\n");
    printf("     FILA DO ACOUGUE (%d pessoa(s))\n", f->tamanho);
    printf("========================================\n");

    No *atual = f->inicio;
    int posicao = 1;

    while (atual != NULL) {
        time_t agora = time(NULL);
        double espera = difftime(agora, atual->horarioEntrada);

        printf("  %d. [%s] %-20s Senha: %s | Aguardando: %.0fs\n",
               posicao,
               atual->idoso ? "IDOSO " : "NORMAL",
               atual->nome,
               atual->senha,
               espera);

        atual = atual->prox;
        posicao++;
    }
    printf("========================================\n");
    printf("  Proxima senha idoso : I-%02d\n", f->proximaSenhaIdoso);
    printf("  Proxima senha normal: N-%02d\n", f->proximaSenhaNormal);
    printf("========================================\n");
}

// ===================== LIBERAR MEMORIA =====================

void liberarFila(Fila *f) {
    No *atual = f->inicio;
    while (atual != NULL) {
        No *prox = atual->prox;
        free(atual);
        atual = prox;
    }
    f->inicio = NULL;
    f->fim = NULL;
    f->tamanho = 0;
}

// ===================== MENU PRINCIPAL =====================

int main() {
    Fila f;
    inicializarFila(&f);

    int opcao;
    char nome[50];

    printf("========================================\n");
    printf("      BEM-VINDO AO ACOUGUE DO ZE!      \n");
    printf("========================================\n");

    do {
        printf("\n--- MENU ---\n");
        printf("1. Entrar na fila (Normal)\n");
        printf("2. Entrar na fila (Idoso/Preferencial)\n");
        printf("3. Chamar proximo cliente\n");
        printf("4. Buscar cliente na fila\n");
        printf("5. Cliente desistiu da fila\n");
        printf("6. Ver fila completa\n");
        printf("0. Sair\n");
        printf("Opcao: ");
        scanf("%d", &opcao);
        getchar();

        switch (opcao) {
            case 1:
                printf("Nome do cliente: ");
                fgets(nome, 50, stdin);
                nome[strcspn(nome, "\n")] = '\0';
                enfileirar(&f, nome, 0);
                break;

            case 2:
                printf("Nome do cliente idoso: ");
                fgets(nome, 50, stdin);
                nome[strcspn(nome, "\n")] = '\0';
                enfileirar(&f, nome, 1);
                break;

            case 3:
                chamarProximo(&f);
                break;

            case 4:
                printf("Nome a buscar: ");
                fgets(nome, 50, stdin);
                nome[strcspn(nome, "\n")] = '\0';
                buscarNaFila(&f, nome);
                break;

            case 5:
                printf("Nome do cliente que desistiu: ");
                fgets(nome, 50, stdin);
                nome[strcspn(nome, "\n")] = '\0';
                desistir(&f, nome);
                break;

            case 6:
                mostrarFila(&f);
                break;

            case 0:
                printf("\nEncerrando sistema. Ate logo!\n");
                liberarFila(&f);
                break;

            default:
                printf("\n[!] Opcao invalida!\n");
        }

    } while (opcao != 0);

    return 0;
}
