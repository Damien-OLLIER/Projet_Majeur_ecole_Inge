#####Import#####

import paho.mqtt.client as mqtt
import json
import random
import requests
import base64
import datetime

#####Initialize the Client###########

#Variables
BROKERIP = "eu1.cloud.thethings.network"
BROKERPORT = 1883
TOPIC = "v3/test-cpe@ttn/devices/eui-70b3d57ed0059615/up"
USERNAME = "test-cpe@ttn"
PASSWORD = "NNSXS.TQNODSPOTAUCYRNFTRMXHLHML6AJLZEGR4DVZHA.EBWLM3GUDPHFJUTHKCPGMXCPCXFCBDH5FBBGA4ZF377HX644INAA"
CLIENT_ID = f'test-cpe@ttn-{random.randint(0, 100)}'

APIURL = "https://ct7hsc8zyk.execute-api.us-east-1.amazonaws.com/prod"

ID = 1

CPELAT = 45.7841601
CPELONG = 4.8675628


def on_connect(client, userdata, flags, rc):
    """Coonect to broker and subscribe to /data topic"""
    # This will be called once the client connects
    print(f"Connected with result code {rc}")
    # Subscribe here!
    client.subscribe(TOPIC)


def on_message(client, userdata, msg):
    """print msg when payload received"""
    reponse = json.loads(str(msg.payload.decode()))
    print(f"Message received {reponse}")

    try :
        test = reponse["uplink_message"]["frm_payload"]
        print(test)
        donnees = traitement(reponse)
    except:
        donnees = None

    if donnees == None:
        print("Pas d'envoi")
    else:
        envoi(donnees)


def traitement(reponse):
    global ID
    global CPELONG
    global CPELONG
    """process msg when payload received"""
    print("process data")

    value = reponse["uplink_message"]["frm_payload"]
    value = base64.b64decode(value)
    print(str(value))
    donnees = str(value.decode()).split(":")

    try:
        capteurCo2 = float(donnees[0])
        print(str(capteurCo2))
        capteurFeu = donnees[1]
        print(str(capteurFeu))

        # Analyse capteur feu
        if str(capteurFeu) == "1":
            print("DANGER !! Le capteur détecte du Feu, veuillez prendre les précautions nécessaires")
            etatActuelFeu = "FEU"
        elif str(capteurFeu) == "0":
            print("Le capteur ne détecte aucun danger")
            etatActuelFeu = "RAS"
        else:
            etatActuelFeu = "Err"
            print("Mauvaise valeur")

        # Analyse capteur Co2
        if capteurCo2 < 3000:
            print("DANGER !! Le capteur détecte une concentration en CO2 très faible, veuillez prendre les précautions nécessaires")
            etatActuelCO2 = "CO2 FAIBLE"
        elif (capteurCo2 >= 3000) and (capteurCo2 < 35000):
            print("Le capteur ne détecte aucun danger")
            etatActuelCO2 = "RAS"
        elif (capteurCo2 >= 35000) and (capteurCo2 < 70000):
            print("DANGER !! Le capteur détecte une concentration en CO2 très élevée, veuillez prendre les précautions nécessaires")
            etatActuelCO2 = "CO2 ELEVEE"
        elif capteurCo2 > 70000:
            print("DANGER !! Le capteur détecte une concentration en CO2 maximale, veuillez prendre les précautions nécessaires")
            etatActuelCO2 = "ALERTE CO2"
        else : 
            etatActuelCO2 = "Analyse impossible"

        jsonenvoi = {"id":str(ID), "valueFeu":str(capteurFeu), "valueCO2":str(capteurCo2), "etatFeu":etatActuelFeu, "etatCO2":etatActuelCO2, "CapteurPosX": str(CPELAT), "CapteurPosY":str(CPELONG), "NumLabo":"A"+str(random.randint(100,200)), "date":str(datetime.datetime.now())}

        ID = ID + 1

        print("data processed")

        if str(etatActuelFeu) == "Err":
            return None
        else:
            return jsonenvoi

    except Exception as ex:

        print("données erronées -> skip error code: %s", ex)

        return None

def envoi(donnees):
    """send msg after processing"""
    print("send data")

    x = requests.post(f"{APIURL}/capteur", json = donnees)
    print(x.json())

    print("data send")


#print("send data")
#data = {'api-pk':'test', 'name':'test'}
#headers = {'Content-type': 'application/json'}
#x = requests.post(f"{APIURL}/test", json = data, headers=headers)
#print( x.json())
#print("data send")

#index = requests.get(f"{APIURL}/capteurs")
#print(index.json())

#parametervalue = {"id":"1"}
#indexBis = requests.get(f"{APIURL}/capteur", params=parametervalue)

#print(indexBis.json())

#for i in range (1, 50):

#    jsonData = {"id":str(i), "valueFeu":"0", "valueCO2":"40000.50", "etatFeu":"RAS", "etatCO2":"RAS"}

#    x = requests.post(f"{APIURL}/capteur", json = jsonData)
#    print(x.json())

client = mqtt.Client(CLIENT_ID) 
client.username_pw_set(USERNAME, PASSWORD)
client.on_connect = on_connect
client.on_message = on_message
client.connect(BROKERIP, BROKERPORT)
client.loop_forever()  # Start networking daemon

