# Lab 0 - Preparation


## Table of contents
- [Lab 0 - Preparation](#lab-0---preparation)
  - [Table of contents](#table-of-contents)
  - [Instructions](#instructions)
  - [Questions](#questions)
    - [Q1](#q1)
    - [Q2](#q2)
  - [Creators](#creators)
  - [Copyright and license](#copyright-and-license)


## Instructions

[Lab0 - Préparation initiale](https://gitedu.hesge.ch/flg_courses/embedded_linux/embedd_linux_pub_fall23/-/blob/master/labs/lab00-preparation.pdf)

## Questions

### Q1
#### Quel est le rôle des symboles `&&` ci-dessus ?

Les symboles `&&` permettent d'exécuter plusieurs commandes à la suite dans un terminal. Nous exécutons donc ici `sudo apt-get update` pui `sudo apt-get -y upgrade` de manière autonome.

### Q2
#### Pourquoi passer `-y` à `apt-get upgrade` ?

Dans le cas où nous aurions effectivement des paquets à upgrade, le terminal nous demandera confirmation si nous voulons ou non les installer. L'argument `-y` permet de bypass cette confirmation et de directement commencer à mettre a jour tout les paquets qui le demande.

## Creators

**Michael Divià**

- <https://gitedu.hesge.ch/michael.divia>

## Copyright and license

Code and documentation copyright 2023 the authors. Code released under the [MIT License](https://gitedu.hesge.ch/flg_courses/embedded_linux/students/michael.divia/-/blob/master/LICENSE).

Enjoy :metal: