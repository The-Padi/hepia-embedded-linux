# Lab 5 - Sécurité, mdev et modules

# Table des matières
- [Lab 5 - Sécurité, mdev et modules](#lab-5---sécurité-mdev-et-modules)
- [Table des matières](#table-des-matières)
- [Instruction \& Description](#instruction--description)
- [Mise en place de la communication série avec la carte](#mise-en-place-de-la-communication-série-avec-la-carte)
  - [Branchements sur la carte](#branchements-sur-la-carte)
  - [Picocom](#picocom)
- [Laboratoire \& Questions](#laboratoire--questions)
  - [Sécurisation du système et authentification](#sécurisation-du-système-et-authentification)
    - [Q1 - Décrivez précisément et de manière détaillée les différentes étapes que vous avez réalisées pour sécuriser le système et obtenir le mécanisme d’authentification souhaité.](#q1---décrivez-précisément-et-de-manière-détaillée-les-différentes-étapes-que-vous-avez-réalisées-pour-sécuriser-le-système-et-obtenir-le-mécanisme-dauthentification-souhaité)
  - [Hotplug et mdev](#hotplug-et-mdev)
    - [Q2 - Donnez l’emplacement et le contenu des scripts que vous avez implémentés](#q2---donnez-lemplacement-et-le-contenu-des-scripts-que-vous-avez-implémentés)
  - [Support USB avec des modules noyau](#support-usb-avec-des-modules-noyau)
    - [Q3 - Quelle est la liste de tous les modules que vous avez déployés sur votre système embarqués (fichiers .ko) ? Est-ce que cette liste contient des modules autres que pour l’USB ?](#q3---quelle-est-la-liste-de-tous-les-modules-que-vous-avez-déployés-sur-votre-système-embarqués-fichiers-ko--est-ce-que-cette-liste-contient-des-modules-autres-que-pour-lusb-)
    - [Q4 - Donnez les noms des modules nécessaires à votre noyau pour que la clé USB soit utilisable en tant que périphérique de stockage de masse. Veuillez décrire les modules sous forme d’arbres décrivant les dépendences intra-modules.](#q4---donnez-les-noms-des-modules-nécessaires-à-votre-noyau-pour-que-la-clé-usb-soit-utilisable-en-tant-que-périphérique-de-stockage-de-masse-veuillez-décrire-les-modules-sous-forme-darbres-décrivant-les-dépendences-intra-modules)
    - [Q5 - Quel fichier avez-vous ajouté à votre système pour ce dernier point et quel est son contenu ?](#q5---quel-fichier-avez-vous-ajouté-à-votre-système-pour-ce-dernier-point-et-quel-est-son-contenu-)
- [Creators](#creators)
- [Copyright and license](#copyright-and-license)


# Instruction & Description

[Lab5 - Sécurité, mdev et modules](https://gitedu.hesge.ch/flg_courses/embedded_linux/embedd_linux_pub_fall23/-/blob/master/labs/lab05-security_mdev_mods.pdf)

Dans ce laboratoire, nous commencerons par sécurisez notre système embarqué. Ensuite, nous mettrons en place un système de montage automatisé à l’aide du mécanisme hotplug du noyau et mdev. <br>
Enfin, nous nous familiariserons avec le déploiement et l’utilisation de modules noyau.

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

## Sécurisation du système et authentification

Dû à un bug dans BusyBox ou la librairie uClibc, il est nécessaire de recompiliez Busybox en réalisant une édition des liens statique avant de continuer.

Nous allons donc modifier la configuration :

~~~cmd
❯ make ARCH=arm CROSS_COMPILE=arm-linux- menuconfig
~~~

Et nous activons de nouveau cette option :

- Dans “Settings” :
  - `Build static binary (no shared libs)`
  
Nous pouvons ensuite recompiler BusyBox :

~~~cmd
❯ make ARCH=arm CROSS_COMPILE=arm-linux-
[...]
❯ make ARCH=arm CROSS_COMPILE=arm-linux- install
[...]
❯ ls -lh ../nfsroot/bin/busybox
-rwxr-xr-x 1 padi padi 1.2M nov 21 17:17 ../nfsroot/bin/busybox
~~~

Tout c'est bien passé car BusyBox est retourné à la taille qu'il avait avant le labo 4.

Nous pouvons ensuite sécuriser notre système. Nous allons commencer par créer le fichier `/etc/passwd` et `/etc/group` initial :

~~~cmd
/ # echo "root:x:0:0:root:/root:/bin/bash" > /etc/passwd
/ # echo "root:x:0:" > /etc/group
~~~

Nous créons ensuite le répertoire qui accueillera les répertoires `home` des utilisateurs et le répertoire `home` du root :

~~~cmd
/ # mkdir home
/ # mkdir root
~~~

Nous pouvons ensuite ajouter un mot de passe pour l'utilisateur root :

~~~cmd
/ # passwd
Changing password for root
New password: [Very Long And Complex Password]
Retype password: [Very Long And Complex Password]
ㅤpasswd: password for root changed by root
~~~

Puis créer un utilisateur non-root :

~~~cmd
/ # adduser Padi
Changing password for Padi
New password: [Very Long And Complex Password]
Retype password: [Very Long And Complex Password]
ㅤpasswd: password for Padi changed by root
/ # cat /etc/passwd 
root:NTqTy0v47Rzc6:0:0:root:/root:/bin/bash
Padi:Rf.v.QXfdFtl2:1000:1000:Linux User,,,:/home/Padi:/bin/sh
~~~

Nous allons ensuite créer un utilisateur système qui aura pour seul utilisé d'exécuter le service web que nous avons créé au précédent laboratoire :

~~~cmd
/ # addgroup -S nogroup
/ # adduser -S -h /var/www -D -G nogroup www
~~~

`-S` défini que nous créons un groupe ou un utilisateur système.<br>
`-h` défini où sera le  répertoire `home` de l'utilisateur.<br>
`-D` défini qu'il en faut pas de mot de passe pour cet utilisateur.

La prochaine étape est de permettre à ces utilisateurs de se connecter :

~~~cmd
/ # vi etc/inittab 
~~~

Où nous remplaçons `::respawn:-/bin/sh` en `::respawn:/sbin/getty 115200 ttyS0`.

Nous pouvons vérifier si nos modifications ont fonctionné en redémarrant notre carte :

~~~cmd
[... BOOT ...]
192.168.1.2 login: root
ㅤPassword: 
Jan  1 00:54:26 login[94]: root login on 'ttyS0'
~ # exit

192.168.1.2 login: www
ㅤPassword: 
Login incorrect
Jan  1 00:58:07 login[95]: invalid password for 'www' on 'ttyS0'
192.168.1.2 login: Padi
ㅤPassword: 
~ $
~~~

Nous pouvons ensuite aller modifier le démarrage de notre serveur web afin que l'utilisateur `www` s'en occupe :

~~~cmd
/ # vi /etc/init.d/S11webserver
~~~

Où nous remplaçons `httpd -h /var/www` en `httpd -h /var/www -u 100`.

Pour terminer nous allons modifier quelques permissions afin que tout sois conforme :

~~~cmd
~ # chown root:root /bin/busybox
~ # chmod u+s /bin/busybox
~ #
~ # chown -R www:nogroup /var/www
~ # chmod -R g-w,o-w /var/www
~ #
~ # cd /home/Padi/
/home/Padi # ls -la
drwxr-sr-x    2 Padi     Padi          4096 Nov 21  2023 .
drwxrwxrwx    3 root     root          4096 Nov 21  2023 ..
/home/Padi # cd /
/ # chown root:root /
/ # chmod g-w,o-w /
/ # chown -R root:root bin
/ # chmod -R g-w,o-w bin
/ # chown -R root:root etc
/ # chmod -R g-w,o-w etc
/ # chown -R root:root lib
/ # chmod -R g-w,o-w lib
/ # chown -R root:root sbin
/ # chmod -R g-w,o-w sbin
/ # chown -R root:root usr
/ # chmod -R g-w,o-w usr
/ # chown -R root:root dev
/ # chmod -R g-w,o-w dev
/ # chown root:root home
/ # chmod g-w,o-w home
/ # chown root:root linuxrc
/ # chmod g-w,o-w linuxrc
/ # chown -R root:root root
/ # chmod -R g-w,o-w root
/ # chown -R root:root sys
/ # chmod -R g-w,o-w sys
/ # chown root:root var
/ # chmod g-w,o-w var
~~~

Comme vous pouvez le voir, nous n'avons pas du modifier les droits du répertoire `/home/Padi` car elle correspondait déjà à ce que nous attendions.

### Q1 - Décrivez précisément et de manière détaillée les différentes étapes que vous avez réalisées pour sécuriser le système et obtenir le mécanisme d’authentification souhaité.

Tout est donc expliqué ci-dessus.

## Hotplug et mdev

Nous allons maintenant mettre en place le montage automatique de stockage de masse USB, l’allumage d'une LED d'indication d’état, le montage en lecture seul de ce dernier, la copie des images présentent vers le serveur web ainsi que le démontage.

Pour commencer, nous allons tester le bon fonctionnement de la LED rouge.

~~~cmd
/ # cd /sys/class/leds/d2/device/leds/d3
/sys/devices/platform/leds/leds/d3 # ls
brightness      max_brightness  subsystem       uevent
device          power           trigger
/sys/devices/platform/leds/leds/d3 # echo "1" > brightness 
~~~

La LED rouge de la board s'allume bien !

Préparons à présent la clé usb depuis la machine hôte.

~~~cmd
❯ ls
disk_ext4.img.xz
❯ unxz disk_ext4.img.xz
❯ ls
disk_ext4.img
❯ sudo dd if=disk_ext4.img of=/dev/sda
[sudo] Mot de passe de padi : 
131072+0 enregistrements lus
131072+0 enregistrements écrits
67108864 octets (67 MB, 64 MiB) copiés, 7.81641 s, 8.6 MB/s
❯ ls /media/padi/78d7ce66-3acd-44b2-8973-d14dd29f89d6/
16138501907_815e50d51a_k.jpg  30941582392_818906a7e0_o.jpg  50541605451_8564d0279a_k.jpg  51345631281_ba0f233b7c_k.jpg
2480028401_c325f59709_k.jpg   32178534444_eba729ed7f_k.jpg  50791488916_6acc6fb002_k.jpg  51357939936_3652ef8989_k.jpg
25011113087_02ff51099e_k.jpg  33994406348_1629ae4e9c_k.jpg  50899859382_12d5eebbcc_k.jpg  51389523157_e564256b65_k.jpg
25050827701_87b701be6d_k.jpg  36419625564_88c578bff7_k.jpg  50931301453_4f13ce750a_k.jpg  51391300535_7212caac01_k.jpg
25144108165_db6c36bcdc_k.jpg  36443288663_f4a6dc1fc1_k.jpg  51131999811_e09c8c7c87_k.jpg  51619205263_156d780bb9_k.jpg
25269111389_e2ed516dd7_k.jpg  4666028952_32f2055d56_k.jpg   51165672541_a46d9e41c6_k.jpg  8226774297_d0c1d2e6dc_k.jpg
25422896582_2d8dc79f70_k.jpg  4927459373_1692a72008_k.jpg   51199164534_af55a7d739_k.jpg  25999653882_01609b1473_k.jpg  49628614811_d1af91dfd2_k.jpg  51217247762_af17283c35_k.jpg
30497771915_b333bb77f7_k.jpg  49817420326_f8d6fede7e_k.jpg  51218173723_50d4ad6656_k.jpg  lost+found
~~~

Notre clé USB a bien tout les fichiers attendu. Nous pouvons donc passer au montage de la clé sur la board.

~~~cmd
~ # usb 1-2: new high-speed USB device number 2 using atmel-ehci
usb 1-2: New USB device found, idVendor=0781, idProduct=5580, bcdDevice= 0.10
usb 1-2: New USB device strings: Mfr=1, Product=2, SerialNumber=3
usb 1-2: Product: Extreme
usb 1-2: Manufacturer: SanDisk
usb 1-2: SerialNumber: AA010824151931565434
usb-storage 1-2:1.0: USB Mass Storage device detected
scsi host0: usb-storage 1-2:1.0
scsi 0:0:0:0: Direct-Access     SanDisk  Extreme          0001 PQ: 0 ANSI: 6
sd 0:0:0:0: [sda] 61282631 512-byte logical blocks: (31.4 GB/29.2 GiB)
sd 0:0:0:0: [sda] Write Protect is off
sd 0:0:0:0: [sda] Write cache: disabled, read cache: enabled, doesn't support DPO or FUA
sd 0:0:0:0: [sda] Attached SCSI removable disk
~ # 
~ # 
~ # mkdir /media
~ # chmod -R g-w,o-w /media
~ # cd /media
/media # mkdir sda
/media # chmod -R g-w,o-w sda
/media # mount /dev/sda sda
EXT4-fs (sda): recovery complete
EXT4-fs (sda): mounted filesystem with ordered data mode. Quota mode: disabled.
/media # cd sda/
/media/sda # ls
16138501907_815e50d51a_k.jpg  49817420326_f8d6fede7e_k.jpg
2480028401_c325f59709_k.jpg   50541605451_8564d0279a_k.jpg
25011113087_02ff51099e_k.jpg  50791488916_6acc6fb002_k.jpg
25050827701_87b701be6d_k.jpg  50899859382_12d5eebbcc_k.jpg
25144108165_db6c36bcdc_k.jpg  50931301453_4f13ce750a_k.jpg
25269111389_e2ed516dd7_k.jpg  51131999811_e09c8c7c87_k.jpg
25422896582_2d8dc79f70_k.jpg  51165672541_a46d9e41c6_k.jpg
25999653882_01609b1473_k.jpg  51199164534_af55a7d739_k.jpg
30497771915_b333bb77f7_k.jpg  51217247762_af17283c35_k.jpg
30941582392_818906a7e0_o.jpg  51218173723_50d4ad6656_k.jpg
32178534444_eba729ed7f_k.jpg  51345631281_ba0f233b7c_k.jpg
33994406348_1629ae4e9c_k.jpg  51357939936_3652ef8989_k.jpg
36419625564_88c578bff7_k.jpg  51389523157_e564256b65_k.jpg
36443288663_f4a6dc1fc1_k.jpg  51391300535_7212caac01_k.jpg
4666028952_32f2055d56_k.jpg   51619205263_156d780bb9_k.jpg
4927459373_1692a72008_k.jpg   8226774297_d0c1d2e6dc_k.jpg
49628614811_d1af91dfd2_k.jpg  lost+found
~~~

Maintenant que nous avons vu que les éléments externes fonctionne nous pouvons nous lancer dans nos automatisations.

~~~cmd
/ # vi /etc/mdev.conf
~~~

Dans lequel nous mettons : `sd[a-z][0-9]* www:nogroup 440 */etc/mdev/mount.sh $ACTION $MDEV`

~~~cmd
/ # mkdir /etc/mdev
/ # touch /etc/mdev/mount.sh
/ # chmod +x /etc/mdev/mount.sh
/ # chmod -R g-w,o-w /etc/mdev
/ # vi /etc/mdev/mount.sh
~~~

et nous y écrivons :

~~~bash
#!/bin/bash
# Mount USB Stick and copy everything to the Web Server Directory

LOG_FILE="/etc/mdev/mount_log.txt"

if [ "$1" == "add" ]; then

    # Function to log messages
    log() {
        echo "$(date '+%Y-%m-%d %H:%M:%S') - $1" >> "$LOG_FILE"
    }

    # Turn on the LED
    echo "1" > /sys/devices/platform/leds/leds/d3/brightness
    log "LED turned on"

    # Create mounting folder and mount the drive
    mkdir "/media/$2" >> "$LOG_FILE" 2>&1
    mount -r "/dev/$2" "/media/$2" >> "$LOG_FILE" 2>&1
    log "Mounted /dev/$2 to /media/$2"

    # Copy all files to the Web Server Directory 
    install -c -o www -g nogroup -m 644 /media/$2/* /var/www/upload/files/ >> "$LOG_FILE" 2>&1
    log "Copied files to /var/www/upload/files/"

    # Unmount drive and delete the folder
    umount "/media/$2" >> "$LOG_FILE" 2>&1
    rmdir "/media/$2" >> "$LOG_FILE" 2>&1
    log "Unmounted and removed /media/$2"

    # Turn off the LED                                      
    echo "0" > /sys/devices/platform/leds/leds/d3/brightness
    log "LED turned off"

    log "Operation completed"
fi
~~~

Pour que tout cela il ne reste plus qu'à lancer `mdev` en mode daemon automatiquement au boot :

~~~cmd
/ # touch /etc/init.d/S12mdev
/ # chmod +x /etc/init.d/S12mdev
/ # chmod g-w,o-w /etc/init.d/S12mdev
/ # vi /etc/init.d/S12mdev
~~~

Et nous y écrivons :

~~~bash
#!/bin/sh
#
# Start mdev as a daemon
#

mdev -d
~~~

Nous pouvons maintenant faire un test complet en commençant par redémarrer notre carte. Ensuite voici notre dossier image avant l'insertion de la clé :

~~~cmd
/ # ls -la /var/www/upload/files/
drwxr-xr-x    2 www      nogroup       4096 Nov  9  2023 .
drwxr-xr-x    4 www      nogroup       4096 Nov  9  2023 ..
-rw-r--r--    1 www      nogroup    1118982 Nov  9  2023 2_embedded_engineer.png
-rw-r--r--    1 www      nogroup      91810 Nov  9  2023 Embedded_Linux_Primer.jpg
-rw-r--r--    1 www      nogroup      82074 Nov  9  2023 linux_distribs.jpg
-rw-r--r--    1 www      nogroup      64031 Nov  9  2023 preinternet_chat_rooms.jpg
-rw-r--r--    1 www      nogroup      11828 Nov  9  2023 sudo.png
-rw-r--r--    1 www      nogroup      71063 Nov  9  2023 upgrading.png
~~~

Nous insérons notre clé USB :

![Plug USB into Board](USB_into_Board.gif)

Et voici ce que nous avons après l'extinction de la LED :

~~~cmd
~ # cat /etc/mdev/mount_log.txt 
2007-01-01 01:16:37 - LED turned on
2007-01-01 01:16:37 - Mounted /dev/sda to /media/sda
ㅤinstall: omitting directory '/media/sda/lost+found'
2007-01-01 01:16:42 - Copied files to /var/www/upload/files/
2007-01-01 01:16:42 - Unmounted and removed /media/sda
2007-01-01 01:16:42 - LED turned off
2007-01-01 01:16:42 - Operation completed
~ #
~ #
~ # ls -la /var/www/upload/files/
drwxr-xr-x    2 www      nogroup       4096 Nov 25  2023 .
drwxr-xr-x    4 www      nogroup       4096 Nov  9  2023 ..
-rw-r--r--    1 www      nogroup     695976 Nov 25  2023 16138501907_815e50d51a_k.jpg
-rw-r--r--    1 www      nogroup     520834 Nov 25  2023 2480028401_c325f59709_k.jpg
-rw-r--r--    1 www      nogroup     362180 Nov 25  2023 25011113087_02ff51099e_k.jpg
-rw-r--r--    1 www      nogroup     652043 Nov 25  2023 25050827701_87b701be6d_k.jpg
-rw-r--r--    1 www      nogroup     989430 Nov 25  2023 25144108165_db6c36bcdc_k.jpg
-rw-r--r--    1 www      nogroup     556804 Nov 25  2023 25269111389_e2ed516dd7_k.jpg
-rw-r--r--    1 www      nogroup     470387 Nov 25  2023 25422896582_2d8dc79f70_k.jpg
-rw-r--r--    1 www      nogroup     241535 Nov 25  2023 25999653882_01609b1473_k.jpg
-rw-r--r--    1 www      nogroup    1118982 Nov  9  2023 2_embedded_engineer.png
-rw-r--r--    1 www      nogroup     443425 Nov 25  2023 30497771915_b333bb77f7_k.jpg
-rw-r--r--    1 www      nogroup     797427 Nov 25  2023 30941582392_818906a7e0_o.jpg
-rw-r--r--    1 www      nogroup     309397 Nov 25  2023 32178534444_eba729ed7f_k.jpg
-rw-r--r--    1 www      nogroup     600924 Nov 25  2023 33994406348_1629ae4e9c_k.jpg
-rw-r--r--    1 www      nogroup     873670 Nov 25  2023 36419625564_88c578bff7_k.jpg
-rw-r--r--    1 www      nogroup    1180958 Nov 25  2023 36443288663_f4a6dc1fc1_k.jpg
-rw-r--r--    1 www      nogroup     866519 Nov 25  2023 4666028952_32f2055d56_k.jpg
-rw-r--r--    1 www      nogroup     365919 Nov 25  2023 4927459373_1692a72008_k.jpg
-rw-r--r--    1 www      nogroup     607960 Nov 25  2023 49628614811_d1af91dfd2_k.jpg
-rw-r--r--    1 www      nogroup     460307 Nov 25  2023 49817420326_f8d6fede7e_k.jpg
-rw-r--r--    1 www      nogroup    1078358 Nov 25  2023 50541605451_8564d0279a_k.jpg
-rw-r--r--    1 www      nogroup     386779 Nov 25  2023 50791488916_6acc6fb002_k.jpg
-rw-r--r--    1 www      nogroup     292952 Nov 25  2023 50899859382_12d5eebbcc_k.jpg
-rw-r--r--    1 www      nogroup     757407 Nov 25  2023 50931301453_4f13ce750a_k.jpg
-rw-r--r--    1 www      nogroup     694012 Nov 25  2023 51131999811_e09c8c7c87_k.jpg
-rw-r--r--    1 www      nogroup     711957 Nov 25  2023 51165672541_a46d9e41c6_k.jpg
-rw-r--r--    1 www      nogroup     206612 Nov 25  2023 51199164534_af55a7d739_k.jpg
-rw-r--r--    1 www      nogroup     228209 Nov 25  2023 51217247762_af17283c35_k.jpg
-rw-r--r--    1 www      nogroup     281359 Nov 25  2023 51218173723_50d4ad6656_k.jpg
-rw-r--r--    1 www      nogroup     757760 Nov 25  2023 51345631281_ba0f233b7c_k.jpg
-rw-r--r--    1 www      nogroup     404991 Nov 25  2023 51357939936_3652ef8989_k.jpg
-rw-r--r--    1 www      nogroup     625829 Nov 25  2023 51389523157_e564256b65_k.jpg
-rw-r--r--    1 www      nogroup     693915 Nov 25  2023 51391300535_7212caac01_k.jpg
-rw-r--r--    1 www      nogroup     631995 Nov 25  2023 51619205263_156d780bb9_k.jpg
-rw-r--r--    1 www      nogroup     311201 Nov 25  2023 8226774297_d0c1d2e6dc_k.jpg
-rw-r--r--    1 www      nogroup      91810 Nov  9  2023 Embedded_Linux_Primer.jpg
-rw-r--r--    1 www      nogroup      82074 Nov  9  2023 linux_distribs.jpg
-rw-r--r--    1 www      nogroup      64031 Nov  9  2023 preinternet_chat_rooms.jpg
-rw-r--r--    1 www      nogroup      11828 Nov  9  2023 sudo.png
-rw-r--r--    1 www      nogroup      71063 Nov  9  2023 upgrading.png
~~~

Tout est donc en ordre.
En bonus nous pouvons voir que nous recevons énormément de message d'information de la part du kernel quand nous mettons une clé USB dans notre board :

~~~cmd
~ # usb 1-2: new high-speed USB device number 4 using atmel-ehci
usb 1-2: New USB device found, idVendor=0781, idProduct=5580, bcdDevice= 0.10
usb 1-2: New USB device strings: Mfr=1, Product=2, SerialNumber=3
usb 1-2: Product: Extreme
usb 1-2: Manufacturer: SanDisk
usb 1-2: SerialNumber: AA010824151931565434
usb-storage 1-2:1.0: USB Mass Storage device detected
scsi host0: usb-storage 1-2:1.0
scsi 0:0:0:0: Direct-Access     SanDisk  Extreme          0001 PQ: 0 ANSI: 6
sd 0:0:0:0: [sda] 61282631 512-byte logical blocks: (31.4 GB/29.2 GiB)
sd 0:0:0:0: [sda] Write Protect is off
sd 0:0:0:0: [sda] Write cache: disabled, read cache: enabled, doesn't support DPO or FUA
sd 0:0:0:0: [sda] Attached SCSI removable disk
EXT4-fs (sda): mounted filesystem with ordered data mode. Quota mode: disabled.
EXT4-fs (sda): unmounting filesystem.
usb 1-2: USB disconnect, device number 4
~~~

si nous souhaitons que tout cela se fasse de manière un peu plus discrète, pour tout nos test par exemple, il suffit de changer le niveau des message de la part du kernel qui seront affiché dans notre terminal :

~~~cmd
~ # dmesg -n 2
~~~

### Q2 - Donnez l’emplacement et le contenu des scripts que vous avez implémentés

Vous trouverez tout cela juste en dessus.

## Support USB avec des modules noyau

Nous allons préparer notre nouveau noyau avec les modules pour l'USB 2.0 et 1.1 :
~~~cmd
❯ make ARCH=arm CROSS_COMPILE=arm-linux- menuconfig
~~~

- Dans `Device Drivers` :
  - `USB Support`
    - `Support for Host-side USB` doit être en `<M>` pour "module".
  - `Network device support`
    - `USB Network Adapters` peut être désactivé.
    - `Wireless LAN` peut être désactivé.
  - `Misc devices`
    - `EEPROM support`
      - `EEPROM 93CX6 support` doit être en `<*>`.
  - `Hardware Monitor support`
    - `JEDEC JC42.4 compliant memory module temperature sensors` doit être en `<*>`
  - `Voltage and Current Regulator Support`
    - `PWM voltage regulator` doit être en `<*>`.
  - `Multimedia support`
    - `Media drivers`
      - `Media USB Adapters`
        - `USB Video Class (UVC)` peut être désactivé.
    - `Media ancillary drivers`
      - `Camera sensor devices` peut être désactivé.
  - `Sound card support` peut être désactivé.
  - `HID support`
    - `USB HID support`
      - `USB HID transport layer` peut être désactivé.
  - `Industrial I/O support`
    - `Analog to digital converters`
      - `Envelope detector using a DAC and a comparator` peut être désactivé.
    - `Digital to analog converters`
      - `DAC emulation using a DPOT` peut être désactivé.
    - `Digital potentiometers`
      - `Microchip MCP45xx/MCP46xx Digital Potentiometer driver` peut être désactivé.
- Dans `File systems`:
  - `Kernel automounter support` doit être en `<*>`.
- Dans `Library routines`:
  - `CRC-CCITT functions` doit être en `<*>`.
  - `CRC ITU-T V.41 function` doit être en `<*>`.

Lors de cette modification, plusieurs autres options dont nous avons besoin ont été automatiquement modifié en mode "module" : `EHCI HCD (USB 2.0) support`, `OHCI HCD (USB 1.1) support`.

Nous pouvons maintenant recompiler notre noyau puis installer les modules dans le rootfs.

~~~cmd
❯ make ARCH=arm CROSS_COMPILE=arm-linux- -j $(nproc)
[Kernel Compiling...]
❯ sudo make ARCH=arm CROSS_COMPILE=arm-linux- modules_install INSTALL_MOD_PATH=../../nfsroot
❯ rm /srv/tftp/zImage
❯ cp arch/arm/boot/zImage /srv/tftp/
~~~

Nous vérifions maintenant, depuis notre board, que les modules USB sont bien présent :

~~~cmd
/lib/modules/6.1.56/kernel/drivers/usb # ls -Rt
.:
class    core     host     serial   storage

./class:
cdc-acm.ko

./core:
usbcore.ko

./host:
ehci-atmel.ko  ehci-hcd.ko    ohci-at91.ko   ohci-hcd.ko

./serial:
ftdi_sio.ko   pl2303.ko     usbserial.ko

./storage:
usb-storage.ko
~~~

Nous pouvons maintenant redémarrer notre board et Flash notre nouveau Kernel sur la NAND en sélectionnant `U-Boot console` dans notre Boot Menu et en exécutant :

~~~cmd
=> $update_kernel
ㅤethernet@f0028000: PHY present at 7
ㅤethernet@f0028000: Starting autonegotiation...
ㅤethernet@f0028000: Autonegotiation complete
ㅤethernet@f0028000: link up, 1000Mbps full-duplex (lpa: 0x2800)
Using ethernet@f0028000 device
TFTP from server 192.168.1.1; our IP address is 192.168.1.2
Filename 'zImage'.
Load address: 0x21000000
ㅤLoading: #################################################################
	 #################################################################
	 #################################################################
	 #########################################################
	 681.6 KiB/s
done
Bytes transferred = 3694504 (385fa8 hex)

NAND erase: device 0 offset 0x160000, size 0x385fa8
Erasing at 0x4e0000 -- 100% complete.
OK

NAND write: device 0 offset 0x160000, size 0x385fa8
 3694504 bytes written: OK
~~~

Pour terminé nous redémarrons la carte et retournons sur notre Linux. Si nous branchons notre clé USB, plus rien ne fonctionne. Ceci indique donc que nous avons bel et bien notre nouveau Kernel et que nous devons à présent charger les bons modules afin de faire fonctionner USB à nouveau.

### Q3 - Quelle est la liste de tous les modules que vous avez déployés sur votre système embarqués (fichiers .ko) ? Est-ce que cette liste contient des modules autres que pour l’USB ?

Voici la liste complète :

~~~cmd
/lib/modules/6.1.56/kernel # find -name *.ko
./net/802/psnap.ko
./net/802/p8022.ko
./net/802/stp.ko
./net/bridge/bridge.ko
./net/8021q/8021q.ko
./net/llc/llc.ko
./net/dsa/dsa_core.ko
./drivers/net/phy/microchip.ko
./drivers/usb/serial/pl2303.ko
./drivers/usb/serial/usbserial.ko
./drivers/usb/serial/ftdi_sio.ko
./drivers/usb/core/usbcore.ko
./drivers/usb/class/cdc-acm.ko
./drivers/usb/storage/usb-storage.ko
./drivers/usb/host/ohci-at91.ko
./drivers/usb/host/ohci-hcd.ko
./drivers/usb/host/ehci-atmel.ko
./drivers/usb/host/ehci-hcd.ko
./drivers/mtd/tests/mtd_speedtest.ko
./drivers/mtd/tests/mtd_readtest.ko
./drivers/mtd/tests/mtd_nandecctest.ko
./drivers/mtd/tests/mtd_torturetest.ko
./drivers/mtd/tests/mtd_pagetest.ko
./drivers/mtd/tests/mtd_stresstest.ko
./drivers/mtd/tests/mtd_nandbiterrs.ko
./drivers/mtd/tests/mtd_subpagetest.ko
./drivers/mtd/tests/mtd_oobtest.ko
./crypto/af_alg.ko
./crypto/algif_hash.ko
./crypto/algif_skcipher.ko
~~~

Oui il y a d'autre modules que ceux de l'USB. Je ne les ai sois, pas trouvé dans le menuconfig ou alors se sont des modules dont nous avons besoin de toute façons et qui ne peuvent seulement être en mode module.

Afin de déterminer quels modules sont chargé pour que le noyau détecte toute clé USB insérée nous prouvons y aller étape par étape :

~~~cmd
/lib/modules/6.1.56 # cat modules.dep
kernel/crypto/af_alg.ko:
kernel/crypto/algif_hash.ko: kernel/crypto/af_alg.ko
kernel/crypto/algif_skcipher.ko: kernel/crypto/af_alg.ko
kernel/drivers/mtd/tests/mtd_oobtest.ko:
kernel/drivers/mtd/tests/mtd_pagetest.ko:
kernel/drivers/mtd/tests/mtd_readtest.ko:
kernel/drivers/mtd/tests/mtd_speedtest.ko:
kernel/drivers/mtd/tests/mtd_stresstest.ko:
kernel/drivers/mtd/tests/mtd_subpagetest.ko:
kernel/drivers/mtd/tests/mtd_torturetest.ko:
kernel/drivers/mtd/tests/mtd_nandecctest.ko:
kernel/drivers/mtd/tests/mtd_nandbiterrs.ko:
kernel/drivers/net/phy/microchip.ko:
kernel/drivers/usb/core/usbcore.ko:
kernel/drivers/usb/host/ehci-hcd.ko: kernel/drivers/usb/core/usbcore.ko
kernel/drivers/usb/host/ehci-atmel.ko: kernel/drivers/usb/host/ehci-hcd.ko kernel/drivers/usb/core/usbcore.ko
kernel/drivers/usb/host/ohci-hcd.ko: kernel/drivers/usb/core/usbcore.ko
kernel/drivers/usb/host/ohci-at91.ko: kernel/drivers/usb/host/ohci-hcd.ko kernel/drivers/usb/core/usbcore.ko
kernel/drivers/usb/class/cdc-acm.ko: kernel/drivers/usb/core/usbcore.ko
kernel/drivers/usb/storage/usb-storage.ko: kernel/drivers/usb/core/usbcore.ko
kernel/drivers/usb/serial/usbserial.ko: kernel/drivers/usb/core/usbcore.ko
kernel/drivers/usb/serial/ftdi_sio.ko: kernel/drivers/usb/serial/usbserial.ko kernel/drivers/usb/core/usbcore.ko
kernel/drivers/usb/serial/pl2303.ko: kernel/drivers/usb/serial/usbserial.ko kernel/drivers/usb/core/usbcore.ko
kernel/net/802/p8022.ko: kernel/net/llc/llc.ko
kernel/net/802/psnap.ko: kernel/net/llc/llc.ko
kernel/net/802/stp.ko: kernel/net/llc/llc.ko
kernel/net/8021q/8021q.ko:
kernel/net/llc/llc.ko:
kernel/net/bridge/bridge.ko: kernel/net/802/stp.ko kernel/net/llc/llc.ko
kernel/net/dsa/dsa_core.ko: kernel/net/bridge/bridge.ko kernel/net/802/stp.ko kernel/net/llc/llc.ko
~~~

Nous pouvons assumer que `usb-storage.ko` et donc `usbcore.ko` seront nécessaires :

~~~cmd
/lib/modules/6.1.56 # modprobe usbcore
ㅤusbcore: registered new interface driver usbfs
ㅤusbcore: registered new interface driver hub
ㅤusbcore: registered new device driver usb
/lib/modules/6.1.56 # modprobe usb-storage
ㅤusbcore: registered new interface driver usb-storage
~~~

La clé USB ne fonctionne pas encore. Ensuite le support pour l'USB 1.1 et 2.0 :

~~~cmd
/lib/modules/6.1.56 # modprobe ehci-hcd
/lib/modules/6.1.56 # modprobe ohci-hcd
~~~

Testons la clé USB... Toujours rien :

~~~cmd
/lib/modules/6.1.56 # modprobe ehci-atmel
atmel-ehci 700000.ehci: EHCI Host Controller
atmel-ehci 700000.ehci: new USB bus registered, assigned bus number 1
atmel-ehci 700000.ehci: irq 57, io mem 0x00700000
atmel-ehci 700000.ehci: USB 2.0 started, EHCI 1.00
usb usb1: New USB device found, idVendor=1d6b, idProduct=0002, bcdDevice= 6.01
usb usb1: New USB device strings: Mfr=3, Product=2, SerialNumber=1
usb usb1: Product: EHCI Host Controller
usb usb1: Manufacturer: Linux 6.1.56 ehci_hcd
usb usb1: SerialNumber: 700000.ehci
hub 1-0:1.0: USB hub found
hub 1-0:1.0: 3 ports detected
/lib/modules/6.1.56 # modprobe ohci-at91
at91_ohci 600000.ohci: USB Host Controller
at91_ohci 600000.ohci: new USB bus registered, assigned bus number 2
at91_ohci 600000.ohci: irq 57, io mem 0x00600000
usb usb2: New USB device found, idVendor=1d6b, idProduct=0001, bcdDevice= 6.01
usb usb2: New USB device strings: Mfr=3, Product=2, SerialNumber=1
usb usb2: Product: USB Host Controller
usb usb2: Manufacturer: Linux 6.1.56 ohci_hcd
usb usb2: SerialNumber: at91
hub 2-0:1.0: USB hub found
hub 2-0:1.0: 3 ports detected
~~~

Voilà qui est prométeur ! Testons notre clé USB : La LED s'allume le script se déclanche, les images apparaissent à nouveau sur le server Web et le kernel nous dis :

~~~cmd
~ # usb 1-2: new high-speed USB device number 2 using atmel-ehci
usb 1-2: New USB device found, idVendor=0781, idProduct=5580, bcdDevice= 0.10
usb 1-2: New USB device strings: Mfr=1, Product=2, SerialNumber=3
usb 1-2: Product: Extreme
usb 1-2: Manufacturer: SanDisk
usb 1-2: SerialNumber: AA010824151931565434
usb-storage 1-2:1.0: USB Mass Storage device detected
scsi host0: usb-storage 1-2:1.0
scsi 0:0:0:0: Direct-Access     SanDisk  Extreme          0001 PQ: 0 ANSI: 6
sd 0:0:0:0: [sda] 61282631 512-byte logical blocks: (31.4 GB/29.2 GiB)
sd 0:0:0:0: [sda] Write Protect is off
sd 0:0:0:0: [sda] Write cache: disabled, read cache: enabled, doesn't support DPO or FUA
sd 0:0:0:0: [sda] Attached SCSI removable disk
/dev/sda: Can't open blockdev
/dev/sda: Can't open blockdev
EXT4-fs (sda): mounted filesystem with ordered data mode. Quota mode: disabled.
EXT4-fs (sda): unmounting filesystem.
~~~

Notre clé USB a donc bien été détectée !

### Q4 - Donnez les noms des modules nécessaires à votre noyau pour que la clé USB soit utilisable en tant que périphérique de stockage de masse. Veuillez décrire les modules sous forme d’arbres décrivant les dépendences intra-modules.

`ehci-atmel`<br>
│<br>
├─ `ehci-hcd`<br>
│ㅤㅤㅤㅤ└─ `usbcore`<br>
└─ `usbcore`<br>


`ohci-at91`<br>
│<br>
├─ `ohci-hcd`<br>
│ㅤㅤㅤㅤ└─ `usbcore`<br>
└─ `usbcore`<br>

`usb-storage`<br>
│<br>
└─ `usbcore`<br>

Nous avons donc `usbcore`, `usb-storage`, `ohci-hcd`, `ohci-at91`, `ehci-hcd` et `ehci-atmel`.

Nous voulons donc que tout ces modules se charge automatiquement au boot :

~~~cmd
/ # touch /etc/init.d/S13usb
/ # chmod +x /etc/init.d/S13usb
/ # chmod g-w,o-w /etc/init.d/S13usb
/ # vi /etc/init.d/S13usb
~~~

Dans lequel nous mettons :

~~~bash
#!/bin/sh
#
# Start Modules for USB Storage Support
#
 
modprobe ehci-atmel
modprobe ohci-at91
modprobe usb-storage
~~~

Comme on peut le voir nous n'avons qu'à charger les tête des arbres de la `Q4` car la commande `modprobe` charge les modules **et** ces dépendances.

Nous pouvons finalement tester le tout en redémmarant la board et en insérent la clé usb.

### Q5 - Quel fichier avez-vous ajouté à votre système pour ce dernier point et quel est son contenu ?

J'ai créer un nouveau Service (script) `S13usb` dans le `init.d`:

~~~bash
#!/bin/sh
#
# Start Modules for USB Storage Support
#
 
modprobe ehci-atmel
modprobe ohci-at91
modprobe usb-storage
~~~

# Creators

**Michael Divià**

- <https://gitedu.hesge.ch/michael.divia>

# Copyright and license

Code and documentation copyright 2023 the authors. Code released under the [MIT License](https://gitedu.hesge.ch/flg_courses/embedded_linux/students/michael.divia/-/blob/master/LICENSE).

Enjoy :metal:
