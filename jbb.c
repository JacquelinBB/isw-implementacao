#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

typedef struct pageTable
{
    int frame;
    int bit;
} PageTable;

typedef struct memoriaPrincipal
{
    char binValor[256];
    struct MemoriaPrincipal *proximo;
} MemoriaPrincipal;

PageTable tamPageTable[256];
MemoriaPrincipal tamMemoria[256];

typedef struct pilha
{
    int test;
    MemoriaPrincipal *topo;
} Pilha;

int lerArquivo();                                       // OK
char *conversorBinario(int);                            // OK
void separarPage(char *copiarDaqui, char *colarAqui);   // OK
void separarOffset(char *copiarDaqui, char *colarAqui); // OK
int conversorDecimal(char bin[], int length);           // OK
void lerBackStore(int);
Pilha *criarPilha();
void push(Pilha *pilha, char aux[]);
void imprimir();
// void lotou();
// void pop();

FILE *arq;
int num[1000], i = 0, decimal = 0, pageDecimal = 0, offsetDecimal = 0, calculo;
char *ponteiroConversor;
char pageBin[32], offsetBin[32];
FILE *arqBin;
int pageFault = 0, bin;
int tlbHits = 0, valor, result, atualizar, physicalAddress;
float pageFaultRate, tlbHitRate;
char *p;

int main()
{
    Pilha *p = criarPilha();
    lerArquivo();
    for (i = 0; i < 5; i++)
    {
        do
        {
            printf("\nVirtual address: %d\n", num[i]); // CASO QUEIRA SABER QUAL O ARQUIVO
            ponteiroConversor = conversorBinario(num[i]);
            printf("Number Binary of %d is: %s\n", num[i], ponteiroConversor); // CASO QUEIRA SABER A CONVERSÃO
            separarPage(ponteiroConversor, pageBin);
            printf("Number Binary of Page is: %s\n", pageBin);
            int length1 = strlen(pageBin);
            for (int j = 0; j < length1; j++)
            {
                if (j == length1 - 1)
                {
                    pageDecimal = conversorDecimal(pageBin, length1);
                    printf("Number Decimal of Page is: %d\n", pageDecimal);
                }
            }
            separarOffset(ponteiroConversor, offsetBin);
            printf("Number Binary of Offset is: %s\n", offsetBin);
            int length2 = strlen(offsetBin);
            for (int j = 0; j < length2; j++)
            {
                if (j == length2 - 1)
                {
                    offsetDecimal = conversorDecimal(offsetBin, length2);
                    printf("Number Decimal of Offset is: %d\n", offsetDecimal);
                }
            }
            // O PAGE ESTA NA TABELA DE PAGINAS
            if (tamPageTable[pageDecimal].bit == 1)
            {
                printf("Bit corresponde a valido\n");

                printf("Physical address: %d\n", tamMemoria[tamPageTable[pageDecimal].frame * 256 + offsetDecimal]);
                valor = tamMemoria[tamPageTable[pageDecimal].frame].binValor[offsetDecimal];
                printf("Value: %d\n", valor);
            }
            // O PAGE NAO ESTA NA TABELA DE PAGINAS
            if (tamPageTable[pageDecimal].bit == 0)
            {
                printf("Bit corresponde a invalido\n");
                lerBackStore(pageDecimal);
                printf("PASSOU POR AQUI\n");
                pageFault = pageFault + 1;
                tamPageTable[pageDecimal].bit = 1;
                fclose(arqBin);
                atualizar = atualizar + 1;
            }
            free(ponteiroConversor);
        } while (atualizar < 3);
    }
    printf("%d\n", atualizar);
    fclose(arq);
    printf("Page Faults = %d\n", pageFault);
    pageFaultRate = pageFault / 1000;
    printf("Page Fault Rate = %f\n", pageFaultRate);
    printf("TLB Hits = %d\n", tlbHits);
    tlbHitRate = tlbHits / 1000;
    printf("TLB Hit Rate =  %f\n", tlbHitRate);
}

int lerArquivo()
{
    arq = fopen("addresses.txt", "r");

    if (arq == NULL)
    {
        printf("ERRO: Em ler o arquivo.\n");
        exit(0);
    }
    else
    {
        printf("Arquivo addresses.txt foi aberto com sucesso.\n");
        for (i = 0; i < 1000; i++)
        {
            fscanf(arq, "%d,", &num[i]);
        }
    }

    return 0;
}

char *conversorBinario(int num)
{
    int n, c, d, count, j;

    count = 0;
    ponteiroConversor = (char *)malloc(32 + 1);

    if (ponteiroConversor == NULL)
        exit(1);

    for (c = 31; c >= 0; c--)
    {
        d = num >> c;

        if (d & 1)
            *(ponteiroConversor + count) = 1 + '0';
        else
            *(ponteiroConversor + count) = 0 + '0';

        count++;
    }
    *(ponteiroConversor + count) = '\0';

    return ponteiroConversor;
}

void separarPage(char *copiarDaqui, char *colarAqui)
{
    while (*copiarDaqui != '\0' && *(copiarDaqui + 8))
    {
        *colarAqui = *copiarDaqui;
        ++copiarDaqui;
        ++colarAqui;
    }
    *colarAqui = '\0';

    return 0;
}

void separarOffset(char *copiarDaqui, char *colarAqui)
{
    while (*copiarDaqui != '\0')
    {
        *colarAqui = *(copiarDaqui + 24);
        ++copiarDaqui;
        ++colarAqui;
    }
    *colarAqui = '\0';

    return 0;
}

int conversorDecimal(char bin[], int length)
{
    int lugar = 0, index = length - 1;
    decimal = 0;

    while (index >= 0)
    {
        decimal = decimal + (bin[index] - 48) * pow(2, lugar);
        index--;
        lugar++;
    }

    return decimal;
}

void lerBackStore(int pageDecimal)
{
    char aux[256];
    arqBin = fopen("BACKING_STORE.bin", "rb");
    calculo = pageDecimal * 256;

    if (arqBin == NULL)
    {
        printf("ERRO: O arquivo binario.bin não foi aberto e criado.\n");
        exit(1);
    }
    else
    {
        printf("Arquivo binario.bin foi aberto com sucesso.\n");
        fseek(arqBin, calculo, SEEK_SET);
        // PRINTANDO A POSICAO DO PONTEIRO
        // printf("%ld\n", ftell(arqBin));

        fread(aux, 256, 1, arqBin);

        for (int i = 0; i < 256; i++)
        {
            printf("%d ", aux[i]);
        }

        push(p, aux);
    }

    return 0;
}

Pilha *criarPilha()
{
    Pilha *pilha = (Pilha *)malloc(sizeof(Pilha));
    if (!pilha)
    {
        printf("Sem memoria disponivel!\n");
        exit(0);
    }
    pilha->topo = NULL;
    pilha->test = 10;
    return pilha;
}

void push(Pilha *pilha, char aux[])
{
    printf("\nTA AQUI 4\n");
    MemoriaPrincipal *memoriaPrincipal = (MemoriaPrincipal *)malloc(sizeof(MemoriaPrincipal));
    printf("TA AQUI 5\n");
    for (int i = 0; i < 256; i++)
    {
        memoriaPrincipal->binValor[i] = aux[i];
    }
    printf("TA AQUI 6\n\n");
    printf("%d ", pilha->test);
    memoriaPrincipal->proximo = pilha->topo;
    printf("\n\nTA AQUI 7\n");
    pilha->topo = memoriaPrincipal;
    printf("TA AQUI 8\n");
    // imprimir();
}

void imprimir()
{
    printf("TA AQUI 9\n");
    MemoriaPrincipal *p = p;
    for (int i; i < 256; i++)
    {
        printf("\n%s", p->binValor);
        p = p->proximo;
    }
    printf("\n");
}

/*
void pop(Pilha *pilha)
{
    memoriaPrincipal *pointer = pilha->topo;
    pilha->topo = pointer->proximo;
    printf("%d\n", pointer->aux);
    free(pointer);
}
*/
