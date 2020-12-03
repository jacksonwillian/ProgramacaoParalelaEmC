# Atividade 4

resolva o jantar dos filosofos classico evitando deadlocks.
faca uma versao que tb aceite como argumentos o numero de filosofos dentro do circulo, todo o resto do problema se mantem.
Como acabar o sistema? Verifique se cada filosofo comeu pelo menos X vezes [outro parametro q pode ser usado como argumento] rs


uma boa forma de testar esse codigo, eh que sempre que um filosofo come, ele printe na tela algo como:

Filosofo X esta comendo.

e sempre que ele esteja esperando para comer [pq nao conseguiu os recursos para isso] ele imprima algo como:

Filosofo X esta pensando...
[e obviamente , se vc nao colocar essa thread do que esta pensando pra esperar um pouquinho, ou seja, Sleep ou algum outro tipo de solucao interessante, vcs vao ter concorrencia ferrenha, eh legal testar primeiro sem o Sleep pra voces verem o banho de sangue q fica pra cada um comer e esperar rsrsr]

claro que colocar variaveis de contabilizacao de quantas vezes cada filosofo comeu eh interessante para verificar starvation.

deadlocks podem ser notados a medida que o sistema pare de imprimir qualquer coisa ne? :D rs

LiveLocks tb, pois se um dos filosofos nao come depois de algum bom tempo , eh pq algo ta bem errado.

Obs.: Para criar o executável:

        gcc -Wall -pedantic -o <binario> <codigofonte> -lpthread

     Para executar:
        
        ./<binario> <quantidadeFilosofos> <capacidadeFilosofos>
