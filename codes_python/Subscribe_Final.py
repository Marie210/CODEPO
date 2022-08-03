# -*- coding: utf-8 -*-
import paho.mqtt.client as mqtt
import json
import os
import csv
import requests
import pvlib
import numpy as np
from bs4 import BeautifulSoup
import matplotlib.pyplot as plt
from tkinter import *


##### Calcul de la puissance fournie par les panneaux solaires ###################################################################################

def calcul_puissance_theorique():
    """
    Calcul de la puissance théorique en sortie des panneaux solaires à partir de l'irradiance journalière
    sur la ville de Kinshasa.
    Le modèle du panneaux solaire se retrouve dans la base de données CECmod (base de donnée de la comission à
    l'énergie Californienne reprenant énormement de modèle de panneaux photovoltaique différent).
    Returns
    -------
    dc_power: liste float
        liste des puissance théoriques fournies par les panneaux solaire entre 8h et 16h.
    """

    # Acquisition des données d'irradiance
    url = "https://fr.tutiempo.net/radiation-solaire/kinshasa.html"
    page = requests.get(url)
    soup = BeautifulSoup(page.content, 'html.parser')
    Irradiance = []
    Irrad_data = soup.find_all("strong")

    # Acquisition des données de température
    url_temp_old = "https://fr.tutiempo.net/kinshasa.html?donnees=dernieres-24-heures"
    page_temp1 = requests.get(url_temp_old)
    soup_Temp = BeautifulSoup(page_temp1.content, 'html.parser')
    Temp = []
    Temp_data = soup_Temp.find_all("td", class_="t Temp")

    # Traitement des données de température et d'irradiance
    count = 0
    for elem_irr in Irrad_data:
        if 3 <= count <= 11:
            Irradiance.append(float(elem_irr.string))
        count += 1

    count_Temp = 0
    for elem_temp in Temp_data:
        if count_Temp <= 8:
            data = elem_temp.string
            count_data = 1
            data_new = data[count_data - 1]
            while data[count_data] != '°':
                data_new += data[count_data]
                count_data += 1
            Temp.append(float(data_new))
        count_Temp += 1

    # Acquisition des données des panneaux solaires à partir de la base de données CECmod
    # cec_mod_db = pvlib.pvsystem.retrieve_sam('CECmod')
    # pdc0 = float(cec_mod_db.iloc[2, 8458])  # STC power
    # gamma_pdc = float(cec_mod_db.iloc[21, 8458]) / 100  # The temperature coefficient in units of 1/C

    pdc0 = 260
    gamma_pdc = -0.0043

    Irradiance_a = np.array(Irradiance)
    Temp_a = np.array(Temp)

    dc_power = pvlib.pvsystem.pvwatts_dc(Irradiance_a, Temp_a, pdc0, gamma_pdc, temp_ref=25.0)
    for i in range(len(dc_power)):
        dc_power[i] = int(dc_power[i] * 20)

    return dc_power.tolist()


##### Traitement des données ###################################################################################

''' Les fonctions ci-dessous permettent de traiter les données provenant du dispositif. 
Le dispositif envoi ses données sous un format JSON qui est réceptioné et convertie en un format 
CSV pour permettre un affichage sur Excel.  '''


def readJson(jsonMessage):
    # remplace les single quotes par des double quotes
    jsonMessage = jsonMessage.replace('\'', '\"')
    jsonMessage = jsonMessage.replace('id', 'batterie')
    # parse le message JSON
    return json.loads(jsonMessage)


def convertToCsvBatteries(jsonLoad):
    # Convertie la partie des JSON relative aux batteries dans  un format CSV
    # input :  string json
    # output : rien mais création d'un fichier CSV
    batteries_data = jsonLoad['batteries']  # Choisis les éléments relatifs aux batteries
    data_file = open('batteries_data.csv', 'w')  # Ouvre un fichier .csv pour écrire dedans
    data_file.write("sep=,\n")
    # create the csv writer object
    csv_writer = csv.writer(data_file)
    # Counter variable used for writing
    # headers to the CSV file
    count = 0
    for emp in batteries_data:
        if count == 0:
            # Writing headers of CSV file
            header = emp.keys()
            csv_writer.writerow(header)
            count += 1

        # Writing data of CSV file
        csv_writer.writerow(emp.values())

    data_file.close()
    # Crée un nouveau fichier sans les sauts de lignes
    with open('batteries_data.csv', newline='') as in_file:
        with open('batteries_data2.csv', 'w', newline='') as out_file:
            writer = csv.writer(out_file)
            for row in csv.reader(in_file):
                if row:
                    writer.writerow(row)
    # On ferme les fichiers
    in_file.close()
    out_file.close()


def convertToCsvSolar(jsonLoad):
    """
    Fonction traduisant les données contenue dans le format json en un fichier CSV
    Parameters
    ----------
    jsonLoad : json text
        fichier json obtenue à partir du text json.
    Returns
    -------
    None.
    """
    data = jsonLoad
    solar_data = data['solarPannels']

    # now we will open a file for writing
    data_file = open('solar_data.csv', 'w')
    data_file.write("sep=,\n")
    csv_writer = csv.writer(data_file)
    count = 0
    for emp in solar_data:
        if count == 0:
            # Writing headers of CSV file
            header = emp.keys()
            csv_writer.writerow(header)
            count += 1

        # Writing data of CSV file
        csv_writer.writerow(emp.values())

    data_file.close()
    csv_name = 'solar_data2.csv'
    with open('solar_data.csv', newline='') as in_file:
        with open(csv_name, 'w', newline='') as out_file:
            writer = csv.writer(out_file)
            for row in csv.reader(in_file):
                if row:
                    writer.writerow(row)

    in_file.close()
    out_file.close()


def erased(file_name):
    """
    Fonction permettant la destruction des fichiers CSV partiel
    Parameters
    ----------
    file_name : string
        nom du fichier à effacer .
    Returns
    -------
    None.
    """
    file = file_name
    if os.path.exists(file) and os.path.isfile(file):
        os.remove(file)


def checkFileExistance(filePath):
    """
    Vérifie l'existance d'un fichier
    Parameters
    ----------
    filePath : string
        nom du fichier à tester .
    Returns
    -------
    bool
        valeur de retour de la fonction 'true' si le fichier existe 'false' sinon .
    """
    try:
        with open(filePath, 'r') as f:
            return True
    except FileNotFoundError as e:
        return False
    except IOError as e:
        return False


def add_csv(old, new):
    """
    Ajoute le contenue du Csv partiel au Csv total.
    Si le fichier total n'existe pas on renome le fichier partiel en fichier total '
    Parameters
    ----------
    old : string
        nom du chier partiel.
    new : string
        nom du fichier total.
    Returns
    -------
    None.
    """

    if checkFileExistance(new):
        lignes_total = []
        lignes_add = []
        with open(new, 'r') as f:

            for elem in f:
                lignes_total.append(elem)

        with open(old, 'r') as f2:
            for i in f2:
                lignes_add.append(i)

        for j in range(len(lignes_total)):
            if j > 1:
                lignes_add.insert(len(lignes_add), lignes_total[j])

        # file = open('sola_data2.csv','r')

        fichier = open(new, 'w')

        for element in lignes_add:
            fichier.write(element)
        fichier.close()
    else:
        # file_oldname = os.path.join("C:\Users\tehoe\OneDrive\Documents\ULB_2022\Projet_cooperation_au_developpement\Code_python",old)
        # file_newname_newfile = os.path.join("C:\Users\tehoe\OneDrive\Documents\ULB_2022\Projet_cooperation_au_developpement\Code_python", new)

        os.rename(old, new)


def correctBatteriesCSV(CSVfile):
    """
    Traite le contenue du CSV_batterie de manière à extraire les listes envoyé par le BMS
    et d'afficher les valeurs contenue dans ces listes heure par heure'
    Parameters
    ----------
    CSVfile : string
        Nom du fichier CSV à traiter .
    Returns
    -------
    dico : liste
        DESCRIPTION.
    """
    nb_mesures = 6
    ext = "["
    ext2 = "]"
    f = open(CSVfile)
    liste = f.readlines()
    f.close()

    n = len(liste[2])
    date = liste[2][n - 10: n - 1]

    liste[1] = liste[1].replace('S', 'State of charge')
    liste[1] = liste[1].replace('V', 'Voltage')
    liste[1] = liste[1].replace('I', 'Courant')
    liste[1] = liste[1].replace('H', 'Heure')
    liste[1] = liste[1].replace('D', 'Date')
    liste[1] = liste[1].replace('N', 'Batterie ID')
    liste[1] = liste[1].replace('T', 'Temperature')
    liste[1] = liste[1].replace('P', 'Puissance ')

    string = liste[0] + liste[1]
    heures = []
    dico = []
    SOC = []
    V = []
    I = []
    P = []

    for k in range(2, len(liste)):
        liste[k] = liste[k].replace(', ', ' ')
        i = 3
        values = []
        for j in range(4):
            lis = liste[k][liste[k].find(ext, i) + 1:liste[k].find(ext2, i)]
            values.append([])
            for elem in lis.split():
                values[j].append(elem)
            i = liste[k].find(ext2, i) + 1
        if k == 2:
            lis = liste[k][liste[k].find(ext, i) + 1:liste[k].find(ext2, i)]
            for elem in lis.split():
                heures.append(elem)
            i = liste[k].find(ext2, i) + 1
            heures.append("moyenne")
        temperature = liste[k][liste[k].find(",", i) + 1:liste[k].find(",", i + 2)]
        i = liste[k].find(",", i + 2)

        for l in range(nb_mesures):
            string = string + str(k - 2) + "," + values[0][l] + "," + values[1][l] + "," + values[2][l] + "," + \
                     values[3][l] + "," + heures[l] + "," + temperature + "," + date + "\n"
            if heures[l] != 'moyenne':
                SOC.append((int(heures[l]), k - 2, float(values[0][l])))
                V.append((int(heures[l]), k - 2, float(values[1][l])))
                I.append((int(heures[l]), k - 2, float(values[2][l])))
                P.append((int(heures[l]), k - 2, float(values[1][l])))
                if k - 2 == 0:
                    soc_tuple = (heures[l], k - 2, float(values[0][l]), date)
                    dico.append(soc_tuple)

    data_file = open("batteries_data2.csv", 'w')
    data_file.write(string)
    data_file.close()
    return dico, SOC, V, I, P,


def commande(power, mode):
    heures = [8, 9, 10, 11, 12, 13, 14, 15, 16]
    for i in range(len(power)):
        power[i] = float(power[i])

    if mode == 1:
        plt.title('Puissance mesurée en sortie des panneaux')
        plt.xlabel('Heures de la journée [H]')
        plt.ylabel('Puissance en courant continue [W]')
        plt.plot(heures, power)
        plt.show()
    if mode == 2:
        plt.title('Puissance théorique mesurée en sortie des panneaux solaire sans le MPPT')
        plt.xlabel('Heures de la journée [H]')
        plt.ylabel('Puissance en courant continue [W]')
        plt.plot(heures, power)
        plt.show()
    if mode == 3:
        plt.title('Puissance théorique du mppt')
        plt.xlabel('Heures de la journée [H]')
        plt.ylabel('Puissance en courant continue [W]')
        plt.plot(heures, power)
        plt.show()


def test_power(liste_bat, liste_power, power_theo, canvas):
    """
    Vérifie si la puissance fournie par les panneaux correspond bien à la puissance
    théorique demandé par le MPPT en fonction de l'ensoleillement.
    Un problème d'encrassement des panneaux apparait si :
        - La puissance pour un SOC aux environs de 0.9 est inférieur à la puissance
        normalement necessaire au charge controler et que la puissance prédite par le modèle
        théorique des panneaux est supérieur à la puissance demandé par le charge controller.
    Parameters
    ----------
    liste_bat : liste tuple
        reprend les différents tuples coprenant l'heure de la mesure ,
        l'id de la batterie (on monitore ici le battery pack en entier ) , le SOC de la
        batterie ,  la date de la mesure .
    liste_power : liste str
        liste de puissances des panneaux toutes les heures entre 8h et 16h.
    power_theo : liste double
        liste reprenant les puissances prédites par la fonction Calcul_puisssance_moyenne ()
        pour la puissance de sortie des panneaux.
    Returns
    -------
    None.
    """
    count = 0
    power_MPPT = 1305

    for elem in range(len(liste_bat)):
        hours = int(liste_bat[elem][0])
        SOC = liste_bat[elem][2]

        if 0.9 <= SOC <= 0.96 and 8 <= hours <= 16:

            sun_power = power_theo[hours - 8]

            power = float(liste_power[hours - 8])

            if power < power_MPPT < sun_power:
                count = count + 1
                mot = "Problème d'encrassement détecté à " + str(hours) + 'H le ' + str(liste_bat[0][3])
                canvas.create_text(20, 60 + count * 40, text=mot, font="Arial 12 italic", fill="white", anchor="w")


def correctSolarCSV(CSVfile, power_theo, puissance, tension, courant):
    """
    Corrige le CSV partiel de manière à afficher la puissance théorique des panneaux ansi que la puissance réel ,  à chaque heure
    entre 8h et 16h.
    Parameters
    ----------
    CSVfile : str
        nom du fichier partiel .
    power_theo : liste float
       puissance théorique des panneaux entre 8h et 16h .
    Returns
    -------
    None.
    """

    heures = ["8h", "9h", "10h", "11h", "12h", "13h", "14h", "15h", "16h"]
    f = open(CSVfile)
    liste = f.readlines()
    f.close()
    liste[1] = liste[1].replace('D', 'Date')
    liste[1] = liste[1].replace('V', 'Voltage')
    liste[1] = liste[1].replace('I', 'Courant')
    liste[1] = liste[1].replace('P', 'Puissance')

    liste[1] = liste[1][:len(liste[1]) - 1] + ",Heure" + "\n"
    liste[1] = liste[1][:len(liste[1]) - 1] + ",Puissance_theorique" + "\n"
    string = liste[0] + liste[1]

    date = liste[2][:9]

    for l in range(len(heures)):
        s = date + "," + str(courant[l]) + "," + str(tension[l]) + "," + str(puissance[l]) + "," + heures[
            l] + "," + str(power_theo[l]) + "\n"
        string = string + s
    f = open(CSVfile, "w")
    f.write(string)
    f.close()


def extract(text):
    pos = 17
    I = []
    V = []
    P = []
    string = []
    for i in range(3):

        if i == 0:
            string = text[text.find('solarPannels') + pos:]

            start = string.find('[')
            end = string.find(']')
            I = string[start + 1:end].replace(',', ' ')
            I.split()

            pos = end
        if i == 1:
            string = string[pos + 1:]

            start = string.find('[')

            end = string.find(']')

            V = string[start + 1:end].replace(',', ' ')
            V.split()
            pos = end
        if i == 2:
            string = string[pos + 1:]

            start = string.find('[')
            end = string.find(']')
            P = string[start + 1:end].replace(',', ' ')
            P.split()

            pos = end

    return I, V, P


def traitement(text):
    """
    Lance le protocol de traitement des données provenant du BMS
    Parameters
    ----------
    text : str
        JSON correspondant aux données fournies par le BMS .
    Returns
    -------
    None.
    """
    file_pv = "solar_data2.csv"
    file_pv_total = "solar_data_total.csv"
    file_batterie = "batteries_data2.csv"
    file_batterie_total = "batteries_data_total.csv"
    res = extract(text)
    I = res[0].split()
    print("I = ", I)
    V = res[1].split()
    print("V = ", V)
    P = res[2].split()
    print("P = ", P)
    for elem in range(len(I)):
        I[elem] = float(I[elem])
        V[elem] = float(V[elem])
        P[elem] = float(P[elem])

    jsonMessage = readJson(text)

    convertToCsvBatteries(jsonMessage)
    power_theo = calcul_puissance_theorique()

    convertToCsvSolar(jsonMessage)

    res = correctBatteriesCSV(file_batterie)
    dico = res[0]
    SOC = res[1]

    V_bat = res[2]
    I_bat = res[3]
    P_bat = res[4]
    sun_hour = [8, 9, 10, 11, 12, 13, 14, 15, 16]

    root = Tk()
    root.title("Mobateli")  # Window title
    root.config(bg="#FFFFE8")  # Background color
    root.geometry('900x700')

    title = Label(root, text="Mobateli 1", bg="#ACC8E6", width=100, font=('Helvetica bold', 26))
    title.pack()

    canvas = Canvas(root, width=450, height=560, background='black')
    txt = canvas.create_text(20, 60, text=">>", font="Arial 12 italic", fill="white", anchor="w")
    canvas.place(x=400, y=120)

    btn1 = Button(root, text='Affichage SOC',
                  command=lambda: plt_batterie(canvas, txt, ">> Affichage du SOC", SOC, 1), padx=30, pady=20, bg="#ADD8E6")
    btn2 = Button(root, text='Affichage Tension',
                  command=lambda: plt_batterie(canvas, txt, ">> Affichage tension", V_bat, 2), padx=30, pady=20, bg="#ADD8E6")
    btn3 = Button(root, text='Affichage Courant',
                  command=lambda: plt_batterie(canvas, txt, ">> Affichage Courant", I_bat, 3), padx=30, pady=20, bg="#ADD8E6")
    btn4 = Button(root, text='Affichage puissance batterie',
                  command=lambda: plt_batterie(canvas, txt, ">> Affichage Puissance batteries", P_bat, 4), padx=30,
                  pady=20, bg="#ADD8E6")
    btn5 = Button(root, text='Affichage puissance entré du MPPT',
                  command=lambda: power_pannel(canvas, txt, ">> Affichage puissance panneaux", P, sun_hour), padx=30,
                  pady=20, bg="#ADD8E6")
    btn6 = Button(root, text='Affichage puissance théorique sortie de panneaux',
                  command=lambda: power_pannel(canvas, txt, ">> Affichage puissance théorique", power_theo,
                                               sun_hour), padx=30, pady=20, bg="#ADD8E6")

    btn1.place(x=5, y=110)
    btn2.place(x=5, y=210)
    btn3.place(x=5, y=310)
    btn4.place(x=5, y=410)
    btn5.place(x=5, y=510)
    btn6.place(x=5, y=610)

    correctSolarCSV(file_pv, power_theo, P, V, I)
    test_power(dico, P, power_theo, canvas)
    add_csv(file_pv, file_pv_total)
    add_csv(file_batterie, file_batterie_total)
    erased('solar_data.csv')
    erased('batteries_data.csv')
    erased('solar_data2.csv')
    erased('batteries_data2.csv')
    root.mainloop()
    print("Fin du traitement")


def power_pannel(canvas, txt, mot, pt, hour):
    canvas.itemconfigure(txt, text=mot)
    if mot == ">> Affichage puissance théorique":
        plt.title('Puissance théorique des panneaux sans batteries')
        plt.xlabel('Heures de la journée [H]')
        plt.ylabel('Puissance en courant continue [W]')

    elif mot == ">> Affichage puissance panneaux":
        plt.title('Puissance mesurée en sortie des panneaux')
        plt.xlabel('Heures de la journée [H]')
        plt.ylabel('Puissance en courant continue [W]')

    plt.plot(hour, pt)
    plt.show()

def plt_batterie(canvas, txt, mot, pt, mode):
    canvas.itemconfigure(txt, text=mot)
    if mode == 1:
        X = []
        Y = []
        plt.title("Calcul SOC")
        for elem in range(1, len(pt)):
            i = elem - 1
            if pt[elem][1] == pt[i][1]:

                X.append(pt[i][0])
                Y.append(pt[i][2])
            else:
                X.append(pt[i][0])
                Y.append(pt[i][2])

                plt.plot(X, Y, 'o-', label='id' + str(pt[elem][1]))
                X = []
                Y = []
        plt.xlabel('Heures de la mesure [h]')
        plt.ylabel('SOC [%]')
        plt.legend()
        plt.show()
    elif mode == 2:
        X = []
        Y = []
        plt.title("Calcul tension batteries")
        for elem in range(1, len(pt)):
            if pt[elem][1] == pt[elem - 1][1]:

                X.append(pt[elem - 1][0])
                Y.append(pt[elem - 1][2])
            else:
                X.append(pt[elem - 1][0])
                Y.append(pt[elem - 1][2])

                plt.plot(X, Y, 'o-', label='id ' + str(pt[elem][1]))
                X = []
                Y = []
        plt.xlabel('Heures de la mesure [h]')
        plt.ylabel('Voltage[V]')
        plt.legend()
        plt.show()

    elif mode == 3:
        X = []
        Y = []
        plt.title("Mesure des courants ")
        for elem in range(1, len(pt)):
            if pt[elem][1] == pt[elem - 1][1]:
                X.append(pt[elem - 1][0])
                Y.append(pt[elem - 1][2])
            else:
                X.append(pt[elem - 1][0])
                Y.append(pt[elem - 1][2])

                plt.plot(X, Y, 'o-', label='id ' + str(pt[elem][1]))
                X = []
                Y = []
        plt.xlabel('Heures de la mesure [h]')
        plt.ylabel('Courant [A]')
        plt.legend()
        plt.show()
    elif mode == 4:
        X = []
        Y = []
        plt.title("Puissance en entrée du MPPT")
        for elem in range(1, len(pt)):
            if pt[elem][1] == pt[elem - 1][1]:
                X.append(pt[elem - 1][0])
                Y.append(pt[elem - 1][2])
            else:
                X.append(pt[elem - 1][0])
                Y.append(pt[elem - 1][2])

                plt.plot(X, Y, 'o-', label='id ' + str(pt[elem][1]))
                X = []
                Y = []
        plt.xlabel('Heures de la mesure [h]')
        plt.ylabel('Puissance [W]')
        plt.legend()
        plt.show()


##### Communication ###################################################################################

broker_address = "mqtt.thingstream.io"
username = "K7T841RL7HZ3P98IBS6J"
clientid = "device:77d5e918-673f-4639-afa6-c08082db6802"  # Change pour chaque thing.
password = "G5l/8382odCJ+u0DyE/agXzLzuhD14Hlcz3kEVQG"  # Change pour chaque thing.


def on_connect(client, userdata, flags, rc):  # Réponse à une connection
    print("Connected with result code " + str(
        rc) + "  (0: Connection successful / 1: Connection refused - incorrect protocol version / 2: Connection refused - invalid client identifier / 3: Connection refused - server unavailable / 4: Connection refused - bad username or password / 5: Connection refused)")  # rc correspond au résultat de connexion
    if rc == 0:
        client.subscribe(topic="Cameskin", qos=1)  # S'abonne à un topic avec une qos


def on_message(client, userdata, msg):  # Réponse à un message reçu
    print(msg.topic + ' ' + str(msg.payload.decode("utf-8")))
    ms = str(msg.payload.decode("utf-8"))
    traitement(ms)


# Create client object with a persistent connection (!clean session), in this mode the broker will store subscription information, and undelivered messages for the client.
client = mqtt.Client(client_id=clientid, clean_session=False)

# Callbacks are functions that are called in response to an event, here we attach function to callback
client.on_connect = on_connect  # Response to a connection event
client.on_message = on_message  # Response to a message received event

client.username_pw_set(username, password)

# Connection to the brooker
# port : the network port of the server host to connect to. Defaults to 1883.
# keepalive : maximum period in seconds allowed between communications with the broker. If no other messages are being exchanged, this controls the rate at which the client will send ping messages to the broker
client.connect(host=broker_address, port=1883, keepalive=600)
client.loop_forever()

#traitement("{'batteries':[{'id':0,'S':[0.0,1.0,1.0,1.0,1.0,1.0],'V':[0.0,12.9,12.9,12.9,12.9,12.9],'I':[0.0,0.0,0.0,0.0,0.0,0.0],'P':[0.0,0.0,0.0,0.0,0.0,0.0],'H':[0,0,0,0,0],'T':27.9,'D':'25-7-2022'},{'id':1,'S':[0.0,1.0,1.0,1.0,1.0,1.0],'V':[0.0,12.9,12.9,12.9,12.9,12.9],'I':[0.0,0.0,0.0,0.0,0.0,0.0],'P':[0.0,0.0,0.0,0.0,0.0,0.0],'T':30.7},{'id':2,'S':[0.0,1.0,1.0,1.0,1.0,1.0],'V':[0.0,12.9,12.9,12.9,12.9,12.8],'I':[0.0,0.0,0.0,0.0,0.0,0.0],'P':[0.0,0.0,0.0,0.0,0.0,0.0],'T':30.1},{'id':3,'S':[0.0,1.0,1.0,1.0,1.0,1.0],'V':[0.0,12.9,12.9,12.9,12.9,12.9],'I':[0.0,0.0,0.0,0.0,0.0,0.0],'P':[0.0,0.0,0.0,0.0,0.0,0.0],'T':31.6},{'id':4,'S':[0.0,1.0,1.0,1.0,1.0,1.0],'V':[0.0,12.9,12.9,12.9,12.9,12.9],'I':[0.0,0.0,0.0,0.0,0.0,0.0],'P':[0.0,0.0,0.0,0.0,0.0,0.0],'T':28.7},{'id':5,'S':[0.0,1.0,1.0,1.0,1.0,1.0],'V':[0.0,12.9,12.9,12.9,12.9,12.9],'I':[0.0,0.0,0.0,0.0,0.0,0.0],'P':[0.0,0.0,0.0,0.0,0.0,0.0],'T':29.8}],'solarPannels':[{'D':'25-7-2022','I':[0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0],'V':[0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0],'P':[0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0]}]}")
