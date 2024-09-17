# lab2

- Une description des objectifs du labo aurait été souhaitable
- Incomplet : picocom sur ttyUSB0 fonctionne vraiment sans rien faire de particulier ?
- Incomplet : comment et où avez-vous récupéré le code source d’at91bootstrap pour la bonne version ?
- Q1 : justification de la configuration choisie manquante
- Q2 : une explication/description de se que vous présentez aurait été souhaitable
- Incomplet : comment avez-vous compilé at91 ? Aucune explication
- Incomplet : comment et où avez-vous récupéré le code source d’U-boot ?
- Incomplet : comment avez-vous compilé u-boot ? Aucune explication
- bootmenu : incomplet : cette partie est absente de votre journal
- Q5 : justificaiton manquante
- Q6 : une explication/description de se que vous présentez aurait été souhaitable
- sam-ba : nandflash : justification manquante de la valeur utilisée pour le header ; aussi pourquoi ne spécifiez-vous pas le header lors du 2ème appel ?
- Explications manquantes sur l’installation du serveur tftp sur le host
- Explications manquantes sur la configuration de l’interface réseau sur le host et la cible
- Sur quelle interface Ethernet avez-vous connectée votre câble rj45 ?
- Q11 : les valeurs hexadécimales présentées ne sont pas parlantes, difficile de se convaincre que c’est le résultat attendu ; de plus, pourquoi inspectez-vous l’adresse 0x100000 ? Qu’espérez-vous y trouver ?

eval: 0.6

# lab4

+ bien : commits fréquents
- mise en place nfsroot exporté en nfs: une explication du “sed '$d' /etc/exports | sudo tee /etc/exports” aurait été souhaitable
- Q8: vous n’avez pas créé le patch comme demandé

eval: 0.9

# lab5

- Lancement de httpd : vous ne spécifiez que le user, pas le groupe, aussi spécifiez plutôt le nom que le GID (plus parlant)
- La description de votre mdev.conf aurait été souhaitable
- mdev.conf: seule la détection de l’insertion nous intéresse car une fois la copie terminée, le device doit être démonté automatiquement ; votre script devrait seulement être executé à l’insertion
+ vidéo de l’insertion de la clé
- Votre arbre des modules n’est pas vraiment un arbre
- La manière dont les dépendances de l’arbre des modules sont représentées n’est pas clair : manque une description de la signification de l’arbre càd où se trouve la/les dépendences
- résultats/validation du fonctionnement avec les modules pour l’usb ?

eval: 0.8

# lab7

+ Très bien
- Votre rootfs final est monté en read/write plutôt qu'en read-only comme demandé

eval: 1

# lab8

- driver: tests  d'inspection de /dev, /proc, /sys pour irq, class, déchargement du module, etc. auraient été souhaitables
- Q7: incorrect : -l ne permet pas de spécifier le shell à utiliser
- modif ctris: explications bcp trop succintes de vos modifications, p.ex. role de DELAY_INPUT ?
- modif ctris: il aurait été préférable d’ouvrir le device 1x au début de ctris, plutot que de faire un open() avant chaque read()

eval: 0.9
