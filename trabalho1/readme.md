# Trabalho 1

Existem 6 processos nesse problema: 3 processos infectados e 3 processos laboratorios.

Cada um dos infectados vai fazer uma vacina e usa-la [a vacina eh uma porcaria e tem q ser reaplicada a todo momento que for possivel].

Para fazer uma vacina eh necessario virus-morto, injecao e insumo-secreto.

Cada infectado tem 1 dos 3 produtos.

Exemplo: 1 processo infectado tem virus-morto, outro tem injecao e o ultimo tem o insumo-secreto.

Cada laboratorio tem um diferente suprimento infinito de 2 dos 3 produtos.
* Exemplo:laboratorio1 tem injecao, virus-morto, laboratorio2 tem injecao e insumo-secreto e laboratorio3 tem insumo-secreto e virus-morto.

Cada laboratorio coloca 2 dos produtos a disposicao de quem quiser e todos os infectados correm pra pegar os dois que eles precisam pra fazer a vacina e somente renovam quando os SEUS produtos distribuidos forem consumidos. [o lab2 não renova se consumir a injeção do lab1 e o insumo secreto do lab3, por exemplo]


* Como garantir exclusao mutua, sem dead-lock e sem starvation ?

[dica, soh vai funcionar com uma mescla de semaforos e mutexes]

a entrada do codigo será um numero que vai ser o numero de vezes MINIMO que cada thread realizou seu objetivo primordial.

A saida deverá ser o numero de vezes que cada thread realizou seu objetivo primordial:
no caso, os infectados irao contabilizar o numero de vezes q cada um se injetou com a vacina e os laboratorios o numero de vezes que cada um renovou seus estoques:
Exemplo de de execucao:

    ./covid-19 90  
    infectado 1: 120
    infectado 2: 100
    infectado 3: 90
    laboratorio 1: 120
    laboratorio 2: 100
    laboratorio 3: 90

Obs.: Ira executar ate todas as threads terem executado seu objetivo PELO MENOS 90 vezes.
Isso eh um exemplo de MOLDE e em nenhum momento condiz com a real saida de numeros esperada.

gcc -Wall -pedantic -o <executavel> <codigos fontes> -lpthread