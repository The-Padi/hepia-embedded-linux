# Lab 7 - Systèmes de fichiers de type “flash”

# Table des matières
- [Lab 7 - Systèmes de fichiers de type “flash”](#lab-7---systèmes-de-fichiers-de-type-flash)
- [Table des matières](#table-des-matières)
- [Instruction \& Description](#instruction--description)
- [Mise en place de la communication série avec la carte](#mise-en-place-de-la-communication-série-avec-la-carte)
	- [Branchements sur la carte](#branchements-sur-la-carte)
	- [Picocom](#picocom)
- [Laboratoire \& Questions](#laboratoire--questions)
	- [Modification d’U-Boot](#modification-du-boot)
	- [Caractéristiques de la NAND](#caractéristiques-de-la-nand)
		- [Q1 - Quelle est la taille d’un erase block, d’une page et d’une sous-page pour la flash NAND de votre carte SAMA5D3 ?](#q1---quelle-est-la-taille-dun-erase-block-dune-page-et-dune-sous-page-pour-la-flash-nand-de-votre-carte-sama5d3-)
		- [Q2 - Comment avez-vous déterminés ces informations de la flash de votre carte ?](#q2---comment-avez-vous-déterminés-ces-informations-de-la-flash-de-votre-carte-)
		- [Q3 - Où sont donc définies les partitions MTD de la NAND de votre carte ?](#q3---où-sont-donc-définies-les-partitions-mtd-de-la-nand-de-votre-carte-)
	- [Partitionnement de la NAND](#partitionnement-de-la-nand)
		- [Q4 - Quelles partitions avez-vous déclarées en lecture seule et en lecture-écriture et pourquoi ?](#q4---quelles-partitions-avez-vous-déclarées-en-lecture-seule-et-en-lecture-écriture-et-pourquoi-)
	- [Création du device UBI et des volumes UBI](#création-du-device-ubi-et-des-volumes-ubi)
		- [Q5 - Pour chaque volume UBI créé, décrivez et justifiez :](#q5---pour-chaque-volume-ubi-créé-décrivez-et-justifiez-)
	- [Création des images UBIFS](#création-des-images-ubifs)
	- [Écriture des images UBIFS](#écriture-des-images-ubifs)
	- [Modification du système pour qu’il boot et fonctionne correctement](#modification-du-système-pour-quil-boot-et-fonctionne-correctement)
- [Creators](#creators)
- [Copyright and license](#copyright-and-license)


# Instruction & Description

[Lab7 - Systèmes de fichiers de type “flash”](https://gitedu.hesge.ch/flg_courses/embedded_linux/embedd_linux_pub_fall23/-/blob/master/labs/lab07-fs_flash.pdf)

Contrairement au labo précédent où vous avez utilisé une carte SD (managed flash) pour y stocker les différents systèmes de fichiers, le noyau et le device tree, le but ici est d’utiliser exclusivement la NAND raw de votre carte SAMA5D3. Nous sommes donc dans un cas où il n’y a plus de FTL (Flash Translation Layer), la mémoire flash étant utilisée nativement. C’est donc le travail du système de fichiers de gérer le wear-leveling et les blocs défectueux.

Vous désirez configurer la NAND raw de votre carte de la manière indiquée en Figure 1, à savoir :

- Quatre partitions MTD
- Un device UBI dans la dernière partition
- Cinq volumes UBI au sein du device UBI

Notez que le contenu de la NAND contenant les trois premières partitions reste inchangé par rapport aux labos précédents.

La NAND doit être divisée en quatre partitions :

- 1ère partition pour AT91 bootstrap
- 2ème partition pour U-Boot
- 3ème partition pour l’environnement d’U-Boot ainsi que son backup
- 4ème partition pour un device UBI
- 
Au sein du device UBI, cinq volumes sont à créér, chacun hébergeant un système de fichiers UBIFS :

- boot : noyau et device tree ; toujours accédé en lecture seule
- rootfs : système de fichiers racine ; toujours accédé en lecture seule
- root : système de fichiers monté dans /root ; accédé en lecture-écriture
- var : système de fichiers monté dans /var ; accédé en lecture-écriture
- home : système de fichiers monté dans /home ; accédé en lecture-écriture

Bien entendu le système devra se comporter identiquement à celui du labo précédent (utilisateurs,sécurité, serveur web).

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

## Modification d’U-Boot

Afin de supporter les commandes `mtd` et `mtdparts` nous avons besoin de modifier la configuration de U-Boot.
Nous allons donc devoir appliquer le patch `u-boot_v2021.07.mtdparts.patch` :

~~~cmd
❯ cp u-boot_v2021.07.mtdparts.patch ~/Git/linux_embarq/michael.divia/bootloaders/
❯ cd ~/Git/linux_embarq/michael.divia/bootloaders/u-boot-2021.07
❯ cat ../u-boot_v2021.07.mtdparts.patch | patch -p1 --dry-run
checking file board/atmel/sama5d3_xplained/sama5d3_xplained.c
❯ cat ../u-boot_v2021.07.mtdparts.patch | patch -p1
patching file board/atmel/sama5d3_xplained/sama5d3_xplained.c
~~~

Maintenant que notre U-Boot est patcher nous pouvons aller modifier certaines options dans ça configuration :

~~~cmd
❯ make ARCH=arm CROSS_COMPILE=arm-linux- menuconfig
~~~

Puis :

- Dans `Device Drivers` :
  - Dans `MTD Support` :
    - Activer `Allow MTDPARTS to be configured at runtime`.
- Dans `Command line interface` :
  - Dans `Device access commands` :
    - Activer `mtd`.

Nous pouvons maintenant recompiler U-Boot et l'uploader sur notre board :

~~~cmd
❯ make ARCH=arm CROSS_COMPILE=arm-linux-
[... Compiling ...]
[ Remove Jumper JP5 + Reset the Board ]
❯ cd ../sam-ba_3.5
❯ ./sam-ba -p usb -b sama5d3-xplained -a nandflash -c erase:0x00040000:0x100000
Opening serial port 'ttyACM0'
Connection opened.
Detected memory size is 268435456 bytes.
Page size is 2048 bytes.
Buffer is 22528 bytes (11 pages) at address 0x0030a580.
NAND header value is 0xc0902405.
Supported erase block sizes: 128KB
Executing command 'erase:0x00040000:0x100000'
Erased 131072 bytes at address 0x00040000 (12.50%)
Erased 131072 bytes at address 0x00060000 (25.00%)
Erased 131072 bytes at address 0x00080000 (37.50%)
Erased 131072 bytes at address 0x000a0000 (50.00%)
Erased 131072 bytes at address 0x000c0000 (62.50%)
Erased 131072 bytes at address 0x000e0000 (75.00%)
Erased 131072 bytes at address 0x00100000 (87.50%)
Erased 131072 bytes at address 0x00120000 (100.00%)
Connection closed.
❯ ./sam-ba -p usb -b sama5d3-xplained -a nandflash -c write:../u-boot-2021.07/u-boot.bin:0x00040000
Opening serial port 'ttyACM0'
Connection opened.
Detected memory size is 268435456 bytes.
Page size is 2048 bytes.
Buffer is 22528 bytes (11 pages) at address 0x0030a580.
NAND header value is 0xc0902405.
Supported erase block sizes: 128KB
Executing command 'write:../u-boot-2021.07/u-boot.bin:0x00040000'
Appending 914 bytes of padding to fill the last written page
Wrote 22528 bytes at address 0x00040000 (2.86%)
Wrote 22528 bytes at address 0x00045800 (5.71%)
Wrote 22528 bytes at address 0x0004b000 (8.57%)
Wrote 22528 bytes at address 0x00050800 (11.43%)
Wrote 22528 bytes at address 0x00056000 (14.29%)
Wrote 18432 bytes at address 0x0005b800 (16.62%)
Wrote 22528 bytes at address 0x00060000 (19.48%)
Wrote 22528 bytes at address 0x00065800 (22.34%)
Wrote 22528 bytes at address 0x0006b000 (25.19%)
Wrote 22528 bytes at address 0x00070800 (28.05%)
Wrote 22528 bytes at address 0x00076000 (30.91%)
Wrote 18432 bytes at address 0x0007b800 (33.25%)
Wrote 22528 bytes at address 0x00080000 (36.10%)
Wrote 22528 bytes at address 0x00085800 (38.96%)
Wrote 22528 bytes at address 0x0008b000 (41.82%)
Wrote 22528 bytes at address 0x00090800 (44.68%)
Wrote 22528 bytes at address 0x00096000 (47.53%)
Wrote 18432 bytes at address 0x0009b800 (49.87%)
Wrote 22528 bytes at address 0x000a0000 (52.73%)
Wrote 22528 bytes at address 0x000a5800 (55.58%)
Wrote 22528 bytes at address 0x000ab000 (58.44%)
Wrote 22528 bytes at address 0x000b0800 (61.30%)
Wrote 22528 bytes at address 0x000b6000 (64.16%)
Wrote 18432 bytes at address 0x000bb800 (66.49%)
Wrote 22528 bytes at address 0x000c0000 (69.35%)
Wrote 22528 bytes at address 0x000c5800 (72.21%)
Wrote 22528 bytes at address 0x000cb000 (75.06%)
Wrote 22528 bytes at address 0x000d0800 (77.92%)
Wrote 22528 bytes at address 0x000d6000 (80.78%)
Wrote 18432 bytes at address 0x000db800 (83.12%)
Wrote 22528 bytes at address 0x000e0000 (85.97%)
Wrote 22528 bytes at address 0x000e5800 (88.83%)
Wrote 22528 bytes at address 0x000eb000 (91.69%)
Wrote 22528 bytes at address 0x000f0800 (94.55%)
Wrote 22528 bytes at address 0x000f6000 (97.40%)
Wrote 18432 bytes at address 0x000fb800 (99.74%)
Wrote 2048 bytes at address 0x00100000 (100.00%)
Connection closed.
~~~

Nous pouvons maintenant reset la board et vérifier si `mtd` fonctionne comme attendu :

~~~cmd
=> mtd list
List of MTD devices:
* nand0
  - type: NAND flash
  - block size: 0x20000 bytes
  - min I/O: 0x800 bytes
  - OOB size: 64 bytes
  - OOB available: 34 bytes
  - ECC strength: 4 bits
  - ECC step size: 2048 bytes
  - bitflip threshold: 3 bits
  - 0x000000000000-0x000010000000 : "nand0"
	  - 0x000000000000-0x000010000000 : "data"
=> mtdparts 

device nand0 <atmel_nand>, # parts = 1
 #: name		size		offset		mask_flags
 0: data                0x10000000	0x00000000	0

active partition: nand0,0 - (data) 0x10000000 @ 0x00000000

defaults:
mtdids  : nand0=atmel_nand
mtdparts: mtdparts=atmel_nand:-(data)
~~~

## Caractéristiques de la NAND

Pour la création du système de fichier `UBIFS` nous aurons besoin de la taille des `erase blocks`, des `pages` et des `sous-pages` :

~~~cmd
=> nand info

Device 0: nand0, sector size 128 KiB
  Page size       2048 b
  OOB size          64 b
  Erase size    131072 b
  subpagesize     2048 b
  options     0x40004200
  bbt options 0x00028000
~~~

### Q1 - Quelle est la taille d’un erase block, d’une page et d’une sous-page pour la flash NAND de votre carte SAMA5D3 ?

Nous pouvons donc en ressortir les informations suivante :

- Erase Block : 131072 bytes (ou 128 KiB)
- Page : 2048 bytes
- Sous-Page : 2048 bytes

### Q2 - Comment avez-vous déterminés ces informations de la flash de votre carte ?

En utilisant la commande : `nand info`.

### Q3 - Où sont donc définies les partitions MTD de la NAND de votre carte ?

Les partitions MTD sont actuellement définie dans notre `DTS` comme suit :

~~~bash
partitions {
				compatible = "fixed-partitions";
				#address-cells = <1>;
				#size-cells = <1>;

				at91bootstrap@0 {
					label = "at91bootstrap";
					reg = <0x0 0x40000>;
				};

				bootloader@40000 {
					label = "bootloader";
					reg = <0x40000 0xc0000>;
				};

				bootloaderenvred@100000 {
					label = "bootloader env redundant";
					reg = <0x100000 0x40000>;
				};

				bootloaderenv@140000 {
					label = "bootloader env";
					reg = <0x140000 0x40000>;
				};

				dtb@180000 {
					label = "device tree";
					reg = <0x180000 0x80000>;
				};

				kernel@200000 {
					label = "kernel";
					reg = <0x200000 0x600000>;
				};

				rootfs@800000 {
					label = "rootfs";
					reg = <0x800000 0x0f800000>;
				};
			};

~~~

## Partitionnement de la NAND

Nous allons donc maintenant définir les 4 partions `MTD` comme demandé. Pour rappel, voici ce que nous voulons :

- AT91Bootstrap : `0x00000` -> `0x40000`
- U-Boot : `0x40000` -> `0x100000`
- U-Boot env / Backup env : `0x100000` -> `0x160000`
- UBI device : `0x160000` -> `0x10000000`

~~~cmd
=> editenv mtdparts 
edit : mtdparts=atmel_nand:0x40000(AT91Bootstrap)ro,0xc0000(U-Boot)ro,0x60000(U-Boot-Env),-(UBI-Device)
=> editenv bootmenu_4
edit : Linux 6.1.56 - Netowork+MTD = setenv bootargs $bootargs $mtdparts;tftpboot 0x21000000 zImage;tftpboot 0x22000000 at91-sama5d3_xplained.dtb;bootz 0x21000000 - 0x22000000;
=> saveenv 
Saving Environment to NAND... Erasing redundant NAND...
Erasing at 0x100000 -- 100% complete.
Writing to redundant NAND... OK
OK
[... Reboot on bootmenu_4 ...]
~ # cat /proc/mtd
dev :    size   erasesize  name
mtd0 : 00040000 00020000 "AT91Bootstrap"
mtd1 : 000c0000 00020000 "U-Boot"
mtd2 : 00060000 00020000 "U-Boot-Env"
mtd3 : 0fea0000 00020000 "UBI-Device"
~~~

Et nous voyons que notre Linux a bien pris nos modification car nous lui passons `$mtdparts` dans nos `$bootargs`.

### Q4 - Quelles partitions avez-vous déclarées en lecture seule et en lecture-écriture et pourquoi ?

Les partition pour `AT91Bootstrap` et `U-Boot` sont en lecture seul car elle ne doivent pas être modifiée. les partition `U-Boot Env` et `UBI Device` sont en lecture/écriture car nous devons encore modifier les variable d'environnement de U-Boot et UBI est logiquement voué à changer.

## Création du device UBI et des volumes UBI

Pour commencer nous allons effacer le contenu de la partition `MTD` qui contiendra le device `UBI` :

~~~cmd
=> nand erase 0x160000 0xFEA0000

NAND erase: device 0 offset 0x160000, size 0xfea0000
Skipping bad block at  0x0ff80000                                          
Skipping bad block at  0x0ffa0000                                          
Skipping bad block at  0x0ffc0000                                          
Skipping bad block at  0x0ffe0000                                          

OK
~~~

Nous pouvons maintenant remettre a jour notre `bootargs` afin qu'il prenne notre nouveau device `UBI`  :

~~~cmd
=> editenv bootmenu_4
edit : Linux 6.1.56 - Netowork+MTD = setenv bootargs $bootargs $mtdparts ubi.mtd=3;tftpboot 0x21000000 zImage;tftpboot 0x22000000 at91-sama5d3_xplained.dtb;bootz 0x21000000 - 0x22000000;
~~~

### Q5 - Pour chaque volume UBI créé, décrivez et justifiez :

Pour commencer nous avons `boot` qui prendra notre kernel (~4 Mo) et notre `DTB` (~30 ko). Nous allons donc créer une partition de 6 Mo (afin d'avoir un peu de marge si nous voulons ajouter des éléments à notre kernel) en statique (comme indiqué en slide 22 du document `11-MTD_and_flash_filesystems.pdf`) :

~~~cmd
~ # ubimkvol -N boot -s 6MiB -t static /dev/ubi0
~~~

Ensuite nous avons `rootfs` qui sera notre système de fichier racine. Notre `rootfs` actuel se rapproche des 6 Mo. Se volume sera dynamique (comme indiqué en slide 22 du document `11-MTD_and_flash_filesystems.pdf`) :

~~~cmd
~ # ubimkvol -N rootfs -s 6MiB -t dynamic /dev/ubi0
~~~

Ensuite nous avons `root` qui est actuellement vide, donc nous pouvons lui allouer 4 Mo en cas de besoin. Se volume sera dynamique (comme indiqué en slide 22 du document `11-MTD_and_flash_filesystems.pdf`) :

~~~cmd
~ # ubimkvol -N root -s 4MiB -t dynamic /dev/ubi0
~~~

Ensuite pour `home` qui n'a que 1 utilisateur pour le moment, j'ai allouer 20 Mo afin d'allouer 5 Mo pour 4 potentielles utilisateur. Se volume sera dynamique (comme indiqué en slide 22 du document `11-MTD_and_flash_filesystems.pdf`) :

~~~cmd
~ # ubimkvol -N home -s 20MiB -t dynamic /dev/ubi0
~~~

Et pour terminer nous avons `var` à qui nous n'allons pas spécifier de taille afin d'utiliser le maximum encore disponible pour le stockage de notre server web. Se volume sera dynamique (comme indiqué en slide 22 du document `11-MTD_and_flash_filesystems.pdf`) :

~~~cmd
~ # ubimkvol -N var -m -t dynamic /dev/ubi0
~~~

Nous pouvons confirmer que nous avons bien 5 volumes de créé :

~~~cmd
~ # ls /dev/ubi0*
/dev/ubi0    /dev/ubi0_0  /dev/ubi0_1  /dev/ubi0_2  /dev/ubi0_3  /dev/ubi0_4
~~~

## Création des images UBIFS

Nous pouvons maintenant créer nos différentes images :

~~~cmd
❯ sudo mkfs.ubifs -F -m 2048 -e 126976 -c 50 -r boot boot.img
~~~

Où 50 ≃ 6291456/126976

~~~cmd
❯ sudo mkfs.ubifs -F -m 2048 -e 126976 -c 50 -r nfsroot_copy /srv/tftp/rootfs.img
~~~

Où 50 ≃ 6291456/126976

~~~cmd
❯ sudo mkfs.ubifs -F -m 2048 -e 126976 -c 34 -r nfsroot/root /srv/tftp/root.img
~~~

Où 34 ≃ 4194304/126976

~~~cmd
❯ sudo mkfs.ubifs -F -m 2048 -e 126976 -c 166 -r nfsroot/home /srv/tftp/home.img
~~~

Où 166 ≃ 20971520/126976

~~~cmd
❯ sudo mkfs.ubifs -F -m 2048 -e 126976 -c 1800 -r nfsroot/var /srv/tftp/var.img
~~~

Où 1800 ≃ 228589568/126976

## Écriture des images UBIFS

Nous pouvons maintenant écrire toute nos images dans leurs volumes `UBI` correspondant :

~~~cmd
~ # ubiupdatevol /dev/ubi0_0 boot.img
~ # ubiupdatevol /dev/ubi0_1 rootfs.img
~ # ubiupdatevol /dev/ubi0_2 root.img
~ # ubiupdatevol /dev/ubi0_3 home.img
~ # ubiupdatevol /dev/ubi0_4 var.img
~~~

En redémarrant notre board nous pouvons voir que nos images on bien été écrites :

~~~cmd
=> ubi part UBI-Device
ubi0 : attaching mtd4
ubi0 : scanning is finished
ubi0 : attached mtd4 (name "UBI-Device", size 254 MiB)
ubi0 : PEB size: 131072 bytes (128 KiB), LEB size: 126976 bytes
ubi0 : min./max. I/O unit sizes: 2048/2048, sub-page size 2048
ubi0 : VID header offset: 2048 (aligned 2048), data offset: 4096
ubi0 : good PEBs: 2033, bad PEBs: 4, corrupted PEBs: 0
ubi0 : user volume: 5, internal volumes: 1, max. volumes count: 128
ubi0 : max/mean erase counter: 2/1, WL threshold: 4096, image sequence number: 149254540
ubi0 : available PEBs: 2, total reserved PEBs: 2031, PEBs reserved for bad PEB handling: 36
=> ubifsmount ubi0:boot
=> ubifsls
	    28392  Thu Dec 28 15:11:28 2023  at91-sama5d3_xplained.dtb
	  3855824  Thu Dec 28 15:11:31 2023  zImage
=> ubifsumount          
Unmounting UBIFS volume boot!
=> ubifsmount ubi0:rootfs
=> ubifsls
<DIR>	     6136  Sat Dec 30 16:09:59 2023  bin
<DIR>	      160  Sat Dec 30 16:09:59 2023  dev
<DIR>	      680  Sat Dec 30 16:09:59 2023  etc
<DIR>	      600  Sat Dec 30 16:09:59 2023  lib
<DIR>	      160  Sat Dec 30 16:09:59 2023  sys
<DIR>	      288  Sat Dec 30 16:09:59 2023  usr
<DIR>	      160  Sat Dec 30 16:09:59 2023  proc
<DIR>	     4976  Sat Dec 30 16:09:59 2023  sbin
<LNK>	       11  Sat Dec 30 16:09:59 2023  linuxrc
<DIR>	      160  Sat Dec 30 16:10:03 2023  media
~~~

## Modification du système pour qu’il boot et fonctionne correctement

Il ne nous reste plus qu'à indiqué a `U-Boot` le nouvel emplacement de nos images, nous devons le faire avec 2 variables d'environnement différente car sinon nous atteignons la taille max de ces dernières :

~~~cmd
=> editenv bargsubi 
edit: root=ubi0:rootfs rootfstype=ubifs rw ubi.mtd=3 ip=192.168.1.2:::::eth0
=> editenv bootmenu_4 
edit : Linux 6.1.56 - NAND = setenv bootargs $bargsubi $mtdparts; ubi part UBI-Device; ubifsmount ubi0:boot; ubifsload 0x21000000 zImage; ubifsload 0x22000000 at91-sama5d3_xplained.dtb; ubifsload 0x22000000 at91-sama5d3_xplained.dtb; bootz 0x21000000 - 0x22000000
=> saveenv 
Saving Environment to NAND... Erasing redundant NAND...
Erasing at 0x100000 -- 100% complete.
Writing to redundant NAND... OK
OK
~~~

Et pour terminer, boot sur notre nouveau système et modifier notre scripte `S10Mount` afin qu'il monte correctement nos nouveaux Volumes pour `var`, `home` et `root` :

~~~cmd
[... BOOT ...]
/ # vi /etc/init.d/S10mount
~~~

Et nous y mettons :

~~~bash
#!/bin/sh
#
# Mount proc & sys & var & home & root
#

mount -t proc proc /proc
mount -t sysfs sysfd /sys
                              
mount -t ubifs ubi0:root /root
mount -t ubifs ubi0:home /home
mount -t ubifs ubi0:var /var
~~~

Après un reboot, nous pouvons voir que tout nos dossiers sont présent :

~~~cmd
/ # ls
bin      etc      lib      media    root     sys      var
dev      home     linuxrc  proc     sbin     usr
/ # cd var
/var # ls
www
~~~

Il ne reste plus qu'à tester notre serveur web en uploadant un fichier :

~~~cmd
/var/www/upload/files # ls
2_embedded_engineer.png     preinternet_chat_rooms.jpg
Embedded_Linux_Primer.jpg   sudo.png
linux_distribs.jpg          upgrading.png
padi_full.png
[... Branchement de la clé USB ...]
/var/www/upload/files # ls
16138501907_815e50d51a_k.jpg  50791488916_6acc6fb002_k.jpg
2480028401_c325f59709_k.jpg   50899859382_12d5eebbcc_k.jpg
25011113087_02ff51099e_k.jpg  50931301453_4f13ce750a_k.jpg
25050827701_87b701be6d_k.jpg  51131999811_e09c8c7c87_k.jpg
25144108165_db6c36bcdc_k.jpg  51165672541_a46d9e41c6_k.jpg
25269111389_e2ed516dd7_k.jpg  51199164534_af55a7d739_k.jpg
25422896582_2d8dc79f70_k.jpg  51217247762_af17283c35_k.jpg
25999653882_01609b1473_k.jpg  51218173723_50d4ad6656_k.jpg
2_embedded_engineer.png       51345631281_ba0f233b7c_k.jpg
30497771915_b333bb77f7_k.jpg  51357939936_3652ef8989_k.jpg
30941582392_818906a7e0_o.jpg  51389523157_e564256b65_k.jpg
32178534444_eba729ed7f_k.jpg  51391300535_7212caac01_k.jpg
33994406348_1629ae4e9c_k.jpg  51619205263_156d780bb9_k.jpg
36419625564_88c578bff7_k.jpg  8226774297_d0c1d2e6dc_k.jpg
36443288663_f4a6dc1fc1_k.jpg  Embedded_Linux_Primer.jpg
4666028952_32f2055d56_k.jpg   linux_distribs.jpg
4927459373_1692a72008_k.jpg   padi_full.png
49628614811_d1af91dfd2_k.jpg  preinternet_chat_rooms.jpg
49817420326_f8d6fede7e_k.jpg  sudo.png
50541605451_8564d0279a_k.jpg  upgrading.png
~~~

Tout fonctionne correctement, c'est une victoire !

# Creators

**Michael Divià**

- <https://gitedu.hesge.ch/michael.divia>

# Copyright and license

Code and documentation copyright 2023 the authors. Code released under the [MIT License](https://gitedu.hesge.ch/flg_courses/embedded_linux/students/michael.divia/-/blob/master/LICENSE).

Enjoy :metal:
