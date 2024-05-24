# Luma3DS

*"Custom Firmware" pour Nintendo 3DS*

## Qu'est-ce que c'est ?
**Luma3DS** est un programme qui corrige et réimplémente des parties importantes du logiciel fonctionnant sur tous les modèles de la famille de consoles Nintendo 3DS.

Il vise à améliorer considérablement l'expérience de l'utilisateur et à soutenir la 3DS bien au-delà de sa fin de vie. Les caractéristiques sont les suivantes :
* Prise en charge de premier ordre du homebrew 3DSX
* Un menu superposé appelé « Rosalina » (déclenchable par <kbd>L+Down+Select</kbd> par défaut), permettant entre autres de prendre des captures d'écran en cours de jeu.
* Suppression des restrictions telles que le verrouillage régional
* Paramètres de langue par jeu, redirection du chemin d'accès au contenu des actifs (LayeredFS), plugins de jeu...
* Un stub GDB à part entière permettant de déboguer des logiciels (homebrew et logiciels système)
* ... et bien plus encore!

Luma3DS nécessite un exploit persistant sur l'ensemble du système, tel que [boot9strap](https://github.com/SciresM/boot9strap) pour l'exécuter.

## Compilation

Pour compiler Luma3DS, les éléments suivants sont nécessaires :
* git
* Avoir à jour devkitARM et libctru
* [makerom](https://github.com/jakcron/Project_CTR) dans PATH
* [firmtool](https://github.com/TuxSH/firmtool) installé

Le fichier `boot.firm` produit est destiné à être copié à la racine de votre carte SD pour être utilisé avec Boot9Strap.

## Configuration / Utilisation / Fonctionnalités
Voir https://github.com/LumaTeam/Luma3DS/wiki (doit être retravaillé)

## Crédits
Voir https://github.com/LumaTeam/Luma3DS/wiki/Credits (doit être retravaillé)

## Licences
Ce logiciel est sous licence GPLv3. Vous trouverez une copie de la licence dans le fichier LICENSE.txt.

Les fichiers dans le stub GDB sont plutôt sous triple licence MIT ou « GPLv2 ou toute version ultérieure », dans ce cas c'est spécifié dans l'en-tête du fichier.

En contribuant à ce dépôt, vous acceptez de céder vos modifications aux propriétaires du projet.
