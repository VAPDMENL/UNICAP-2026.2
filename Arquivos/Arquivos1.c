#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARQ_PRODUTOS "produtos.dat"
#define ARQ_VENDAS   "vendas.dat"
#define ARQ_TEMP     "temp.dat"

#define TAM_DESC     50
#define TAM_CLIENTE  50
#define TAM_DATA     11   /* dd/mm/aaaa */
#define MAX_ITENS    10   /* produtos distintos por venda */

/* ------------------------------------------------------------------ */
/* Registros                                                           */
/* O campo "id" e sempre o PRIMEIRO membro (usado na remocao generica) */
/* ------------------------------------------------------------------ */

typedef struct {
    int   id;
    char  descricao[TAM_DESC];
    float preco;
    int   estoque;
} Produto;

typedef struct {
    int   id;
    char  cliente[TAM_CLIENTE];
    char  data[TAM_DATA];
    int   qtdItens;                  /* quantos itens do vetor estao em uso */
    int   idsProdutos[MAX_ITENS];    /* vetor de IDs (relacionamento) */
    int   quantidades[MAX_ITENS];    /* quantidade vendida de cada produto */
    float precos[MAX_ITENS];         /* preco unitario no momento da venda */
} Venda;

/* ------------------------------------------------------------------ */
/* Entrada de dados                                                    */
/* ------------------------------------------------------------------ */

static void lerTexto(const char *msg, char *dest, size_t tam) {
    printf("%s", msg);
    if (fgets(dest, (int)tam, stdin) == NULL) {
        exit(0); /* fim da entrada */
    }
    size_t n = strlen(dest);
    if (n > 0 && dest[n - 1] == '\n') {
        dest[n - 1] = '\0';
    } else {
        int c;
        while ((c = getchar()) != '\n' && c != EOF) { /* descarta o excesso */ }
    }
}

static int lerInt(const char *msg) {
    char buf[64], *fim;
    long v;
    while (1) {
        lerTexto(msg, buf, sizeof buf);
        v = strtol(buf, &fim, 10);
        if (fim != buf && *fim == '\0') return (int)v;
        printf("Valor invalido. Tente novamente.\n");
    }
}

static int lerIntNaoNegativo(const char *msg) {
    int v;
    while ((v = lerInt(msg)) < 0) printf("O valor nao pode ser negativo.\n");
    return v;
}

static float lerPreco(const char *msg) {
    char buf[64], *fim;
    float v;
    while (1) {
        lerTexto(msg, buf, sizeof buf);
        for (char *c = buf; *c; c++) if (*c == ',') *c = '.';
        v = strtof(buf, &fim);
        if (fim != buf && *fim == '\0' && v >= 0) return v;
        printf("Valor invalido. Tente novamente.\n");
    }
}

/* ------------------------------------------------------------------ */
/* Operacoes basicas - PRODUTOS                                        */
/* ------------------------------------------------------------------ */

/* Leitura de um elemento especifico (busca sequencial por ID) */
static int buscarProduto(int id, Produto *saida) {
    FILE *f = fopen(ARQ_PRODUTOS, "rb");
    Produto aux;
    if (!f) return 0;
    while (fread(&aux, sizeof aux, 1, f) == 1) {
        if (aux.id == id) {
            if (saida) *saida = aux;
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

static int proximoIdProduto(void) {
    FILE *f = fopen(ARQ_PRODUTOS, "rb");
    Produto aux;
    int max = 0;
    if (!f) return 1;
    while (fread(&aux, sizeof aux, 1, f) == 1)
        if (aux.id > max) max = aux.id;
    fclose(f);
    return max + 1;
}

/* Escrita de registro novo (no fim do arquivo) */
static int inserirProduto(const Produto *p) {
    FILE *f = fopen(ARQ_PRODUTOS, "ab");
    if (!f) return 0;
    fwrite(p, sizeof *p, 1, f);
    fclose(f);
    return 1;
}

/* Escrita de atualizacao (sobrescreve o registro que ja existe) */
static int gravarProduto(const Produto *p) {
    FILE *f = fopen(ARQ_PRODUTOS, "rb+");
    Produto aux;
    if (!f) return 0;
    while (fread(&aux, sizeof aux, 1, f) == 1) {
        if (aux.id == p->id) {
            fseek(f, -(long)sizeof aux, SEEK_CUR);
            fwrite(p, sizeof *p, 1, f);
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

/* ------------------------------------------------------------------ */
/* Operacoes basicas - VENDAS                                          */
/* ------------------------------------------------------------------ */

static int buscarVenda(int id, Venda *saida) {
    FILE *f = fopen(ARQ_VENDAS, "rb");
    Venda aux;
    if (!f) return 0;
    while (fread(&aux, sizeof aux, 1, f) == 1) {
        if (aux.id == id) {
            if (saida) *saida = aux;
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

static int listarProdutos(void) {
    FILE *f = fopen(ARQ_PRODUTOS, "rb");
    if (!f) return 0;
    Produto aux;
    int cont = 0;
    while (fread(&aux, sizeof aux, 1, f) == 1){
        printf("ID: %d | Descrição: %s | Preço: %.2f | Estoque: %d\n", aux.id, aux.descricao, aux.preco, aux.estoque);
        cont ++;
        }
    if (cont == 0) {printf("Arquivo vazio.\n");}
    fclose(f);
    return 1;
}

static int removerProduto(int idAlvo) {
    FILE *orig = fopen(ARQ_PRODUTOS, "rb");
    FILE *temp = fopen(ARQ_TEMP, "wb");
    if (!orig || !temp) {
        if (orig) fclose(orig);
        if (temp) fclose(temp);
        return 0;}
    Produto aux;
    int removido = 0;
    while (fread(&aux, sizeof aux, 1, orig) == 1){
    if (aux.id == idAlvo) removido = 1;
    else {
        fwrite(&aux, sizeof aux, 1, temp);} }
    fclose(orig);
    fclose(temp);
    remove(ARQ_PRODUTOS);
    rename(ARQ_TEMP, ARQ_PRODUTOS);
    if (removido == 0) remove(ARQ_TEMP);
    return removido;    }

static int atualizarProduto(int idAlvo) {
    Produto p;
    if (buscarProduto(idAlvo, &p) == 0) {
        printf("Produto inexistente");
        return 0;
    }
    printf("Descrição : %s | Preço : %.2f | Estoque : %d\n", p.descricao, p.preco, p.estoque);
    printf("Quais os novos valores?");
    p.preco = lerPreco("Novo preco: ");
    p.estoque = lerIntNaoNegativo("Novo estoque: ");
    if (gravarProduto(&p)) {
        return 1;
    }
    return 0;
}
static int proximoIdVenda(void) {
    FILE *f = fopen(ARQ_VENDAS, "rb");
    Venda aux;
    int max = 0;
    if (!f) return 1;
    while (fread(&aux, sizeof aux, 1, f) == 1)
        if (aux.id > max) max = aux.id;
    fclose(f);
    return max + 1;
}

static int inserirVenda(const Venda *v) {
    FILE *f = fopen(ARQ_VENDAS, "ab");
    if (!f) return 0;
    fwrite(v, sizeof *v, 1, f);
    fclose(f);
    return 1;
}

static int listarVendas(void) {
    FILE *f = fopen(ARQ_VENDAS, "rb");
    if (!f) return 0;
    Venda aux;
    int cont = 0;
    while (fread(&aux, sizeof aux, 1, f) == 1){
        printf("ID: %d | Cliente: %s | Data: %s | Quantidade: %d\n", aux.id, aux.cliente, aux.data, aux.qtdItens);
        cont ++;
        }
    if (cont == 0) {printf("Arquivo vazio.\n");}
    fclose(f);
    return 1;
}

static int removerVenda(int idAlvo) {
    FILE *orig = fopen(ARQ_VENDAS, "rb");
    FILE *temp = fopen(ARQ_TEMP, "wb");
    if (!orig || !temp) {
        if (orig) fclose(orig);
        if (temp) fclose(temp);
        return 0;}
    Venda aux;
    int removido = 0;
    while (fread(&aux, sizeof aux, 1, orig) == 1){
    if (aux.id == idAlvo) removido = 1;
    else {
        fwrite(&aux, sizeof aux, 1, temp);} }
    fclose(orig);
    fclose(temp);
    remove(ARQ_VENDAS);
    rename(ARQ_TEMP, ARQ_VENDAS);
    if (removido == 0) remove(ARQ_TEMP);
    return removido;    }

static void exibirVendaDetalhada(const Venda *v) {
    printf("Id : %d | Cliente : %s | Data : %s| Quantidade : %d\n", v->id, v->cliente, v->data, v->qtdItens);
    float totalGeral = 0.0f;
    for (int i = 0; i < v->qtdItens; i++){
        int idProd = v->idsProdutos[i];
        Produto prod;
        totalGeral += (v->quantidades[i]*v->precos[i]);
        if (buscarProduto(idProd, &prod)) {
            printf("Descrição : %s| Data: %s| Quantidade : %d| Preço : %.2f\n", prod.descricao, v->data, v->quantidades[i], v->precos[i]);
        }
        else {
            printf("Produto não encontrado (ID %d)\n", idProd);
        }
    }
    printf("Total da venda : %.2f\n", totalGeral);
}
static void cadastrarVenda(void) {
    Venda v;
    v.id = proximoIdVenda();
    lerTexto("Cliente : ", v.cliente, sizeof v.cliente);
    lerTexto("Data (dd/mm/aaaa) : ", v.data, sizeof v.data);
    v.qtdItens = 0;
    while (v.qtdItens < MAX_ITENS) {
        int idProd = lerInt("Id do produto (0 para eencerrar): ");
        if (idProd == 0) {
            break;
        }
        Produto prod;
        if (buscarProduto(idProd, &prod) == 0) {
            printf("Produto não encontrado.");
            continue;
        }
        if (prod.estoque == 0) {
            printf("Estoque vazio.");
            continue;
        }
        int qtd = lerIntNaoNegativo("Quantidade : ");
        if (qtd == 0) continue;
        if (qtd > prod.estoque) {
        int resp =lerInt("Estoque insuficiente. Deseja levar o restante e zerar o estoque? (1 : Sim | 0 :Não ): \n");
        if (resp == 1) {
            qtd = prod.estoque;
        } else if (resp == 0) {
            continue;
        }
        else {
            printf("Valor inválido.\n");
            continue;}
        }
        int idx = v.qtdItens;
        v.idsProdutos[idx] = prod.id;
        v.quantidades[idx] = qtd;
        v.precos[idx] = prod.preco;
        v.qtdItens++;
        prod.estoque -= qtd;
        gravarProduto(&prod);}
        if (v.qtdItens > 0) {
            inserirVenda(&v);
            printf("Confirmado.\n");
        }else {
            printf("Nenhum item adicionado.");
        }
        }
static void cadastrarProduto(void) {
    Produto p;
    p.id = proximoIdProduto();
    lerTexto("Descrição : ", p.descricao, sizeof p.descricao);
    p.preco = lerPreco("Preço : ");
    p.estoque = lerIntNaoNegativo("Estoque inicial : ");
    if (inserirProduto(&p) == 0) {
        printf("Erro ao inserir o produto.\n");
        return;
    }
    printf("Produto acrescentado com sucesso. (ID %d)\n", p.id);

}

static void exibirRelatorio(void) {
    FILE *fp = fopen(ARQ_PRODUTOS, "rb");
    Produto p;
    int totalProdutos = 0;
    int totalEstoque = 0;

    printf("\n================ RELATORIO GERAL ================\n");
    printf("--- ESTOQUE DE PRODUTOS ---\n");
    if (!fp) {
        printf("Nenhum arquivo de produtos encontrado.\n");
    } else {
        while (fread(&p, sizeof p, 1, fp) == 1) {
            printf("ID: %d | %s | Preco: %.2f | Estoque: %d\n",
                   p.id, p.descricao, p.preco, p.estoque);
            totalProdutos++;
            totalEstoque += p.estoque;
        }
        fclose(fp);
        printf("Total de tipos de produtos: %d | Unidades em estoque: %d\n",
               totalProdutos, totalEstoque);
    }

    FILE *fv = fopen(ARQ_VENDAS, "rb");
    Venda v;
    int totalVendas = 0;
    float faturamentoTotal = 0.0f;

    printf("\n--- VENDAS E FATURAMENTO ---\n");
    if (!fv) {
        printf("Nenhum arquivo de vendas encontrado.\n");
    } else {
        while (fread(&v, sizeof v, 1, fv) == 1) {
            float subtotalVenda = 0.0f;
            for (int i = 0; i < v.qtdItens; i++) {
                subtotalVenda += v.quantidades[i] * v.precos[i];
            }
            printf("Venda #%d | Cliente: %s | Data: %s | Itens: %d | Total: %.2f\n",
                   v.id, v.cliente, v.data, v.qtdItens, subtotalVenda);
            totalVendas++;
            faturamentoTotal += subtotalVenda;
        }
        fclose(fv);
        printf("Total de vendas: %d | Faturamento total: %.2f\n",
               totalVendas, faturamentoTotal);
    }
    printf("=================================================\n");
}

static void pausarMenu(void) {
    printf("\nPressione ENTER para voltar ao menu...");
    fflush(stdout);
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        /* aguarda o ENTER */
    }
}

int main(void) {
    int opcao = -1;
    
    while (1) {
        printf(
        "\n========================================\n"
        "      SISTEMA DE ESTOQUE E VENDAS       \n"
        "========================================\n"
        "1 - Cadastrar produto\n"
        "2 - Listar produtos\n"
        "3 - Atualizar produto\n"
        "4 - remover produto\n"
        "5 - registrar venda\n"
        "6 - Registrar venda\n"
        "7 - buscar vendas\n"
        "8 - remover venda\n"
        "9 - Relatorio / Resumo\n"
        "0 - Sair\n"
        "----------------------------------------\n"
        "Escolha uma opcao: "
        );

        opcao = lerInt("");      
        switch (opcao) {
        case 1 : 
        cadastrarProduto();
        break;
        case 2 : 
        if (!listarProdutos()) printf("Nenhum produto cadastrado. \n");
        pausarMenu();
        break;
        case 3 : 
        atualizarProduto(lerInt("ID do produto : "));
        break;
        case 4 : 
        removerProduto(lerInt("ID do produto : "));
        break;
        case 5 : 
        cadastrarVenda();
        break;
        case 6 : 
        if (!listarVendas()) printf("Nenhuma venda cadastrada. \n");
        pausarMenu();
        break;
        case 7 : {
        Venda v;
        int id = lerInt("ID da venda : " );
        if (buscarVenda(id, &v)) exibirVendaDetalhada(&v);
        else printf("Venda não encontrada.\n");
        pausarMenu();
        break;
        }
        case 8 : 
        removerVenda(lerInt("ID da venda : "));
        break;
        case 9 :
        exibirRelatorio();
        pausarMenu();
        break;
        case 0 : 
        return 0;
        default :
        printf("Opção invalída");
        break;
        }
    }
}