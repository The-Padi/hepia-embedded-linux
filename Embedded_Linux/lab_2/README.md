# Lab 2 - Bootloaders


## Table of contents
- [Lab 2 - Bootloaders](#lab-2---bootloaders)
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
  - [Creators](#creators)
  - [Copyright and license](#copyright-and-license)


## Instructions

[Lab2 - Bootloaders](https://gitedu.hesge.ch/flg_courses/embedded_linux/embedd_linux_pub_fall23/-/blob/master/labs/lab02-bootloader.pdf)

## Mise en place de la communication série avec la carte

### Branchements sur la carte

`TXD -> Blanc`<br>
`RXD -> Vert`<br>
`GND -> Noir`

### Picocom

`picocom -b 115200 /dev/ttyUSB0`

## Questions

### Q1
#### Quel fichier de configuration avez vous choisi ?

J'ai choisi le fichier de configuration `sama5d3_xplained/sama5d3_xplainednf_uboot_defconfig`.

### Q2
#### A quelle adresse se trouve U-Boot en flash NAND et quelle est la taille maximale de celui-ci (donnez les tailles en hexadécimal et en base 10) ?

~~~h
(0x00040000) Flash Offset for U-Boot (NEW)
(0x000c0000) U-Boot Image Size (NEW)
(0x26F00000) The External Ram Address to Load U-Boot Image (NEW)
~~~

Le U-Boot devra se trouver à l'adresse `0x00040000` -> 262'144.<br>
La taille du U-Boot devrait être `0x000c0000` -> 786'432.


### Q3
#### Dans quel répertoire se trouve le bootloader que vous venez de compiler, comment se nomme-t-il et quelle est sa taille ? (indice : extension bin)

Le bootloader que je viens de compiler grâce à la commande `make CROSS_COMPILE=arm-linux-` se trouve dans le dossier `binaries`.<br>
Il se nomme `sama5d3_xplained-nandflashboot-uboot-3.9.3.bin` et fait 15,2 ko.

### Q4
#### Quelle configuration U-Boot avez-vous utilisée ?

J'ai choisi le fichier de configuration `sama5d3_xplained_nandflash_defconfig`.

### Q5
#### Sur quel type de support de stockage est sauvegardé l’environnement d’U-Boot ?

Le U-Boot sera sauvegardé dans la NAND de la carte.

### Q6
#### A quelles adresses se trouvent l’environnement et son backup ?

~~~h
(0x140000) Environment offset
(0x100000) Redundant environment offset
~~~

### Q7
#### Quelle est la taille de l’environnement ?

~~~h
(0x20000) Environment Size
~~~

### Q8
#### Quel fichier pensez-vous flasher sur votre carte et quelle est sa taille ?

D'après les slides du cours je devrai flasher `u-boot.bin` et il fait 779,9 ko.

### Q9
#### Donnez les commandes qui vous ont permis de décompresser l’archive de sam-ba.

J'ai pu décompresser l'archive grâce à la commande `gunzip sam-ba_3.5-linux_x86_64.tar.gz && tar fx sam-ba_3.5-linux_x86_64.tar`.

### Q10
#### Donnez la commande complète vous ayant permis de flasher AT91Bootstrap et U-Boot dans la NAND de votre carte.

J'ai commencé par tout erase sur ma board car le U-Boot était déjà présent avec la commande suivant : `./sam-ba -p usb -b sama5d3-xplained -a nandflash -c erase`. <br>
J'ai ensuite écrit le bootloader AT91Bootstrap avec : `./sam-ba -p usb -b sama5d3-xplained -a nandflash:1:8:0xc0902405 -c writeboot:sama5d3_xplained-nandflashboot-uboot-3.9.3.bin`. <br>
Et pour terminer j'ai écrire le bootloader U-Boot avec : `./sam-ba -p usb -b sama5d3-xplained -a nandflash -c write:u-boot.bin:0x00040000`.

Nous pouvons ensuite faire le test avec la variable d'environnement : 
~~~bash
=> editenv test_1
edit: 123456789
=> env print -a test_1
test_1=123456789
=> env save
Saving Environment to NAND... Erasing redundant NAND...
Erasing at 0x100000 -- 100% complete.
Writing to redundant NAND... OK
OK
=> reset
resetting ...

[----------reboot----------]

=> env print -a test_1
test_1=123456789
~~~

Victory !

### Q11
#### Comment avez-vous inspecté le contenu de la NAND et retrouvez-vous bien le contenu escompté ?

J'ai utilisé la commande `nand dump 0x100000`.

Je reçois ceci :

~~~h
4b 0d 83 b1 01 61 72 63  68 3d 61 72 6d 00 62 61
75 64 72 61 74 65 3d 31  31 35 32 30 30 00 62 6f
61 72 64 3d 73 61 6d 61  35 64 33 5f 78 70 6c 61
69 6e 65 64 00 62 6f 61  72 64 5f 6e 61 6d 65 3d
73 61 6d 61 35 64 33 5f  78 70 6c 61 69 6e 65 64
00 62 6f 6f 74 61 72 67  73 3d 63 6f 6e 73 6f 6c
65 3d 74 74 79 53 30 2c  31 31 35 32 30 30 20 65
61 72 6c 79 70 72 69 6e  74 6b 20 6d 74 64 70 61
72 74 73 3d 61 74 6d 65  6c 5f 6e 61 6e 64 3a 32
35 36 6b 28 62 6f 6f 74  73 74 72 61 70 29 72 6f
2c 37 36 38 6b 28 75 62  6f 6f 74 29 72 6f 2c 32
35 36 4b 28 65 6e 76 5f  72 65 64 75 6e 64 61 6e
74 29 2c 32 35 36 6b 28  65 6e 76 29 2c 35 31 32
6b 28 64 74 62 29 2c 36  4d 28 6b 65 72 6e 65 6c
29 72 6f 2c 2d 28 72 6f  6f 74 66 73 29 20 72 6f
6f 74 66 73 74 79 70 65  3d 75 62 69 66 73 20 75
62 69 2e 6d 74 64 3d 36  20 72 6f 6f 74 3d 75 62
69 30 3a 72 6f 6f 74 66  73 00 62 6f 6f 74 63 6d
64 3d 6e 61 6e 64 20 72  65 61 64 20 30 78 32 31
30 30 30 30 30 30 20 30  78 31 38 30 30 30 30 20
30 78 38 30 30 30 30 3b  6e 61 6e 64 20 72 65 61
64 20 30 78 32 32 30 30  30 30 30 30 20 30 78 32
30 30 30 30 30 20 30 78  36 30 30 30 30 30 3b 62
6f 6f 74 7a 20 30 78 32  32 30 30 30 30 30 30 20
2d 20 30 78 32 31 30 30  30 30 30 30 00 62 6f 6f
74 64 65 6c 61 79 3d 33  00 63 70 75 3d 61 72 6d
76 37 00 66 64 74 63 6f  6e 74 72 6f 6c 61 64 64
72 3d 32 66 62 33 37 66  35 30 00 73 6f 63 3d 61
74 39 31 00 73 74 64 65  72 72 3d 73 65 72 69 61
6c 40 66 66 66 66 65 65  30 30 00 73 74 64 69 6e
3d 73 65 72 69 61 6c 40  66 66 66 66 65 65 30 30
00 73 74 64 6f 75 74 3d  73 65 72 69 61 6c 40 66
66 66 66 65 65 30 30 00  74 65 73 74 5f 31 3d 31
32 33 34 35 36 37 38 39  00 76 65 6e 64 6f 72 3d
61 74 6d 65 6c
~~~

Si nous le passons [ici](https://www.rapidtables.com/convert/number/hex-to-ascii.html), nous pouvons lire les informations.<br>
Le plus important dans tout cela se trouve à la fin : `test_1=123456789`. Nous avons bel et bien notre variable d'environnement.

### Q12
#### Listez les commandes que vous avez écrites pour parvenir au résultat demandé.

~~~bash
=>setenv bootmenu_0 Test_1=echo "Hello N# 1 !"
=>setenv bootmenu_1 Test_2=echo "Hello N# 2 !"
=>setenv bootmenu_2 Test_3=echo "Hello N# 3 !"
=>editenv bootmenu
edit: bootmenu 15
=>env save boot
~~~

### Q13
#### Comment pouvez-vous vous assurer que votre serveur TFTPD est bien en attente de connexions (indice : ss. . . ) ?

Grâce à la command `sudo netstat -tuln` je peux voir quelles ports sont en attente de connexion sur mon PC :
~~~bash
❯ sudo netstat -tul
Connexions Internet actives (seulement serveurs)
Proto    Recv-Q    Send-Q    Adresse locale    Adresse distante    Etat      
[...]
udp      0         0         0.0.0.0:tftp      0.0.0.0:*
[...]
~~~

### Q14
#### Comment avez-vous pu vérifier que le fichier a été transféré correctement ? Détaillez votre méthode.

J'ai commencé car créer un fichier test sur ma machine hôte :

~~~bash
❯ cat TEST_2.txt
YAY !
~~~

Je l'ai ensuite transféré sur ma carte :
~~~bash
=> tftpboot 0x22000000 TEST_2.txt
ethernet@f0028000: PHY present at 7
ethernet@f0028000: Starting autonegotiation...
ethernet@f0028000: Autonegotiation complete
ethernet@f0028000: link up, 1000Mbps full-duplex (lpa: 0x2800)
Using ethernet@f0028000 device
TFTP from server 192.168.1.1; our IP address is 192.168.1.2
Filename 'TEST_2.txt'.
Load address: 0x22000000
Loading:  #
          0 Bytes/s
done
Bytes transferred = 6 (6 hex)
~~~

Et je suis ensuite aller lire la DRAM à l’adresse que nous avions spécifier grâce à la commande md (memory display):
~~~
=> md 0x22000000 
22000000: 20594159 80900a21 08510825 60e032c0    YAY !...%.Q..2.`
22000010: 30a11e4c e4280100 1049a065 d84842b0    L..0..(.e.I..BH.
22000020: 124a2884 2c041529 40002224 84c00192    .(J.)..,$".@....
22000030: 0100555d 2062a102 e2361840 c9f21908    ]U....b @.6.....
22000040: 00381800 02090281 008c2d08 20851040    ..8......-..@.. 
22000050: 04484100 0ca80341 904a8902 020043d1    .AH.A.....J..C..
22000060: 24301110 c0501066 50869424 0b422866    ..0$f.P.$..Pf(B.
22000070: b4028004 20000302 b2a44112 81010906    ....... .A......
22000080: 0808034c 84c18031 260d0981 c8490440    L...1......&@.I.
22000090: 25011069 924a5010 28100f1d 42400900    i..%.PJ....(..@B
220000a0: 90500a2c 122a1000 c0010840 00000602    ,.P...*.@.......
220000b0: 6228000c 084069b6 94141000 10d29348    ..(b.i@.....H...
220000c0: 80a04942 04648025 c2411eca 0f1011ac    BI..%.d...A.....
220000d0: 43861248 a0084906 0252c489 42a40262    H..C.I....R.b..B
220000e0: 9290a003 028812c7 dc000400 203c88a2    ..............< 
220000f0: 2044c7cc 21540011 0a285264 80c02000    ..D ..T!dR(.. ..
~~~

Et nous pouvons bel et ben voir le contenu de mon fichier text tout au début (`YAY !`).

### Q15
#### Donnez le contenu du script, comment vous l’avez converti en image et comment vous l’avez récupéré et exécuté depuis U-Boot.

Premièrement voici le script `TEST_3.script` que j'ai créé sur ma machine hôte :
~~~bash
❯ cat TEST_3.script
echo Welcome on the $board_name of Padi !;
echo Network Informations :;
echo IP -> $ipaddr;
~~~
Suite à cela. j'ai pu convertir le script en image grâce à la commande `mkimage` du paquet `u-boot-tools` :
~~~bash
❯ mkimage -A arm -O u-boot -T script -n 'Welcome message' -d TEST_3.script TEST_3.img
Image Name:   Welcome message
Created:      Sun Oct  8 17:33:23 2023
Image Type:   ARM U-Boot Script (gzip compressed)
Data Size:    100 Bytes = 0.10 KiB = 0.00 MiB
Load Address: 00000000
Entry Point:  00000000
Contents:
    Image 0: 92 Bytes = 0.09 KiB = 0.00 MiB
❯ ls
TEST_1.png  TEST_2.txt  TEST_3.img  TEST_3.script
~~~
Le fichier `TEST_3.img` ayant été créé je peux maintenant le télécharger sur la carte puis l'exécuter :
~~~bash
=> tftpboot 0x22000000 TEST_3.img
ethernet@f0028000: PHY present at 7
ethernet@f0028000: Starting autonegotiation...
ethernet@f0028000: Autonegotiation complete
ethernet@f0028000: link up, 1000Mbps full-duplex (lpa: 0x6800)
Using ethernet@f0028000 device
TFTP from server 192.168.1.1; our IP address is 192.168.1.2
Filename 'TEST_3.img'.
Load address: 0x22000000
Loading: #
    0 Bytes/s
done
Bytes transferred = 164 (a4 hex)
=> source 0x22000000
## Executing script at 22000000
Welcome on the sama5d3_xplained of Padi !
Network Informations :
IP -> 192.168.1.2
~~~

## Creators

**Michael Divià**

- <https://gitedu.hesge.ch/michael.divia>

## Copyright and license

Code and documentation copyright 2023 the authors. Code released under the [MIT License](https://gitedu.hesge.ch/flg_courses/embedded_linux/students/michael.divia/-/blob/master/LICENSE).

Enjoy :metal:
