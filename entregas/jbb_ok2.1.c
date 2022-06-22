#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define MAX_SIZE 256

typedef struct pageTable
{
    int frame;
    int bit;
} PageTable;

typedef struct memoriaPrincipal
{
    char binValor[MAX_SIZE];
} MemoriaPrincipal;

PageTable tamPageTable[MAX_SIZE];
MemoriaPrincipal tamMemoria[MAX_SIZE];

int lerArquivo();                                      // OK
char *conversorBinario(int);                           // OK
int separarPage(char *copiarDaqui, char *colarAqui);   // OK
int separarOffset(char *copiarDaqui, char *colarAqui); // OK
int conversorDecimal(char bin[], int length);          // OK
int lerBackStore(int);                                 // OK

FILE *arq;
FILE *arqBin;
int numInstrucao, i = 0, decimal = 0, pageDecimal = 0, offsetDecimal = 0, calculo, translatedAddresses = 0;
char *ponteiroConversor;
char pageBin[32], offsetBin[32];
float pageFaultRate, tlbHitRate, pageFault = 0, tlbHits = 0;
int indice = 0;

int main()
{
    lerArquivo();
    fpos_t posicao;
    
    while (fscanf(arq, "%d", &numInstrucao) != EOF)
    {
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

        // O PAGE ESTA NA TABELA DE PAGINAS
        if (tamPageTable[pageDecimal].bit == 1)
        {
            // printf("---Bit corresponde a valido---");
            // printf("---Check indice: %d ---", tamPageTable[pageDecimal].frame);
            printf("Virtual address: %d ", numInstrucao);
            printf("Physical address: %u ", tamPageTable[pageDecimal].frame * 256 + offsetDecimal);
            printf("Value: %d\n", tamMemoria[tamPageTable[pageDecimal].frame].binValor[offsetDecimal]);
            fgetpos(arq, &posicao);
        }

        // O PAGE NAO ESTA NA TABELA DE PAGINAS
        if (tamPageTable[pageDecimal].bit == 0)
        {
            // printf("---Bit corresponde a invalido---");
            tamPageTable[pageDecimal].frame = indice;
            lerBackStore(pageDecimal);
            indice = indice + 1;
            pageFault = pageFault + 1;
            tamPageTable[pageDecimal].bit = 1;
            translatedAddresses = translatedAddresses - 1;
            fsetpos(arq, &posicao);
        }
    }
    fclose(arq);
    printf("Number of Translated Addresses = %d\n", translatedAddresses);
    printf("Page Faults = %.0f\n", pageFault);
    pageFaultRate = pageFault / 1000;
    printf("Page Fault Rate = %.3f\n", pageFaultRate);
}

int lerArquivo()
{
    arq = fopen("addresses.txt", "r");

    if (arq == NULL)
    {
        printf("ERRO: Em ler o arquivo.\n");
        exit(0);
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
