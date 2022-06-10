#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define MAX_SIZE 128

typedef struct pageTable
{
    int frame;
    int bit;
    int time;
} PageTable;

typedef struct tlb
{
    int tlbFrame;
    int tlbPage;
    int tlbBit;
    int tlbTime;
} Tlb;

typedef struct memoriaPrincipal
{
    char binValor[256];
} MemoriaPrincipal;

PageTable tamPageTable[256];
MemoriaPrincipal tamMemoria[MAX_SIZE];
Tlb tamTlb[16];

char *conversorBinario(int);
int separarPage(char *copiarDaqui, char *colarAqui);
int separarOffset(char *copiarDaqui, char *colarAqui);
int conversorDecimal(char bin[], int length);
int lerBackStore(int);
int lerBackStoreCheio(int pageDecimal, int pos);

FILE *arq;
FILE *arqBin;
int num[1000], i = 0, decimal = 0, pageDecimal = 0, offsetDecimal = 0, calculo, translatedAddresses = 0, posicao, numInstrucao;
char *ponteiroConversor;
char pageBin[32], offsetBin[32];
float pageFaultRate, tlbHitRate, pageFault = 0, tlbHits = 0, tlbMiss = 0;
int indice = 0, position = 0, contador = 0, k = -1, positionTlb = 0, contadorTlb = 0, positionNew = 0, stop, teste = 0, k2 = -1, countTlb = 0;

int main(int argc, char *argv[])
{
    arq = fopen("addresses.txt", "r");
    fpos_t posicao;
    fgetpos(arq, &posicao);
    while (fscanf(arq, "%d", &numInstrucao) != EOF)
    {
        int pos;
        translatedAddresses = translatedAddresses + 1;
        ponteiroConversor = conversorBinario(numInstrucao);
        // printf("---Number Binary of %d is: %s---", numInstrucao, ponteiroConversor);
        separarPage(ponteiroConversor, pageBin);
        // printf("---Number Binary of Page is: %s---", pageBin);
        int length1 = strlen(pageBin);
        pageDecimal = conversorDecimal(pageBin, length1);
        // printf("---Number Decimal of Page is: %d---", pageDecimal);
        separarOffset(ponteiroConversor, offsetBin);
        // printf("---Number Binary of Offset is: %s---", offsetBin);
        int length2 = strlen(offsetBin);
        offsetDecimal = conversorDecimal(offsetBin, length2);
        // printf("---Number Decimal of Offset is: %d---", offsetDecimal);
        free(ponteiroConversor);

        stop = 0;
        for (int j = 0; j < 16; j++)
        {
            // ESTÁ NA TLB E DEU TLB HIT
            if (tamTlb[j].tlbPage == pageDecimal)
            {
                stop = 1;
                tlbHits = tlbHits + 1;
                printf("Virtual address: %d ", numInstrucao);
                printf("Physical address: %u ", tamPageTable[pageDecimal].frame * 256 + offsetDecimal);
                printf("Value: %d\n", tamMemoria[tamPageTable[pageDecimal].frame].binValor[offsetDecimal]);
                fgetpos(arq, &posicao);
                break;
            }
        }
        if (stop != 0)
        {
        
        }

        // O PAGE NAO ESTÁ NA TABELA DE PAGINAS E DEU PAGE FAULT
        else if (tamPageTable[pageDecimal].bit == 0)
        {
            // printf("---Bit corresponde a invalido---");
            pageFault = pageFault + 1;

            // MEMÓRIA CHEIA
            if (indice == MAX_SIZE)
            {
                if (!strcmp(argv[2], "fifo"))
                {
                    if (position == 128)
                    {
                        position = 0;
                    }
                    for (int j = 0; j < 256; j++)
                    {
                        if (tamPageTable[j].frame == position)
                        {
                            tamPageTable[j].bit = 0;
                            tamPageTable[j].frame = -1;
                        }
                    }
                    tamPageTable[pageDecimal].bit = 1;
                    tamPageTable[pageDecimal].frame = position;
                    lerBackStore(pageDecimal);
                    position = position + 1;
                }
                if (!strcmp(argv[2], "lru"))
                {
                    int menor = contador;
                    for (int j = 0; j < 256; j++)
                    {
                        if (tamPageTable[j].time < menor && tamPageTable[j].bit != 0)
                        {
                            menor = tamPageTable[j].time;
                            pos = tamPageTable[j].frame; // PARA SUBSTITUIR
                            k = j;
                        }
                    }
                    lerBackStoreCheio(pageDecimal, pos); // SUBSTITUIÇÃO
                    // APAGAR ANTIGO
                    tamPageTable[k].bit = 0;
                    tamPageTable[k].frame = -1;
                    // ATUALIZAR NOVO
                    tamPageTable[pageDecimal].bit = 1;
                    tamPageTable[pageDecimal].frame = pos;
                    // TEMPO GLOBAL
                    tamPageTable[pageDecimal].time = contador;
                }
            }
            else
            {
                tamPageTable[pageDecimal].bit = 1;
                tamPageTable[pageDecimal].frame = indice;
                lerBackStore(pageDecimal);
                tamPageTable[pageDecimal].time = contador;
                indice = indice + 1;
            }
            translatedAddresses = translatedAddresses - 1;
            fsetpos(arq, &posicao);
        }

        // O PAGE ESTÁ NA TABELA DE PAGINAS
        else if (tamPageTable[pageDecimal].bit == 1)
        {
            tlbMiss = tlbMiss + 1;
            tamPageTable[pageDecimal].time = contador;
            printf("Virtual address: %d ", numInstrucao);
            printf("Physical address: %u ", tamPageTable[pageDecimal].frame * 256 + offsetDecimal);
            printf("Value: %d\n", tamMemoria[tamPageTable[pageDecimal].frame].binValor[offsetDecimal]);
            fgetpos(arq, &posicao);

            // ATUALIZAR NA TLB SEM LOTAR
            if (positionTlb < 16)
            {
                tamTlb[positionTlb].tlbBit = 1;
                tamTlb[positionTlb].tlbFrame = indice;
                tamTlb[positionTlb].tlbPage = pageDecimal;
            }
            positionTlb = positionTlb + 1;
            // ATUALIZAR NA TLB LOTADA
            if (positionTlb == 16)
            {
                if (!strcmp(argv[3], "fifo"))
                {
                    positionTlb = 0;
                }
                if (!strcmp(argv[3], "lru"))
                {
                    int menorTlb = countTlb;
                    for (int j = 0; j < 16; j++)
                    {
                        if (tamTlb[j].tlbTime < menorTlb && tamTlb[j].tlbBit != 0)
                        {
                            menorTlb = tamTlb[j].tlbTime;
                            pos = tamTlb[j].tlbFrame; // PARA SUBSTITUIR
                            k2 = j;
                        }
                    }
                    // APAGAR ANTIGO
                    tamTlb[k2].tlbBit = 0;
                    tamTlb[k2].tlbFrame = -1;
                    // ATUALIZAR NOVO
                    tamTlb[positionTlb].tlbBit = 1;
                    tamTlb[positionTlb].tlbFrame = pos;
                    // TEMPO GLOBAL
                    tamTlb[positionTlb].tlbTime = countTlb;
                }
            }
        }
        countTlb++;
        contador++;
    }
    fclose(arq);
    printf("%d\n", teste);
    // tlbHits = 1000 - tlbMiss;
    printf("Number of Translated Addresses = %d\n", translatedAddresses);
    printf("Page Faults = %.0f\n", pageFault);
    pageFaultRate = pageFault / 1000;
    printf("Page Fault Rate = %.3f\n", pageFaultRate);
    printf("TLB Miss = %.0f\n", tlbMiss);
    printf("TLB Hits = %.0f\n", tlbHits);
    tlbHitRate = tlbHits / 1000;
    printf("TLB Hit Rate =  %.3f\n", tlbHitRate);
}

char *conversorBinario(int num)
{
    int c, d, count;

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

int separarPage(char *copiarDaqui, char *colarAqui)
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

int separarOffset(char *copiarDaqui, char *colarAqui)
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
    int i = 0, decimal = 0;

    for (i = 0; i < length; i++)
    {
        decimal += (bin[length - i - 1] == '1') ? (1 << i) : 0;
    }

    return decimal;
}

int lerBackStore(int pageDecimal)
{
    arqBin = fopen("BACKING_STORE.bin", "rb");
    calculo = pageDecimal * 256;

    if (arqBin == NULL)
    {
        printf("ERRO: O arquivo binario.bin não foi aberto e criado.\n");
        exit(1);
    }
    else
    {
        // printf("Arquivo binario.bin foi aberto com sucesso.\n");
        fseek(arqBin, calculo, SEEK_SET);
        fread(tamMemoria[tamPageTable[pageDecimal].frame].binValor, 256, 1, arqBin);
        fclose(arqBin);
    }
    return 0;
}

int lerBackStoreCheio(int pageDecimal, int pos)
{
    arqBin = fopen("BACKING_STORE.bin", "rb");
    calculo = pageDecimal * 256;

    if (arqBin == NULL)
    {
        printf("ERRO: O arquivo binario.bin não foi aberto e criado.\n");
        exit(1);
    }
    else
    {
        // printf("Arquivo binario.bin foi aberto com sucesso.\n");
        fseek(arqBin, calculo, SEEK_SET);
        fread(tamMemoria[pos].binValor, 256, 1, arqBin);
        fclose(arqBin);
    }
    return 0;
}
