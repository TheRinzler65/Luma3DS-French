Remplacement du chargeur 3DS
======================

Il s'agit d'une implémentation open source du module système 3DS `loader`, avec 
des fonctionnalités supplémentaires. Le but actuel du projet est de fournir 
un point d'entrée agréable pour patcher les modules 3DS.

## Roadmap
Pour l'instant, il peut servir de substitut open-source au chargeur intégré. 
Il y a un support additionnel pour patcher n'importe quel exécutable après qu'il soit chargé mais avant qu'il ne démarre. 
Par exemple, vous pouvez patcher `menu` pour sauter les vérifications de régions et avoir
un jeu sans région qui se lance directement depuis le menu d'accueil. 
Il y a aussi un support pour la lecture de SDMC (non trouvé dans l'implémentation originale du loader)
ce qui signifie que les patches peuvent être chargés à partir de la carte SD. 
En fin de compte, il y aurait un système de patch qui supporterait le chargement facile de patchs depuis la carte SD.

## Build
Vous avez besoin d'un environnement de construction 3DS fonctionnel avec une copie
assez récente de devkitARM, ctrulib, et makerom. Si vous voyez des erreurs dans le processus de construction,
il est probable que vous utilisiez une ancienne version.

Actuellement, il n'y a pas de support pour la construction de FIRM, vous devez donc effectuer certaines étapes manuellement.
Tout d'abord, vous devez ajouter du rembourrage pour vous assurer que la CNH est de la bonne taille pour la remplacer.
Un moyen détourné est [Ce Patch](http://pastebin.com/nyKXLnNh) qui ajoute des données inutiles. Jouez avec la valeur de
la taille pour que la NCCH ait exactement la même taille que celle trouvée dans votre fichier FIRM décrypté.

Une fois que vous avez un NCCH de la bonne taille, il vous suffit de le remplacer dans
votre FIRM décrypté et de trouver un moyen de le lancer (par exemple avec ReiNAND).
