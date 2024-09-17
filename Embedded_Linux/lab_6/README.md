# Lab 6 - Systèmes de fichiers de type “bloc”

# Table des matières
- [Lab 6 - Systèmes de fichiers de type “bloc”](#lab-6---systèmes-de-fichiers-de-type-bloc)
- [Table des matières](#table-des-matières)
- [Instruction \& Description](#instruction--description)
- [Mise en place de la communication série avec la carte](#mise-en-place-de-la-communication-série-avec-la-carte)
  - [Branchements sur la carte](#branchements-sur-la-carte)
  - [Picocom](#picocom)
- [Laboratoire \& Questions](#laboratoire--questions)
  - [Noyau : support des systèmes de fichiers](#noyau--support-des-systèmes-de-fichiers)
    - [Q1 - Depuis le shell de votre système embarqué, comment pouvez-vous vous assurez que votre noyau est capable de gérer les systèmes de fichiers ci-dessus ?](#q1---depuis-le-shell-de-votre-système-embarqué-comment-pouvez-vous-vous-assurez-que-votre-noyau-est-capable-de-gérer-les-systèmes-de-fichiers-ci-dessus-)
  - [Partitionnement de la carte SD](#partitionnement-de-la-carte-sd)
    - [Q2 - Quel est le nom du périphérique correspondant à votre carte SD détecté par le noyau Linux de votre machine hôte ? Décrivez 4 méthodes différentes permettant de déterminer le nom du périphérique (indice : slide “Block device list” du cours et logs).](#q2---quel-est-le-nom-du-périphérique-correspondant-à-votre-carte-sd-détecté-par-le-noyau-linux-de-votre-machine-hôte--décrivez-4-méthodes-différentes-permettant-de-déterminer-le-nom-du-périphérique-indice--slide-block-device-list-du-cours-et-logs)
  - [Boot du noyau depuis la carte SD](#boot-du-noyau-depuis-la-carte-sd)
    - [Q3 - Sur votre système embarqué, quel est le nom du périphérique correspondant à votre carte SD détecté par le noyau ? Selon vous, pourquoi est-il différent du nom obtenu à la Q2 ?](#q3---sur-votre-système-embarqué-quel-est-le-nom-du-périphérique-correspondant-à-votre-carte-sd-détecté-par-le-noyau--selon-vous-pourquoi-est-il-différent-du-nom-obtenu-à-la-q2-)
    - [Q4 - Dans votre cas d’utilisation ici présent, serait-il plus judicieux d’utiliser un système de fichiers journalisé (comme ext4 ou fs2fs) en lieu et place du système de fichiers FAT ? Justifiez.](#q4---dans-votre-cas-dutilisation-ici-présent-serait-il-plus-judicieux-dutiliser-un-système-de-fichiers-journalisé-comme-ext4-ou-fs2fs-en-lieu-et-place-du-système-de-fichiers-fat--justifiez)
  - [Serveur web : stockage sur la carte SD](#serveur-web--stockage-sur-la-carte-sd)
    - [Q5 - Quels changements avez-vous effectués sur votre système embarqué pour que celui-ci fonctionne correctement ?](#q5---quels-changements-avez-vous-effectués-sur-votre-système-embarqué-pour-que-celui-ci-fonctionne-correctement-)
    - [Q6 - Comment avez-vous vérifié que les fichiers sont bien écrits sur la carte SD ?](#q6---comment-avez-vous-vérifié-que-les-fichiers-sont-bien-écrits-sur-la-carte-sd-)
  - [Fichier logs avec tmpfs](#fichier-logs-avec-tmpfs)
    - [Q7 - Si aucune taille n’est spécifiée, quelle est la taille mémoire maximum utilisée par tmpfs ?](#q7---si-aucune-taille-nest-spécifiée-quelle-est-la-taille-mémoire-maximum-utilisée-par-tmpfs-)
  - [Système de fichiers racine sur carte SD](#système-de-fichiers-racine-sur-carte-sd)
    - [Q8 - Quelle compression vous donne l’image la plus compacte ?](#q8---quelle-compression-vous-donne-limage-la-plus-compacte-)
- [Creators](#creators)
- [Copyright and license](#copyright-and-license)


# Instruction & Description

[Lab6 - Systèmes de fichiers de type “bloc”](https://gitedu.hesge.ch/flg_courses/embedded_linux/embedd_linux_pub_fall23/-/blob/master/labs/lab06-fs_block.pdf)

L’objectif de ce travail pratique est de mettre en place un système embarqué Linux où rootfs, noyau et device tree seront stockés sur un support de stockage de type bloc.
Vous transitionnerez donc d’un rootfs basé sur NFS vers un contenu stocké sur un support MMC (carte SD).
De manière similaire, noyau et device tree ne se trouveront plus en NAND raw, mais sur la carte SD.

# Mise en place de la communication série avec la carte

## Branchements sur la carte

`TXD -> Blanc`<br>
`RXD -> Vert`<br>
`GND -> Noir`<br>
`X   -> Rouge`

## Picocom

`picocom -b 115200 /dev/ttyUSB0`<br>
L'utilisateur doit être dans le groupe `dialout` afin que cela fonctionne.

# Laboratoire & Questions

## Noyau : support des systèmes de fichiers

Nous allons commencer par vérifié quelles système de fichiers sont supporté sur notre board :

~~~cmd
~ # cat /proc/filesystems 
nodev	sysfs
nodev	tmpfs
nodev	bdev
nodev	proc
nodev	cgroup
nodev	cgroup2
nodev	devtmpfs
nodev	configfs
nodev	debugfs
nodev	sockfs
nodev	pipefs
nodev	ramfs
nodev	rpc_pipefs
nodev	devpts
	    ext3
	    ext2
	    ext4
	    vfat
nodev	nfs
nodev	autofs
nodev	ubifs
~~~

Nous avons besoin du support pour `SquashFS`, `EROFS`, `F2FS` et `ext4`. Il nous manque donc `SquashFS`, `EROFS`, `F2FS`.

Nous devons donc aller modifier le noyau af in d'ajouter le support pour ces systèmes de fichier :

~~~cmd
❯ make ARCH=arm CROSS_COMPILE=arm-linux- menuconfig
~~~

- Dans `File systems`:
  - `F2FS filesystem support` doit être en `<*>`.
  - Dans `Miscellaneous filesystems`:
    - `SquashFS 4.0 - Squashed file system support` doit être en `<*>`.
    - `EROFS filesystem support` doit être en `<*>`.

Nous pouvons maintenant recompiler notre noyau.

~~~cmd
❯ make ARCH=arm CROSS_COMPILE=arm-linux- -j $(nproc)
[Kernel Compiling...]
❯ rm /srv/tftp/zImage
❯ cp arch/arm/boot/zImage /srv/tftp/
~~~

Nous pouvons maintenant redémarrer notre board et Flash notre nouveau Kernel sur la NAND en sélectionnant U-Boot console dans notre Boot Menu et en exécutant :

~~~cmd
=> $update_kernel
ㅤethernet@f0028000: PHY present at 7
ㅤethernet@f0028000: Starting autonegotiation...
ㅤethernet@f0028000: Autonegotiation complete
ㅤethernet@f0028000: link up, 1000Mbps full-duplex (lpa: 0x6800)
Using ethernet@f0028000 device
TFTP from server 192.168.1.1; our IP address is 192.168.1.2
Filename 'zImage'.
Load address: 0x21000000
ㅤLoading: #################################################################
	 #################################################################
	 #################################################################
	 #################################################################
	 ###
	 713.9 KiB/s
done
Bytes transferred = 3855824 (3ad5d0 hex)

NAND erase: device 0 offset 0x160000, size 0x3ad5d0
Erasing at 0x500000 -- 100% complete.
OK

NAND write: device 0 offset 0x160000, size 0x3ad5d0
 3855824 bytes written: OK
~~~

Pour terminé nous redémarrons la carte et retournons sur notre Linux afin de vérifier si le support pour tout les système de fichier nécessaire sont présent :

~~~cmd
~ # cat /proc/filesystems
nodev	sysfs
nodev	tmpfs
nodev	bdev
nodev	proc
nodev	cgroup
nodev	cgroup2
nodev	devtmpfs
nodev	configfs
nodev	debugfs
nodev	sockfs
nodev	pipefs
nodev	ramfs
nodev	rpc_pipefs
nodev	devpts
	    ext3
	    ext2
	    ext4
	    squashfs
	    vfat
nodev	nfs
nodev	autofs
	    f2fs
	    erofs
nodev	ubifs
~~~

Nous avons à présent bien le support pour `SquashFS`, `EROFS`, `F2FS` et `ext4`.

### Q1 - Depuis le shell de votre système embarqué, comment pouvez-vous vous assurez que votre noyau est capable de gérer les systèmes de fichiers ci-dessus ?

En allant vérifier dans `/proc/filesystem`. Ce fichier contient tout les systèmes de fichier actuellement supporté par le kernel.

## Partitionnement de la carte SD

Nous branchons tout d'abord la carte SD sur notre machine hôte.

### Q2 - Quel est le nom du périphérique correspondant à votre carte SD détecté par le noyau Linux de votre machine hôte ? Décrivez 4 méthodes différentes permettant de déterminer le nom du périphérique (indice : slide “Block device list” du cours et logs).

Le nom du périphérique est `sda`. Nous pouvons le voir par les méthodes suivante :

En vérifiant dans `/proc/partitions`, qui liste tout les block device disponible :

~~~cmd
❯ cat /proc/partitions
major minor  #blocks  name

   7        0     108296 loop0
   7        1     108360 loop1
   7        2      64988 loop2
   7        3      75416 loop3
   7        4      20412 loop4
 259        0  488386584 nvme1n1
 259        1    1046527 nvme1n1p1
 259        2    4194303 nvme1n1p2
 259        3  478946304 nvme1n1p3
 259        4    4194303 nvme1n1p4
 259        5 1000204632 nvme0n1
 259        6     266240 nvme0n1p1
 259        7      16384 nvme0n1p2
 259        8  975929671 nvme0n1p3
 259        9     716800 nvme0n1p4
 259       10   23068672 nvme0n1p5
 259       11     204800 nvme0n1p6
 253        0    4193791 dm-0
 252        0   15749120 zram0
   8        0    7782400 sda
~~~

En utiliant la commande `lsblk` :

~~~cmd
❯ lsblk
NAME          MAJ:MIN RM   SIZE RO TYPE  MOUNTPOINTS
loop0           7:0    0 105.8M  1 loop  /snap/core/16091
loop1           7:1    0 105.8M  1 loop  /snap/core/16202
loop2           7:2    0  63.5M  1 loop  /snap/core20/2015
loop3           7:3    0  73.6M  1 loop  /snap/desktop-habitica/1
loop4           7:4    0  19.9M  1 loop  /snap/nethack/87
sda             8:0    1   7.4G  0 disk  
zram0         252:0    0    15G  0 disk  [SWAP]
nvme1n1       259:0    0 465.8G  0 disk  
├─nvme1n1p1   259:1    0  1022M  0 part  /boot/efi
├─nvme1n1p2   259:2    0     4G  0 part  /recovery
├─nvme1n1p3   259:3    0 456.8G  0 part  /
└─nvme1n1p4   259:4    0     4G  0 part  
  └─cryptswap 253:0    0     4G  0 crypt [SWAP]
nvme0n1       259:5    0 953.9G  0 disk  
├─nvme0n1p1   259:6    0   260M  0 part  
├─nvme0n1p2   259:7    0    16M  0 part  
├─nvme0n1p3   259:8    0 930.7G  0 part  
├─nvme0n1p4   259:9    0   700M  0 part  
├─nvme0n1p5   259:10   0    22G  0 part  
└─nvme0n1p6   259:11   0   200M  0 part 
~~~

En utilisant la commande `fdisk` :

~~~cmd
Disque /dev/loop0 : 105.76 MiB, 110895104 octets, 216592 secteurs
Unités : secteur de 1 × 512 = 512 octets
Taille de secteur (logique / physique) : 512 octets / 512 octets
taille d'E/S (minimale / optimale) : 512 octets / 512 octets


Disque /dev/loop1 : 105.82 MiB, 110960640 octets, 216720 secteurs
Unités : secteur de 1 × 512 = 512 octets
Taille de secteur (logique / physique) : 512 octets / 512 octets
taille d'E/S (minimale / optimale) : 512 octets / 512 octets


Disque /dev/loop2 : 63.46 MiB, 66547712 octets, 129976 secteurs
Unités : secteur de 1 × 512 = 512 octets
Taille de secteur (logique / physique) : 512 octets / 512 octets
taille d'E/S (minimale / optimale) : 512 octets / 512 octets


Disque /dev/loop3 : 73.65 MiB, 77225984 octets, 150832 secteurs
Unités : secteur de 1 × 512 = 512 octets
Taille de secteur (logique / physique) : 512 octets / 512 octets
taille d'E/S (minimale / optimale) : 512 octets / 512 octets


Disque /dev/loop4 : 19.93 MiB, 20901888 octets, 40824 secteurs
Unités : secteur de 1 × 512 = 512 octets
Taille de secteur (logique / physique) : 512 octets / 512 octets
taille d'E/S (minimale / optimale) : 512 octets / 512 octets


Disque /dev/nvme1n1 : 465.76 GiB, 500107862016 octets, 976773168 secteurs
Disk model: Samsung SSD 970 EVO Plus 500GB          
Unités : secteur de 1 × 512 = 512 octets
Taille de secteur (logique / physique) : 512 octets / 512 octets
taille d'E/S (minimale / optimale) : 512 octets / 512 octets
Type d'étiquette de disque : gpt
Identifiant de disque : 40E3D87F-9807-422D-B162-C42C872021DE

Périphérique       Début       Fin  Secteurs Taille Type
/dev/nvme1n1p1      4096   2097150   2093055  1022M Système EFI
/dev/nvme1n1p2   2097152  10485758   8388607     4G Données de base Microsoft
/dev/nvme1n1p3  10485760 968378367 957892608 456.8G Système de fichiers Linux
/dev/nvme1n1p4 968380464 976769070   8388607     4G Partition d'échange Linux


Disque /dev/nvme0n1 : 953.87 GiB, 1024209543168 octets, 2000409264 secteurs
Disk model: HFM001TD3JX013N                         
Unités : secteur de 1 × 512 = 512 octets
Taille de secteur (logique / physique) : 512 octets / 512 octets
taille d'E/S (minimale / optimale) : 512 octets / 512 octets
Type d'étiquette de disque : gpt
Identifiant de disque : 65F4AC5D-A5D8-47E4-9267-813A0B57DBED

Périphérique        Début        Fin   Secteurs Taille Type
/dev/nvme0n1p1       2048     534527     532480   260M Système EFI
/dev/nvme0n1p2     534528     567295      32768    16M Réservé Microsoft
/dev/nvme0n1p3     567296 1952426638 1951859343 930.7G Données de base Microsoft
/dev/nvme0n1p4 1952428032 1953861631    1433600   700M Environnement de récupération Windows
/dev/nvme0n1p5 1953861632 1999998975   46137344    22G Données de base Microsoft
/dev/nvme0n1p6 1999998976 2000408575     409600   200M Environnement de récupération Windows


Disque /dev/mapper/cryptswap : 4 GiB, 4294442496 octets, 8387583 secteurs
Unités : secteur de 1 × 512 = 512 octets
Taille de secteur (logique / physique) : 512 octets / 512 octets
taille d'E/S (minimale / optimale) : 512 octets / 512 octets


Disque /dev/zram0 : 15.02 GiB, 16127098880 octets, 3937280 secteurs
Unités : secteur de 1 × 4096 = 4096 octets
Taille de secteur (logique / physique) : 4096 octets / 4096 octets
taille d'E/S (minimale / optimale) : 4096 octets / 4096 octets


Disque /dev/sda : 7.42 GiB, 7969177600 octets, 15564800 secteurs
Disk model: STORAGE DEVICE  
Unités : secteur de 1 × 512 = 512 octets
Taille de secteur (logique / physique) : 512 octets / 512 octets
taille d'E/S (minimale / optimale) : 512 octets / 512 octets
~~~

Et pour terminer, en listant tout les fichier dans `/dev` qui ont la bonne structure :

~~~cmd
❯ ls /dev/sd*
/dev/sda
~~~

Nous allons maintenant partitionner notre carte SD. Pour cela nous allons créer un script :

~~~cmd
❯ touch prepare_card
❯ chmod +x prepare_card
❯ nano prepare_card
~~~

~~~bash
#!/bin/sh
# Partitioning of the SD Card

if [ $# -ne 1 ]; then
        echo "Usage: prepare_card [peripheral (/dev/sda)]"
        exit 1
fi

sudo dd if=/dev/urandom of="$1" bs=1M count=1 conv=notrunc

sudo fdisk "$1" <<EOF
o
n
p
1
2048
+12M
t
6
n
p
2
26624
+200M
n
p
3
436224
+64M
w
EOF
~~~

Si nous exécutons notre script sur `/dev/sda` voici le résultat :

~~~cmd
❯ sudo fdisk /dev/sda

Bienvenue dans fdisk (util-linux 2.37.2).
Les modifications resteront en mémoire jusqu'à écriture.
Soyez prudent avant d'utiliser la commande d'écriture.


Commande (m pour l'aide) : p
Disque /dev/sda : 7.42 GiB, 7969177600 octets, 15564800 secteurs
Disk model: STORAGE DEVICE  
Unités : secteur de 1 × 512 = 512 octets
Taille de secteur (logique / physique) : 512 octets / 512 octets
taille d'E/S (minimale / optimale) : 512 octets / 512 octets
Type d'étiquette de disque : dos
Identifiant de disque : 0x2381387b

Périphérique Amorçage  Début    Fin Secteurs Taille Id Type
/dev/sda1               2048  26623    24576    12M  6 FAT16
/dev/sda2              26624 436223   409600   200M 83 Linux
/dev/sda3             436224 567295   131072    64M 83 Linux
~~~

## Boot du noyau depuis la carte SD

Pour commencer, nous allons modifier notre système afin que le noyau Linux et le device tree ne soient plus stockés dans la NAND raw de la carte, mais sur la carte SD.

Pour cela nous allons créer un système de fichier FAT16 sur la première partition et y copier le noyau Linux et le device tree :

~~~cmd
❯ sudo mkfs.fat /dev/sda1 -n KERNEL
mkfs.fat 4.2 (2021-01-31)
❯ sudo mkdir /media/padi/KERNEL
❯ sudo mount /dev/sda1 /media/padi/KERNEL
❯ cp zImage /media/padi/KERNEL
❯ cp at91-sama5d3_xplained.dtb /media/padi/KERNEL
~~~

Nous allons maintenant ajouter une entrée dans notre menu U-Boot afin de charger le noyau et le device tree depuis la carte SD :

~~~cmd
=> editenv bootmenu_2
edit : Linux 6.1.56 - SD Card= fatload mmc 0:1 0x21000000 zImage;fatload mmc 0:1 0x22000000 at91-sama5d3_xplained.dtb;bootz 0x21000000 - 0x22000000;
=> saveenv
~~~

Nous pouvons maintenant reboot et vérifier si notre nouvelle option dans le menu U-Boot fonctionne bien.

### Q3 - Sur votre système embarqué, quel est le nom du périphérique correspondant à votre carte SD détecté par le noyau ? Selon vous, pourquoi est-il différent du nom obtenu à la Q2 ?

Sur la Board, la carte SD est appelée `mmcblk0` et non plus `sda`. D'après mes observations cela est dû au faite que nous avons utilisé un adaptateur USB pour lire la carte SD depuis la machine hôte. Le noyau Linux attribue un nom de périphérique basé sur l'interface USB plutôt que sur le type de carte SD. En revanche, lorsque nous branchons directement la carte SD dans un port SD sur notre système embarqué, le noyau utilise une convention de nommage différente basée sur l'interface MMC (MultiMediaCard).

### Q4 - Dans votre cas d’utilisation ici présent, serait-il plus judicieux d’utiliser un système de fichiers journalisé (comme ext4 ou fs2fs) en lieu et place du système de fichiers FAT ? Justifiez.

Oui il serai intéressant de passer à un système de fichiers journalisé car il peut apporter plusieurs avantage à notre système :

1. Intégrité des données
2. Fiabilité
3. Temps de Recovery

## Serveur web : stockage sur la carte SD

Nous allons migré notre répertoire `var` sur la carte SD.

Pour se faire, nous allons commençons par créer un système de fichier journalisé `F2FS` sur la deuxième partition de notre carte SD :

~~~cmd
❯ sudo mkfs.f2fs -l var /dev/sda2

	F2FS-tools: mkfs.f2fs Ver: 1.14.0 (2020-08-24)

Info : Disable heap-based policy
Info : Debug level = 0
Info : Label = var
Info : Trim is enabled
Info : [/dev/sda2] Disk Model: STORAGE DEVICE  
Info : Segments per section = 1
Info : Sections per zone = 1
Info : sector size = 512
Info : total sectors = 409600 (200 MB)
Info : zone aligned segment0 blkaddr: 512
Info : format version with
  "Linux version 6.5.6-76060506-generic (jenkins@warp.pop-os.org) (x86_64-linux-gnu-gcc-12 (Ubuntu 12.3.0-1ubuntu1~22.04) 12.3.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #202310061235~1697396945~22.04~9283e32 SMP PREEMPT_DYNAMIC Sun O"
Info : [/dev/sda2] Discarding device
Info : This device doesn't support BLKSECDISCARD
Info : This device doesn't support BLKDISCARD
Info : Overprovision ratio = 15.000%
Info : Overprovision segments = 31 (GC reserved = 21)
Info : format successful

❯ sudo mkdir /media/padi/var-sd
❯ sudo mount /dev/sda2 /media/padi/var-sd
❯ sudo cp -r /home/padi/Git/linux_embarq/michael.divia/nfsroot/var/* /media/padi/var-sd
~~~

Maintenant que la partition est prête nous devons aller modifier la configuration de notre système embarqué afin que cette deuxième partition sois notre nouveau dossier `var` :

~~~cmd
/ # cp -r /var/www /home/Padi/www-backup
/ # rm -rf /var/www
/ # ls /var
/ # mount /dev/mmcblk0p2 /var
/ # chown -R www:nogroup /var/www
/ # vi /etc/init.d/S10mount
~~~

et nous le modifions afin d'avoir :

~~~bash
#!/bin/sh
#
# Mount proc & sys & var
#

mount -t proc proc /proc
mount -t sysfs sysfd /sys
mount /dev/mmcblk0p2 /var
~~~

Nous pouvons à présent reboot notre carte et vérifier si tout est bien en place via la carte SD :

~~~cmd
[...Boot...]
/ # ls -la var
drwxr-xr-x    3 root     root          4096 Dec 12  2023 .
drwxr-xr-x   14 root     root          4096 Nov 23  2023 ..
drwxr-xr-x    4 www      nogroup       3488 Dec 12  2023 www
/ # touch /var/This_is_the_SD_Card.txt
~~~

Et que nous vérifions que le dossier `var` dans `nfsroot` sur notre machine hôte est bien vide :

~~~cmd
❯ cd Git/linux_embarq/michael.divia/nfsroot
❯ cd var
❯ ls
❯ ls -la
total 8
drwxr-xr-x  2 root root 4096 déc 12 18:02 .
drwxr-xr-x 14 root root 4096 nov 23 23:04 ..
~~~

### Q5 - Quels changements avez-vous effectués sur votre système embarqué pour que celui-ci fonctionne correctement ?

Nous avons fait en sorite que notre `S10mount` monte la deuxième partition de la carte SD sur le dossier `var` au boot.

### Q6 - Comment avez-vous vérifié que les fichiers sont bien écrits sur la carte SD ?

En créant un fichier sur notre nouveau `var` et en vérifiant que ce dernier n'est pas apparu dans le dossier `nfsroot` sur notre machine hôte.

## Fichier logs avec tmpfs

Nous allons maintenant migrer le fichier `/var/www/upload/logs/upload.log` dans la RAM via l'utilisation d'un système de fichier `tmpfs` :

~~~cmd
/ # vi /etc/init.d/S11webserver
~~~

Et nous le modifions afin d'avoir :

~~~bash
#!/bin/sh
#
# Start our Web Server & and mount log folder in RAM
#
                        
mount -t tmpfs varwwwlog /var/www/upload/logs -o size=4M
httpd -h /var/www -u 100
~~~

Nous pouvons redémarrer notre board et faire un test :

~~~cmd
[...BOOT...]
~ # ls /var/www/upload/logs/
~~~

Nous uploadons un fichier, puis :

~~~cmd
/ # cat /var/www/upload/logs/upload.log 
Mon Jan  1 00:30:40 2007 File uploaded succesfully: padi_full.png (66430 bytes)

[...REBOOT...]

~ # cat /var/www/upload/logs/upload.log 
cat : can't open '/var/www/upload/logs/upload.log': No such file or directory
~~~

### Q7 - Si aucune taille n’est spécifiée, quelle est la taille mémoire maximum utilisée par tmpfs ?

La taille par défaut sera la moitié de la RAM sans le swap.

## Système de fichiers racine sur carte SD

Pour terminer nous allons finir rpar migrer tout le reste de notre `rootfs` sur une partiton `EROFS` sur notre carte SD.

Nous allons commencer par créer plusieurs images `EROFS` de notre `rootfs` afin de trouvé la plus efficace :

~~~cmd
❯ mkfs.erofs -z lz4hc erofs_1.img ../nfsroot
mkfs.erofs 1.4
	c_version :           [     1.4]
	c_dbg_lvl :           [       2]
	c_dry_run :           [       0]
❯ mkfs.erofs -z lz4 erofs_2.img ../nfsroot
mkfs.erofs 1.4
	c_version :           [     1.4]
	c_dbg_lvl :           [       2]
	c_dry_run :           [       0]
❯ ls -la
total 44864
drwxrwxr-x  2 padi padi     4096 déc 12 19:11 .
drwxrwxr-x 16 padi padi     4096 déc  5 16:49 ..
-rw-r--r--  1 padi padi 22839296 déc 12 19:11 erofs_1.img
-rw-r--r--  1 padi padi 23064576 déc 12 19:11 erofs_2.img
-rwxrwxr-x  1 padi padi      275 déc  5 18:25 prepare_card
-rw-rw-r--  1 padi padi    23606 déc 12 19:11 README.md
~~~

Le vainceur est `lz4hc`.

### Q8 - Quelle compression vous donne l’image la plus compacte ?

`lz4hc` donne l'image la plus petite.

Nous pouvons maintenant flasher cette image sur la bonne partition de la carte SD :

~~~cmd
❯ sudo dd if=erofs_1.img of=/dev/sda3
44608+0 enregistrements lus
44608+0 enregistrements écrits
22839296 octets (23 MB, 22 MiB) copiés, 7.10641 s, 3.2 MB/s
~~~

Nous pouvons maintenant aller sur notre Board et rajouter une entrée dans notre me nu U-BOOT :

~~~cmd
=> editenv bootmenu_3
edit : Linux 6.1.56 - Full SD Card= setenv bootargs 'root=/dev/mmcblk0p3 ro rootwait rootfstype=erofs';fatload mmc 0:1 0x21000000 zImage;fatload mmc 0:1 0x22000000 at91-sama5d3_xplained.dtb;bootz 0x21000000 - 0x22000000;
~~~

Après reboot de la carte et avoir choisi notre nouvelle entrée dans le menu nous arrivons malheureusement sur cette erreur :

~~~cmd
[...BOOT...]
Loading compiled-in X.509 certificates
mmc0 :  host does not support reading read-only switch, assuming write-enable
mmc0 : new high speed SDHC card at address 0007
mmcblk0 : mmc0:0007 SD8GB 7.42 GiB 
input : gpio-keys as /devices/platform/gpio-keys/input/input0
cfg80211 : Loading compiled-in X.509 certificates for regulatory database
 mmcblk0 : p1 p2 p3
cfg80211 : Loaded X.509 cert 'sforshee: 00b28ddf47aef9cea7'
platform regulatory.0: Direct firmware load for regulatory.db failed with error -2
cfg80211 : failed to load regulatory.db
erofs : (device mmcblk0p3): mounted with root inode @ nid 36.
VFS : Mounted root (erofs filesystem) readonly on device 179:3.
devtmpfs : mounted
Freeing unused kernel image (initmem) memory: 1024K
Run /sbin/init as init process
Kernel panic - not syncing: Attempted to kill init! exitcode=0x00000004
CPU : 0 PID: 1 Comm: init Not tainted 6.1.56 #5
Hardware name : Atmel SAMA5
 unwind_backtrace from show_stack+0x10/0x14
 show_stack from dump_stack_lvl+0x24/0x2c
 dump_stack_lvl from panic+0xf8/0x2f4
 panic from make_task_dead+0x0/0x18c
---[ end Kernel panic - not syncing: Attempted to kill init! exitcode=0x00000004 ]---
~~~

Après investigation, aucune solutio n'a été trouvée, à part de ne pas compresser l'image erofs, ce qui est contre intuitif pour la fin de se laboratoire.

A cause de cette erreur la suite des questions/étapes n'ont pas pu être effectuées.

# Creators

**Michael Divià**

- <https://gitedu.hesge.ch/michael.divia>

# Copyright and license

Code and documentation copyright 2023 the authors. Code released under the [MIT License](https://gitedu.hesge.ch/flg_courses/embedded_linux/students/michael.divia/-/blob/master/LICENSE).

Enjoy :metal:
