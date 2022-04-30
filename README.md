# projet_rtos
Réalisation d'un progra Arduino  basé sur FreeRTOS qui permet de cré  les 5 tâches suivantes :
1-Récuperation d'une valeu Analogique entre 0 et 1023 d'un potentiomètre  branché sur A0  ensuite envoyé la valeur à la 3ème tâche.
2-Récuperation d'une valeur numérique entre 0 et 2  qui est la somme des deux valeurs  des deux entré numérique 3 et 4  branché à des boutons poussoir puis  envoyé la valeur à la 3ème tâche
3-Stocker les deux valeurs reçu dans une structure et l'envoyé à la 4ème tâche.
4-Reception e t affichage des donneé.
5-Réception des donnée et conversion du temps en minute puis affichage.

Utilisation des mutex pour protéger l’utilisation du port série et les Queues pour communiquer les informations entre les différentes tâches.
