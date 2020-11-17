# Atividade 1

Faca um somatorio de um vetor imenso com multiplas threads [tudo deve ser passado como argumento] SEM USAR NENHUMA VARIAVEL GLOBAL e fazendo sua thread RETORNAR os valores a serem acumulador na main para minimizar as condicoes de corrida.

compare os tempos com o das versoes globais e va aumentando o numero de threads para ver os ganhos q vc consegue ter ate o ponto q comeca a ser ruim.

deve ser entregue aqui SOMENTE OS .C e o .txt com o resultado EM ARQUIVO COMPACTADO.


padrao a ser usado no txt ABAIXO:


| THREADS  | CLICKS | TAMANHO VETOR |
| :------: |:------:| :-----------: |
| 1        | 280    | 100000        |
| 2        | 278    | 100000        |
| 3        | 260    | 100000        |


Obs.: Para criar o executável:

        gcc -Wall -pedantic -o <binario> <codigofonte> -lpthread