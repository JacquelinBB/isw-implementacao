#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define MAX_SIZE 128

typedef struct pageTable
{
    int frame;
    int bit;
} PageTable;

typedef struct memoriaPrincipal
{
    char binValor[256];
} MemoriaPrincipal;

PageTable tamPageTable[256];
MemoriaPrincipal tamMemoria[MAX_SIZE];

int lerArquivo();                                       // OK
char *conversorBinario(int);                            // OK
void separarPage(char *copiarDaqui, char *colarAqui);   // OK
void separarOffset(char *copiarDaqui, char *colarAqui); // OK
int conversorDecimal(char bin[], int length);           // OK
void lerBackStore(int);                                 // OK

FILE *arq;
FILE *arqBin;
int num[1000], i = 0, decimal = 0, pageDecimal = 0, offsetDecimal = 0, calculo, translatedAddresses = 0;
char *ponteiroConversor;
char pageBin[32], offsetBin[32];
float pageFaultRate, tlbHitRate, pageFault = 0, tlbHits = 0;
int indice = 0, index = 0;

int main()
{
    lerArquivo();
    for (i = 0; i < 1000; i++)
    {
        translatedAddresses = translatedAddresses + 1;
        ponteiroConversor = conversorBinario(num[i]);
        // printf("---Number Binary of %d is: %s---", num[i], ponteiroConversor);
        separarPage(ponteiroConversor, pageBin);
        // printf("---Number Binary of Page is: %s---", pageBin);
        int length1 = strlen(pageBin);
        for (int j = 0; j < length1; j++)
        {
            if (j == length1 - 1)
            {
                pageDecimal = conversorDecimal(pageBin, length1);
                // printf("---Number Decimal of Page is: %d---", pageDecimal);
            }
        }
        separarOffset(ponteiroConversor, offsetBin);
        // printf("---Number Binary of Offset is: %s---", offsetBin);
        int length2 = strlen(offsetBin);
        for (int j = 0; j < length2; j++)
        {
            if (j == length2 - 1)
            {
                offsetDecimal = conversorDecimal(offsetBin, length2);
                // printf("---Number Decimal of Offset is: %d---", offsetDecimal);
            }
        }
        free(ponteiroConversor);

        // O PAGE NAO ESTA NA TABELA DE PAGINAS
        if (tamPageTable[pageDecimal].bit == 0)
        {
            // printf("---Bit corresponde a invalido---");
            pageFault = pageFault + 1;

            // FIFO
            if (indice == MAX_SIZE)
            {
                if (index == 128)
                {
                    index = 0;
                }
                for (int j = 0; j < 256; j++)
                {
                    if (tamPageTable[j].frame == index)
                    {
                        tamPageTable[j].bit = 0;
                        tamPageTable[j].frame = -1;
                    }
                }
                tamPageTable[pageDecimal].bit = 1;
                tamPageTable[pageDecimal].frame = index;
                lerBackStore(pageDecimal);
                index = index + 1;
            }
            else
            {
                tamPageTable[pageDecimal].bit = 1;
                tamPageTable[pageDecimal].frame = indice;
                lerBackStore(pageDecimal);
                indice = indice + 1;
            }
        }

        // O PAGE ESTA NA TABELA DE PAGINAS
        if (tamPageTable[pageDecimal].bit == 1)
        {
            // printf("---Bit corresponde a valido---");
            printf("Virtual address: %d ", num[i]);
            printf("Physical address: %u ", tamPageTable[pageDecimal].frame * 256 + offsetDecimal);
            printf("Value: %d\n", tamMemoria[tamPageTable[pageDecimal].frame].binValor[offsetDecimal]);
        }
    }
    fclose(arq);
    printf("Number of Translated Addresses = %d\n", translatedAddresses);
    printf("Page Faults = %.0f\n", pageFault);
    pageFaultRate = pageFault / 1000;
    printf("Page Fault Rate = %.3f\n", pageFaultRate);
    printf("TLB Hits = %.0f\n", tlbHits);
    tlbHitRate = tlbHits / 1000;
    printf("TLB Hit Rate =  %.3f\n", tlbHitRate);
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
        // printf("Arquivo addresses.txt foi aberto com sucesso---");
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
