/*
 * ============================================================
 *  SIMULADOR DE FILA DE ATENDIMENTO - CALL CENTER (SUPORTE VIA TELEFONE)
 * ============================================================
 * Integrantes: [Nome dos Integrantes]
 * Turma: [Turma]
 * Contexto: Simulador de fila de atendimento em Call Center de suporte tecnico via telefone
 * ============================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================
   CONSTANTES E DEFINICOES
   ============================================================ */
#define TAM_NOME        64
#define NORMAL          1
#define PRIORITARIO     2
#define NUM_ATENDENTES  2

/* ============================================================
   STRUCTS
   ============================================================ */

/* Struct do cliente */
typedef struct {
    int  id;
    char nome[TAM_NOME];
    int  tipoServico;   /* 1-hardware 2-software 3-rede 4-outros */
    int  prioridade;    /* NORMAL (1) ou PRIORITARIO (2)          */
    int  horaChegada;
} Cliente;

/* No da lista encadeada */
typedef struct No {
    Cliente    dado;
    struct No *proximo;
} No;

/* Fila (lista encadeada FIFO) */
typedef struct {
    No  *frente;
    No  *cauda;
    int  tamanho;
} Fila;

/* Par de filas: prioritaria + normal */
typedef struct {
    Fila filaPrioritaria;
    Fila filaNormal;
} FilaPrioritaria;

/* Atendente (guiche) */
typedef struct {
    int     numero;
    int     ocupado;
    Cliente clienteAtual;
    int     totalAtendidos;
} Atendente;

/* ============================================================
   IMPLEMENTACAO DA FILA
   ============================================================ */

void inicializarFila(Fila *f) {
    f->frente  = NULL;
    f->cauda   = NULL;
    f->tamanho = 0;
}

int filaVazia(Fila *f) {
    return (f->frente == NULL);
}

/* Enfileirar: insere no FIM (cauda) — FIFO */
void enfileirar(Fila *f, Cliente c) {
    No *novo = (No *)malloc(sizeof(No));
    if (novo == NULL) {
        printf("  [ERRO] Memoria insuficiente.\n");
        return;
    }
    novo->dado     = c;
    novo->proximo  = NULL;

    if (filaVazia(f)) {
        f->frente = novo;
        f->cauda  = novo;
    } else {
        f->cauda->proximo = novo;
        f->cauda          = novo;
    }
    f->tamanho++;
}

/* Desenfileirar: remove da FRENTE — FIFO */
Cliente desenfileirar(Fila *f) {
    Cliente vazio = {0};
    if (filaVazia(f)) return vazio;

    No     *removido = f->frente;
    Cliente dado     = removido->dado;

    f->frente = removido->proximo;
    if (f->frente == NULL)
        f->cauda = NULL;

    free(removido);
    f->tamanho--;
    return dado;
}

int tamanhoFila(Fila *f) {
    return f->tamanho;
}

/* Listar todos os clientes em ordem */
void listarClientes(Fila *f) {
    if (filaVazia(f)) {
        printf("    (vazia)\n");
        return;
    }
    No  *atual = f->frente;
    int  pos   = 1;

    const char *servicos[] = {"", "Suporte de Hardware", "Suporte de Software",
                               "Conexao/Rede", "Reclamacao/Outros"};

    printf("  %-4s %-6s %-20s %-22s %-13s\n",
           "Pos", "ID", "Nome", "Servico", "Prioridade");
    printf("  %-4s %-6s %-20s %-22s %-13s\n",
           "----","------","--------------------","----------------------",
           "-------------");

    while (atual != NULL) {
        printf("  %-4d #%-5d %-20s %-22s %-13s\n",
               pos, atual->dado.id, atual->dado.nome,
               servicos[atual->dado.tipoServico],
               atual->dado.prioridade == PRIORITARIO ? "PRIORITARIO" : "Normal");
        atual = atual->proximo;
        pos++;
    }
}

/* Remover por ID (cancelamento) — retorna 1 se removeu */
int removerPorId(Fila *f, int id) {
    if (filaVazia(f)) return 0;

    No *atual    = f->frente;
    No *anterior = NULL;

    while (atual != NULL) {
        if (atual->dado.id == id) {
            if (anterior == NULL)
                f->frente = atual->proximo;
            else
                anterior->proximo = atual->proximo;

            if (atual == f->cauda)
                f->cauda = anterior;

            free(atual);
            f->tamanho--;
            return 1;
        }
        anterior = atual;
        atual    = atual->proximo;
    }
    return 0;
}

/* Libera toda a memoria da fila */
void liberarFila(Fila *f) {
    while (!filaVazia(f))
        desenfileirar(f);
}

/* ============================================================
   FILA PRIORITARIA
   ============================================================ */
void inicializarFilaPrioritaria(FilaPrioritaria *fp) {
    inicializarFila(&fp->filaPrioritaria);
    inicializarFila(&fp->filaNormal);
}

void liberarFilaPrioritaria(FilaPrioritaria *fp) {
    liberarFila(&fp->filaPrioritaria);
    liberarFila(&fp->filaNormal);
}

/* ============================================================
   FUNCOES AUXILIARES
   ============================================================ */
void limparBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void exibirMenu(void) {
    printf("\n+------------------------------------------+\n");
    printf("|            MENU PRINCIPAL                |\n");
    printf("+------------------------------------------+\n");
    printf("|  1. Adicionar cliente a fila             |\n");
    printf("|  2. Atender proximo cliente              |\n");
    printf("|  3. Listar fila atual                    |\n");
    printf("|  4. Cancelar chamado (por ID)            |\n");
    printf("|  5. Ver estatisticas parciais            |\n");
    printf("|  0. Encerrar simulacao                   |\n");
    printf("+------------------------------------------+\n");
    printf("  Opcao: ");
}

/* ============================================================
   FUNCOES DO SIMULADOR
   ============================================================ */

void adicionarCliente(FilaPrioritaria *fp, int *proximoId,
                      int *tempoAtual, int *tamanhoMaximo) {
    Cliente c;
    memset(&c, 0, sizeof(c));
    c.id         = *proximoId;
    c.horaChegada = *tempoAtual;
    (*tempoAtual)++;

    printf("\n  === NOVO CHAMADO (ID #%04d) ===\n", c.id);

    printf("  Nome do cliente: ");
    fgets(c.nome, TAM_NOME, stdin);
    c.nome[strcspn(c.nome, "\n")] = '\0';
    if (strlen(c.nome) == 0) strcpy(c.nome, "Anonimo");

    printf("  Tipos de servico:\n");
    printf("    1 - Suporte tecnico de hardware\n");
    printf("    2 - Suporte tecnico de software\n");
    printf("    3 - Problema de conexao/rede\n");
    printf("    4 - Reclamacao / outros\n");
    printf("  Tipo (1-4): ");

    int tp = 0;
    while (scanf("%d", &tp) != 1 || tp < 1 || tp > 4) {
        limparBuffer();
        printf("  [!] Invalido. Tipo (1-4): ");
    }
    limparBuffer();
    c.tipoServico = tp;

    printf("  Prioridade:\n");
    printf("    1 - Normal\n");
    printf("    2 - Prioritario (idoso, deficiente, urgencia)\n");
    printf("  Prioridade (1 ou 2): ");

    int pr = 0;
    while (scanf("%d", &pr) != 1 || (pr != 1 && pr != 2)) {
        limparBuffer();
        printf("  [!] Invalido (1 ou 2): ");
    }
    limparBuffer();
    c.prioridade = pr;

    if (pr == PRIORITARIO)
        enfileirar(&fp->filaPrioritaria, c);
    else
        enfileirar(&fp->filaNormal, c);

    (*proximoId)++;

    int tam = tamanhoFila(&fp->filaPrioritaria) + tamanhoFila(&fp->filaNormal);
    if (tam > *tamanhoMaximo)
        *tamanhoMaximo = tam;

    printf("\n  [OK] Cliente '%s' adicionado a fila %s.\n",
           c.nome, pr == PRIORITARIO ? "PRIORITARIA" : "NORMAL");
}

void atenderCliente(FilaPrioritaria *fp, Atendente atendentes[],
                    int numAtendentes, int *totalAtendidos,
                    int *tempoAtual, int *tempoTotalEspera) {

    if (filaVazia(&fp->filaPrioritaria) && filaVazia(&fp->filaNormal)) {
        printf("\n  [!] Nenhum cliente em espera no momento.\n");
        return;
    }

    /* Encontra atendente livre */
    int idx = -1;
    for (int i = 0; i < numAtendentes; i++) {
        if (!atendentes[i].ocupado) { idx = i; break; }
    }
    if (idx == -1) {
        printf("\n  [!] Todos os atendentes estao ocupados.\n");
        return;
    }

    /* Regra: prioritario sempre primeiro */
    Cliente c;
    if (!filaVazia(&fp->filaPrioritaria))
        c = desenfileirar(&fp->filaPrioritaria);
    else
        c = desenfileirar(&fp->filaNormal);

    int espera = *tempoAtual - c.horaChegada;
    *tempoTotalEspera += espera;

    atendentes[idx].clienteAtual  = c;
    atendentes[idx].totalAtendidos++;
    (*totalAtendidos)++;
    (*tempoAtual)++;

    const char *servicos[] = {"", "Suporte de Hardware", "Suporte de Software",
                               "Conexao/Rede", "Reclamacao/Outros"};

    printf("\n  === ATENDIMENTO INICIADO ===\n");
    printf("  Atendente.....: Guiche %d\n", atendentes[idx].numero);
    printf("  ID Chamado....: #%04d\n", c.id);
    printf("  Cliente.......: %s\n", c.nome);
    printf("  Servico.......: %s\n", servicos[c.tipoServico]);
    printf("  Prioridade....: %s\n",
           c.prioridade == PRIORITARIO ? "PRIORITARIO" : "Normal");
    printf("  Tempo de espera: %d unidade(s)\n", espera);
}

void listarFila(FilaPrioritaria *fp) {
    int total = tamanhoFila(&fp->filaPrioritaria) + tamanhoFila(&fp->filaNormal);
    printf("\n  === FILA ATUAL (%d cliente(s)) ===\n", total);

    printf("\n  [FILA PRIORITARIA - %d cliente(s)]\n",
           tamanhoFila(&fp->filaPrioritaria));
    listarClientes(&fp->filaPrioritaria);

    printf("\n  [FILA NORMAL - %d cliente(s)]\n",
           tamanhoFila(&fp->filaNormal));
    listarClientes(&fp->filaNormal);
}

void cancelarCliente(FilaPrioritaria *fp) {
    if (filaVazia(&fp->filaPrioritaria) && filaVazia(&fp->filaNormal)) {
        printf("\n  [!] Nenhum cliente em espera para cancelar.\n");
        return;
    }

    printf("\n  === CANCELAR CHAMADO ===\n");
    printf("  Informe o ID do chamado: #");
    int id = 0;
    while (scanf("%d", &id) != 1 || id <= 0) {
        limparBuffer();
        printf("  [!] ID invalido. Informe o ID: #");
    }
    limparBuffer();

    if (removerPorId(&fp->filaPrioritaria, id) ||
        removerPorId(&fp->filaNormal, id))
        printf("  [OK] Chamado #%04d cancelado com sucesso.\n", id);
    else
        printf("  [!] Chamado #%04d nao encontrado.\n", id);
}

void exibirEstatisticasParciais(FilaPrioritaria *fp,
                                Atendente atendentes[],
                                int numAtendentes, int totalAtendidos) {
    printf("\n  === ESTATISTICAS PARCIAIS ===\n");
    printf("  Clientes atendidos  : %d\n", totalAtendidos);
    printf("  Fila prioritaria    : %d cliente(s)\n",
           tamanhoFila(&fp->filaPrioritaria));
    printf("  Fila normal         : %d cliente(s)\n",
           tamanhoFila(&fp->filaNormal));
    printf("  Total em espera     : %d cliente(s)\n",
           tamanhoFila(&fp->filaPrioritaria) + tamanhoFila(&fp->filaNormal));
    printf("\n  Status dos atendentes:\n");
    for (int i = 0; i < numAtendentes; i++) {
        printf("    Guiche %d: %-8s | Atendidos: %d\n",
               atendentes[i].numero,
               atendentes[i].ocupado ? "OCUPADO" : "Livre",
               atendentes[i].totalAtendidos);
    }
}

void exibirResumoFinal(FilaPrioritaria *fp, int totalAtendidos,
                       int tamanhoMaximo, int tempoTotalEspera,
                       Atendente atendentes[], int numAtendentes) {
    printf("\n+===============================================+\n");
    printf("|           RESUMO DA SIMULACAO                 |\n");
    printf("+===============================================+\n");
    printf("|  Total de clientes atendidos  : %-10d   |\n", totalAtendidos);
    printf("|  Tamanho maximo da fila       : %-10d   |\n", tamanhoMaximo);
    printf("|  Clientes ainda em espera     : %-10d   |\n",
           tamanhoFila(&fp->filaPrioritaria) + tamanhoFila(&fp->filaNormal));

    if (totalAtendidos > 0)
        printf("|  Tempo medio de espera        : %-10.1f   |\n",
               (double)tempoTotalEspera / totalAtendidos);
    else
        printf("|  Tempo medio de espera        : N/A            |\n");

    printf("+-----------------------------------------------+\n");
    printf("|  Desempenho por atendente:                    |\n");
    for (int i = 0; i < numAtendentes; i++)
        printf("|    Guiche %-2d : %-3d atendido(s)                |\n",
               atendentes[i].numero, atendentes[i].totalAtendidos);
    printf("+===============================================+\n");
}

/* ============================================================
   MAIN
   ============================================================ */
int main(void) {
    FilaPrioritaria fp;
    inicializarFilaPrioritaria(&fp);

    Atendente atendentes[NUM_ATENDENTES];
    for (int i = 0; i < NUM_ATENDENTES; i++) {
        atendentes[i].numero         = i + 1;
        atendentes[i].ocupado        = 0;
        atendentes[i].totalAtendidos = 0;
    }

    int opcao            = 0;
    int proximoId        = 1;
    int totalAtendidos   = 0;
    int tamanhoMaximo    = 0;
    int tempoAtual       = 0;
    int tempoTotalEspera = 0;

    printf("\n+===============================================+\n");
    printf("|  CALL CENTER - SUPORTE TECNICO VIA TELEFONE  |\n");
    printf("+===============================================+\n");

    do {
        exibirMenu();

        while (scanf("%d", &opcao) != 1) {
            limparBuffer();
            printf("  [!] Digite um numero valido: ");
        }
        limparBuffer();

        switch (opcao) {
            case 1:
                adicionarCliente(&fp, &proximoId, &tempoAtual, &tamanhoMaximo);
                break;
            case 2:
                atenderCliente(&fp, atendentes, NUM_ATENDENTES,
                               &totalAtendidos, &tempoAtual, &tempoTotalEspera);
                break;
            case 3:
                listarFila(&fp);
                break;
            case 4:
                cancelarCliente(&fp);
                break;
            case 5:
                exibirEstatisticasParciais(&fp, atendentes,
                                           NUM_ATENDENTES, totalAtendidos);
                break;
            case 0:
                exibirResumoFinal(&fp, totalAtendidos, tamanhoMaximo,
                                  tempoTotalEspera, atendentes, NUM_ATENDENTES);
                liberarFilaPrioritaria(&fp);
                printf("\n  Encerrando o sistema. Ate logo!\n\n");
                break;
            default:
                printf("  [!] Opcao invalida. Tente novamente.\n");
        }
    } while (opcao != 0);

    return 0;
}
