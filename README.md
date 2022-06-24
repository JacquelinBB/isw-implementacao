<h1 align="center">Implementação de Infraestrutura de software</h1>
:question: Projetando uma Memória Virtual

# Sumário
- [Como clonar o repositório](#como-clonar-o-repositório)
- [Como compilar o programa](#como-compilar-o-programa)
- [Como testar o programa](#como-testar-o-programa)
- [Como executar o programa](#como-executar-o-programa)

##### Como implementar o projeto
# Como clonar o repositório
```bash
$ git clone https://github.com/JacquelinBB/isw-implementacao.git
```
# Como compilar o programa
```bash
$ cd "nome do aquivo" 
$ make
```
# Como testar o programa
- testff:       ./vm addresses.txt fifo fifo
- testll:       ./vm addresses.txt lru lru
- testfl:       ./vm addresses.txt fifo lru
- testlf:       ./vm addresses.txt lru fifo
- testerro1:    ./vm addresses.txt parametro errado
- testerro2:    ./vm addresses.txt quantidade de parametro
- testerro3:    ./vm addresses.txt fifo fifo f
- testerro4:    ./vm addresses.txt fifo
- testerro5:    ./vm addresses.doc fifo fifo
# Como executar o programa
```bash
$ make "nome do teste"
$ make clean
```
*Para um novo teste novamente, depois do make clean, de make novamente, e repita o processo citado acima em "Como executar o programa"*
## Arquivos .c utilizados:
- jbb.c

## Outros arquivos utilizados:
- Makefile
- addresses.txt
- BACK_STORE.bin

## Sistema operacional que foi implementado:
*Foi utilizado o sistema operacional Linux para a implementação*
