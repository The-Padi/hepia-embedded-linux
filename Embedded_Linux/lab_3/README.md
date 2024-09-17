# Lab 3 - Noyau Linux


## Table of contents
- [Lab 3 - Noyau Linux](#lab-3---noyau-linux)
  - [Table of contents](#table-of-contents)
  - [Instructions](#instructions)
  - [Mise en place de la communication série avec la carte](#mise-en-place-de-la-communication-série-avec-la-carte)
    - [Branchements sur la carte](#branchements-sur-la-carte)
    - [Picocom](#picocom)
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
    - [Q14](#q14)
    - [Q15](#q15)
    - [Q16](#q16)
    - [Q17](#q17)
    - [Q18](#q18)
  - [Creators](#creators)
  - [Copyright and license](#copyright-and-license)


## Instructions

[Lab3 - Kernel](https://gitedu.hesge.ch/flg_courses/embedded_linux/embedd_linux_pub_fall23/-/blob/master/labs/lab03-kernel.pdf)

## Mise en place de la communication série avec la carte

### Branchements sur la carte

`TXD -> Blanc`<br>
`RXD -> Vert`<br>
`GND -> Noir`

### Picocom

`picocom -b 115200 /dev/ttyUSB0`

## Questions

### Q1
#### Quelle est la version du noyau sur votre machine de développement ?

~~~bash
❯ uname -ir
6.5.4-76060504-generic x86_64
~~~

Ma machine est donc sur la version de kernel `6.5.4`.

### Q2
#### Quelle est la dernière version stable du noyau Linux selon le site officiel (indiquez la date de consultation) ?

En date du 10 octobre 2023, la dernière version stable est la `6.5.6`.

### Q3
#### Quelle est la dernière version stable longterm du noyau Linux possédant la plus longue EOL (End Of Life) ? Quelle est sa EOL (indiquez la date de consultation) ?

À se jour, 10 octobre 2023, la version `6.1.56 (LTS)` est celle avec la plus longue EOL, prévue le 31 décembre 2026.

### Q4
#### Est-ce que la combinaison de cette version du noyau et votre chaîne de compilation croisée risque de poser problème ? Justifiez en développant votre réponse.

Les en-têtes doivent correspondre à la version du noyau avec laquelle ils sont utilisés. Dans notre cas, la version des en-têtes (`4.9.234`) est nettement antérieure à celle du noyau (`6.1.56`). Cela signifie que les en-têtes peuvent ne pas contenir les nouvelles fonctionnalités, les structures de données et les modifications apportées dans le noyau Linux `6.1.56`. Par conséquent, si notre système embarqué dépend de ces fonctionnalités ou modifications, cela peut entraîner des erreurs de compilation et/ou d'exécution. Si cela n'est pas le cas. Il ne devrai pas y avoir de problème alors.

### Q5
#### Quel est le nombre total de fichiers sources (.c, .h, et .S) du noyau que vous venez de télécharger ? Déterminer cette valeur en utilisant seulement une seule exécution de la commande find (astuces : find accepte l’argument -o pour réaliser un “ou” logique et pensez à utiliser pipe sur la sortie de find avec le compteur de lignes wc -l).

~~~bash
❯ find -name "*.c" -o -name "*.h" -o -name "*.S" | wc -l
56512
~~~

Nous avons donc 56'512 fichiers sources dans notre kernel `6.0`.

### Q6
#### Quels fichiers patch devez vous donc télécharger ?

Je vais donc devoir télécharger `patch-6.1.xz` puis `patch-6.1.56.xz`. <br>
Voici comment j'ai procédé :

~~~bash
❯ wget https://cdn.kernel.org/pub/linux/kernel/v6.x/patch-6.1.xz & wget https://cdn.kernel.org/pub/linux/kernel/v6.x/patch-6.1.56.xz
[...]
❯ cd linux-6.0
❯ xzcat ../patch-6.1.xz | patch -p1 --dry-run
[...]
❯ xzcat ../patch-6.1.xz | patch -p1
[...]
❯ xzcat ../patch-6.1.56.xz | patch -p1 --dry-run
[...]
❯ xzcat ../patch-6.1.56.xz | patch -p1
[...]
❯ cd ..
❯ mv linux-6.0 linux-6.1.56
~~~

Et voilà, notre kernel est maintenant en version `6.1.56`.

### Q7
#### Quelle configuration avez-vous choisie ?

J'ai choisi la configuration `sama5_defconfig - Build for sama5`.

### Q8
#### Comment pouvez-vous vous assurez avec une assez bonne certitude qu’il s’agit de la bonne configuration ?

Nous pouvons premièrement aller vérifier dans `System Type -> Platform selection` et ainsi voir que `ARMv7 based platforms` est bien sélectionné.<br>
Puis dans `System Type -> AT91/Microchip SoCs` et vérifier que `SAMA5D3 family` est sélectionné. <br>
Nous pouvons maintenant compiler notre kernel linux:
~~~bash
make ARCH=arm CROSS_COMPILE=arm-linux- -j $(nproc)
~~~

### Q9
#### Une fois la compilation du noyau terminée, où se trouve le noyau compilé et quelle est sa taille ?

Notre `zImage`, faisant 5.4 Mo, a donc été créé. Nous pouvons le trouver sous `/arch/arm/boot/`.

### Q10
#### Aussi, quel est le fichier de Device Tree Blob (binaire) pour votre carte Sama5D3 Xplained et quelle est sa taille ?

Le fichier `.dtb` pour notre carte est le suivant : `at91-sama5d3_xplained.dtb`, 28.4 ko.

### Q11
#### Quelle est la nouvelle taille du noyau et quel pourcentage de taille en espace disque avez-vous ainsi gagné par rapport au noyau précédent ?

Notre `zImage` fait maintenant 3.9 Mo. Nous avons donc économisé un peu plus de 27% d'espace disque.

### Q12
#### Après avoir lu les messages du noyau, expliquez aussi simplement que possible, quelle est la raison de la “panique” (ou crash) du noyau ?

~~~cmd
RAMDISK: Couldn't find valid RAM disk image starting at 0.
List of all partitions:
0100            8192 ram0 
 (driver?)
0101            8192 ram1 
 (driver?)
0102            8192 ram2 
 (driver?)
0103            8192 ram3 
 (driver?)
No filesystem could mount root, tried: 
 ext3
 ext2
 ext4
 vfat

Kernel panic - not syncing: VFS: Unable to mount root fs on unknown-block(1,0)
CPU: 0 PID: 1 Comm: swapper Not tainted 6.1.56 #2
Hardware name: Atmel SAMA5
 unwind_backtrace from show_stack+0x10/0x14
 show_stack from dump_stack_lvl+0x24/0x2c
 dump_stack_lvl from panic+0xf8/0x2f4
 panic from mount_block_root+0x248/0x250
 mount_block_root from mount_root+0x22c/0x248
 mount_root from prepare_namespace+0x154/0x190
 prepare_namespace from kernel_init+0x18/0x128
 kernel_init from ret_from_fork+0x14/0x28
Exception stack(0xd0815fb0 to 0xd0815ff8)
5fa0:                                     00000000 00000000 00000000 00000000
5fc0: 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
5fe0: 00000000 00000000 00000000 00000000 00000013 00000000
---[ end Kernel panic - not syncing: VFS: Unable to mount root fs on unknown-block(1,0) ]---
~~~

Il n'arrive pas à monter root filesystem car il n'y en a simplement pas.

### Q13
#### Listez le “code” U-Boot que vous avez écrit pour réaliser ce boot automatique.

~~~bash
=> echo $bootmenu_0
Linux 6.1.56=tftpboot 0x21000000 zImage;tftpboot 0x22000000 at91-sama5d3_xplained.dtb;bootz 0x21000000 - 0x22000000;
~~~

### Q14
#### Quelle sont les tailles, en bytes et KBytes, réservées dnas la NAND au noyau et au Device Tree ?

Pour le Kernel nous avons donc de `0x160000` à `0x700000`, cela correspond à `5'898'240 bytes` ou `5'760 KBytes`.<br>
Pour le DTB nous avons de `0x700000` à `0x720000`, cela correspond à `196'608 bytes` ou `192 KBytes`.

### Q15
#### Comment pouvez-vous vous assurer que la NAND a bien été effacée ?

~~~cmd
=> nand erase 0x160000 0x5C0000

NAND erase: device 0 offset 0x160000, size 0x5c0000
Erasing at 0x700000 -- 100% complete.
OK
=> nand dump 0x160000
Page 00160000 dump:
	ff ff ff ff ff ff ff ff  ff ff ff ff ff ff ff ff
	ff ff ff ff ff ff ff ff  ff ff ff ff ff ff ff ff
	ff ff ff ff ff ff ff ff  ff ff ff ff ff ff ff ff
	ff ff ff ff ff ff ff ff  ff ff ff ff ff ff ff ff
	ff ff ff ff ff ff ff ff  ff ff ff ff ff ff ff ff
	ff ff ff ff ff ff ff ff  ff ff ff ff ff ff ff ff
	ff ff ff ff ff ff ff ff  ff ff ff ff ff ff ff ff
	ff ff ff ff ff ff ff ff  ff ff ff ff ff ff ff ff
	ff ff ff ff ff ff ff ff  [Beaucoup de ff]  ff ff
~~~

### Q16
#### Donnez le contenu des scripts update_kernel et update_dtb.

~~~cmd
=> echo $update_kernel
tftpboot 0x21000000 zImage;nand erase 0x160000 0x$filesize;nand write 0x21000000 0x160000 0x$filesize
=> echo $update_dtb
tftpboot 0x22000000 at91-sama5d3_xplained.dtb;nand erase 0x700000 0x$filesize;nand write 0x22000000 0x700000 0x$filesize
~~~

### Q17
#### Donner les commandes U-Boot que vous avez utilisées par accomplir ceci.

~~~cmd
=> echo $bootmenu_0
Linux 6.1.56 - NAND=nand read 0x21000000 0x160000 0x5A0000;nand read 0x22000000 0x700000 0x20000;bootz 0x21000000 - 0x22000000;
~~~

### Q18
#### Donnez le contenu de vos scripts permettant de ne flasher que l’espace nécessaire dans la NAND de votre carte.

J'avais déjà effectué cette manipulation lors de la question 16 :

~~~cmd
=> echo $update_kernel
tftpboot 0x21000000 zImage;nand erase 0x160000 0x$filesize;nand write 0x21000000 0x160000 0x$filesize
=> echo $update_dtb
tftpboot 0x22000000 at91-sama5d3_xplained.dtb;nand erase 0x700000 0x$filesize;nand write 0x22000000 0x700000 0x$filesize
~~~

## Creators

**Michael Divià**

- <https://gitedu.hesge.ch/michael.divia>

## Copyright and license

Code and documentation copyright 2023 the authors. Code released under the [MIT License](https://gitedu.hesge.ch/flg_courses/embedded_linux/students/michael.divia/-/blob/master/LICENSE).

Enjoy :metal:
