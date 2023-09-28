
# 1. Projet Mob - IoT
- [1. Projet Mob - IoT](#1-projet-mob---iot)
  - [1.1. Équipe de 4 personnes](#11-équipe-de-4-personnes)
  - [1.2. Description du Projet](#12-description-du-projet)
    - [1.2.1. Matériel](#121-matériel)
    - [1.2.2. Cas d'utilisation](#122-cas-dutilisation)
  - [1.3. Répartition des tâches](#13-répartition-des-tâches)
    - [1.3.1. Suivi journalier](#131-suivi-journalier)
  - [1.4. Procédure de mise en place de votre chaîne IoT](#14-procédure-de-mise-en-place-de-votre-chaîne-iot)
  - [1.5. Conclusions et recommandations](#15-conclusions-et-recommandations)

## 1.1. Équipe de 4 personnes
Notre équipe projet est composée par :

- Damien OLLIER: j'ai mise en place le code dans la carte arduino pour gerer les capteurs (flame, air quality et carte wifi)
- Sami Hadjeb
- Antoine Sterna
- Camil Coullon (Chef de groupe)

## 1.2. Description du Projet

Le sujet initial est dans le fichier [sujet.md](sujet.md)
Vous devez décrire ici les fonctionnalités et applications de la maquette que vous avez décidé d'implementer.

Le but est de créer un boîtier capable de détecter un feu et d'alerter rapidement lorsque la qualité de l'air se détériore dans un milieu assez confiné. Pour faire ce boîtier, nous détectons la présence de feu et nous mesurons la qualité de l'air à l'aide de deux capteurs reliés à une carte arduino. Les informations récoltées sont ensuite envoyés à la carte Lora par l'intermédiaire de deux cartes wifi qui communiquent entres elles.

### 1.2.1. Matériel
| Nombre         | Description        |
| ---          | :---         |
|     1    |    Carte arduino et son cable d'alimentation    |
|   1      |        Capteur FLAME Click |
|   2      |        Capteur Wifi BLE Click |
|   1      |        Capteur Air Quality Click |
|  1       |     Carte Lora  stm32  |
|       1  |      Carte Beagle Bone black  et son cable d'alimentation |
|    1     |   Carte SD      |

### 1.2.2. Cas d'utilisation

Notre objectif principal est d'utiliser ce capteurs dans les labos de chimie de CPE lyon de le but de sécuriser au maximum ces salles.

Nous pouvons aussi utiliser le capteurs dans les situations suivantes:
- Dans une cabine de camion transportant des produits dangereux
- Dans un bateau (sous-marin)
- Dans un avion

## 1.3. Répartition des tâches

- Damien OLLIER: mise en place de la carte arduino et des capteurs (flame, air quality et carte wifi)
- Sami Hadjeb: mise en place de la carte Beagle Bone black
- Antoine Sterna: mise en place de la partie cloud
- Camil Coullon: mise en place de la carte LoRa

### 1.3.1. Suivi journalier

**Mercredi 11/01/2023** :
- Choix du sujet
- Nous avons récupéré la boite contenant la carte Arduino, la carte Lora et les câbles.
- Nous avons récupéré les deux capteurs: un capteur de flamme et un capteur de qualité de l'air
- Début des tests des capteurs et de la carte arduino.
- Le capteur de flamme marche

**Jeudi 12/01/2023** :

Le matin:
- Cours sur LTE et début du "TP : technologies émergentes dans les réseaux sans fil 4G-LTE"

L'après midi:
- On a continué le TP sur matlab 
- Chacun a commencer à travailler sur sa tâche
- Le capteur de qualité d'air fonction
- Un script a été réalisé pour récupérer les valeurs retournées par les capteurs d'air et de feu

**Vendredi 13/01/2023** :
- Chacun continue de travailler sur sa tâche
- Carte Lora
  - Compilation du lorawan-mbedos-example
  - Configuration de "The Things Network"
  - Envoie des données de base (Dummy sensor value)
- AWS: Ecriture du code terraform pour deployer la configuration sur le compte AW, cela comprend: Un VPC, deux subnets privés, deux subnets publics, une internet gateway, une NAT gateway, toutes les tables de routages appropriées. Ensuite ecriture du code terraform pour déployer une EC2 dans un subnet public avec une Elastic IP et un ficihier user data qui installe automatiquement les dépendances ainsi que docker à la création de l'EC2
- BeagleBone : installation et mise en place internet sur la carte + début du script python
- Il est maintenant possible de se connecter à un réseau Wifi (Hotspot wifi sur un téléphone pour l'instant) avec la carte ESP32

**Lundi 16/01/2023 après-midi** :
- Il est possible de connecter les deux cartes wifi entre elles, une carte en mode client et l'autre carte en mode ap.
- BeagleBone : Communication entre Lora et BBB en MQTT + envoi de données vers le cloud AWS.
- Côté AWS: changement d'infrastructure, il a été décidé de tout faire en serverless en utilisant les services AWS suivants: 
  - API Gateway
  - Lambda
  - DynamoDB
- Carte Lora
  - Auto-formation command AT
  - prise en main de bufferedserial et ATCmdParser (biblio mbedos pour la gestion des commandes AT et de la liaison serie avec le module Wifi)

L'implémentation et le workflow ressemble à ceci:
![schemaAWS](images/AWS%20schema.png)

**Mardi 17/01/2023** :

- Communication entre les deux cartes wifi fonctionnelle
- Il est possible d'envoyer les données récupérées par les capteurs
- Reception des données des capteurs par la BBB 
- Ecriture du code la Lambda pour gérer toutes les requêtes API
- Carte STM32
  - Création de l'Access Point et configuration du serveur TCP
  - définition de toutes les fonctions de base pour la création et pour la réception de data
  - Test de connexion entre client et AP
  - Envoi de données de test puis envoi des données des capteurs
  - Integration de la fonction receiveData à l'envoie vers TTN en lorawan

**Mercredi 18/01/2023** :

- Optimisation des codes (ex: le code qui tourne sur la carte arduino attend la confirmation de la connexion TCP avant d'envoyer les données)
- Rédaction de la documentation
- Mise en place d'un Grafana pour visualiser les données de la dynamoDB
- Carte STM32
  - Test complet de la chaine avec MQTT
  - Optimisation du code pour gestion des erreurs
  - Changement de carte car lenteur d'envoi vers TTN
  - Création de la vidéo
  

**Jeudi 19/01/2023** :

- finalisation de la vidéo et préparation de l'oral
- Résultat de la finale des projets majeurs:
  - Projet le plus technique: 4ème 
  - Projet le plus utile: 2ème
  - Présentation la plus fun: 4ème
  
**Vendredi 19/01/2023** :

- Préparation démonstration live
- Passage à l'oral
