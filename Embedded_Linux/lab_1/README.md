# Lab 1 - Cross Compilation


## Table of contents
- [Lab 1 - Cross Compilation](#lab-1---cross-compilation)
  - [Table of contents](#table-of-contents)
  - [Instructions](#instructions)
  - [Questions](#questions)
    - [Q1](#q1)
    - [Q2](#q2)
    - [Q3](#q3)
    - [Q4](#q4)
    - [Q5](#q5)
    - [Q6](#q6)
    - [Q7](#q7)
    - [Q8](#q8)
    - [Q9](#q9)
    - [Q10](#q10)
    - [Q11](#q11)
    - [Q12](#q12)
    - [Q13](#q13)
  - [Creators](#creators)
  - [Copyright and license](#copyright-and-license)


## Instructions

[Lab1 - Chaîne de compilation croisée](https://gitedu.hesge.ch/flg_courses/embedded_linux/embedd_linux_pub_fall23/-/blob/master/labs/lab01-cross_compilation.pdf)

## Questions

### Q1
#### Quelle est l’architecture processeur de la carte SAMA5D3 Xplained ?

D'après ça datasheet, la carte SAMS5D3 est basée sur un ARM Cortex-A5 avec un set d'instruction ARM V7-A Thumb2.

### Q2
#### Quel est le nom complet de votre chaîne de compilation croisée ?

Ma chaîne de compilation s'appelle : `armv7-eabihf--uclibc--stable-2020.08-1`.

Elle peut être téléchargée [ici](https://toolchains.bootlin.com/downloads/releases/toolchains/armv7-eabihf/tarballs/armv7-eabihf--uclibc--stable-2020.08-1.tar.bz2).

### Q3
#### Quelle est la version complète du compilateur gcc de votre chaîne de compilation croisée (option -v) ?

Grâce à la commande `arm-linux-gcc --version` nous voyons que la version de gcc est : `arm-linux-gcc.br_real (Buildroot 2020.08-14-ge5a2a90) 9.3.0`.

### Q4
#### Quelle est la version de la librairie uclibc-ng ? Pour confirmer le numéro de version, explorez le répertoire sysroot/lib. A noter que ls -l permet d’afficher la cible d’un lien symbolique.

Dans le dossier `sysroot/lib` nous trouvons le fichier `ld-uClibc-1.0.34.so`.

### Q5
#### Quelle est la version des linux headers ?

`#define LINUX_VERSION_CODE 264682` donc `264682` -> `0100 0000 1001 1110 1010`. <br>
`1110 1010` -> 234<br>
`0000 1001` -> 9<br>
`0000 0100` -> 4<br>

donc la version est `4.9.234`.

### Q6
#### Dans quel répertoire (chemin à partir du répertoire racine où vous avez décompressé votre toolchain) se trouve les binaires principaux de votre toolchain ? (compilateur, linker, assembleur, etc.)

Tout ces éléments peuvent être trouvé dans le répertoire suivant : `toolchain/armv7-eabihf--uclibc--stable-2020.08-1/bin/`.

### Q7
#### Quel est le nom (le nom de l’exécutable) du compilateur C ? Attention à ne pas donner un lien symbolique.

`arm-linux-gcc`->`toolchain-wrapper`.

### Q8
#### Quelle est la taille de l’exécutable obtenu ?

Pour le programme `Hello World` suivant :
~~~
#include <stdio.h>

void main()
{
  printf("Hello World !");
}
~~~

et en le compilant avec la commande `arm-linux-gcc hello_world.c -o hello_world`, le fichier .c fait 64 octets et l'exécutable fait 7,4 ko.

### Q9
#### Que se passe-t-il si vous essayez d’exécuter l’exécutable obtenu sur votre PC de développement et pourquoi ?

Si j'essaye d'exécuté mon `Hello Word` je reçois l'erreur suivante : `bash: ./hello_world : impossible d'exécuter le fichier binaire : Erreur de format pour exec()`. <br>Étant donné que nous utilisons chaîne de compilation croisée le programme sera fait pour être compilé sur une architecture mais exécuté sur une autre.

### Q10
#### Comment pouvez-vous vous assurer que le code binaire généré correspond bien à l’architecture ARM (ELF 32-bit LSB executable) ? (indices : file, readelf)

`file hello_world` -> `hello_world: ELF 32-bit LSB executable, ARM, EABI5 version 1 (SYSV), dynamically linked, interpreter /lib/ld-uClibc.so.0, not stripped`
<br><br>
Nous voyons donc bien `ELF 32-bit LSB executable`.

### Q11
#### A titre de comparaison, compilez à nouveau votre petit programme C en spécifiant l’argument -static à gcc. Quelle est la taille de ce nouvel exécutable ?

En le compilant avec la commande `arm-linux-gcc hello_world.c -o hello_world_static -static`, le nouvel exécutable fait maintenant 138 ko.

### Q12
#### Comment pouvez-vous expliquez la différence de taille avec l’exécutable obtenu sans l’option -static ?

Grâce à l'option `-static`, notre nouvel exécutable comprendra le code complet de toutes les librairies dont nous aurions besoin pour l'exécuter.

### Q13
#### Quelle est la taille de chaque exécutable après la suppression des symboles avec strip ?

Après avoir exécuté `arm-linux-strip hello_world && arm-linux-strip hello_world_static`, `hello_world` fait maintenant 5.3 ko et `hello_world_static` fait 83.6 ko.

## Creators

**Michael Divià**

- <https://gitedu.hesge.ch/michael.divia>

## Copyright and license

Code and documentation copyright 2023 the authors. Code released under the [MIT License](https://gitedu.hesge.ch/flg_courses/embedded_linux/students/michael.divia/-/blob/master/LICENSE).

Enjoy :metal: