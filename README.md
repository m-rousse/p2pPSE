# p2pPSE

C'est un logiciel permettant d'échanger des fichiers en P2P sur un protocole similaire au torrent.

Il est réalisé dans le cadre d'un projet d'étude sur le thème des communications client-serveur.
(La librairie libPSE a été développée par nos enseignants à cet effet.)

#Install

p2pPSE dépend des librairies ncurses, ncursesw et dialog.

```bash
apt-get install libncurses5 libncurses5-dev libncursesw5 libncursesw5-dev
```

#ToDo
- client : thread upload
- client : thread download
- client : fonction découpage fichiers

#Infos
Port d'écoute du serveur 24240  
Port d'écoute du client 24241

#Idées
- Supprimer le serveur et faire une detection en broadcast
