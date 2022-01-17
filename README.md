# 💻  ENSEA-shell

Vous trouverez ici les [travaux réalisés](/questions) lors du TP1 de majeure Informatique. <br>
Le [script](/enseash.c) rédigé permet d'exécuter des commandes simples, et de connaître leur temps d'exécution et code de sortie. <br> <br>

Parmi les fonctionnalités implémentées, on trouve la possibilité d'exécuter des commandes contenant des arguments, de rediriger les entrées et sorties de la console avec les chevrons **`>`** et **`<`** et également la possibilité de connecter deux commandes entre elles avec **`|`**. <br>
Seul manque la possibilité de placer des programmes en arrière-plan. Cela peut être implémenté avec la commande `setpgid(0, 0)` qui permet de mettre l'enfant dans un autre process. Ainsi le père n'a pas a attendre.

