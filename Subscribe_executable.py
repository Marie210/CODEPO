import paho.mqtt.client as mqtt
import json
import os
import csv
import requests
from pvlib import pvsystem
import pvlib
import numpy as np
from bs4 import BeautifulSoup
import matplotlib.pyplot as plt


hostname = "mqtt.thingstream.io"
clientid = "device:77d5e918-673f-4639-afa6-c08082db6802"
username = "K7T841RL7HZ3P98IBS6J"
password = "G5l/8382odCJ+u0DyE/agXzLzuhD14Hlcz3kEVQG"

def Calcul_puissance_moyenne():
    # Irradiance data
    print("ok\n")
    url = "https://fr.tutiempo.net/radiation-solaire/kinshasa.html"
    page = requests.get(url)
    soup = BeautifulSoup(page.content, 'html.parser')
    Irradiance = []
    Irrad_data = soup.find_all("strong")

    # Température deffinition
    url_temp_old = "https://fr.tutiempo.net/kinshasa.html?donnees=dernieres-24-heures"
    page_temp1 = requests.get(url_temp_old)
    soup_Temp = BeautifulSoup(page_temp1.content, 'html.parser')
    Temp = []
    Temp_data = soup_Temp.find_all("td", class_="t Temp")

    # Traitement des données de Température et d'Irradiance
    count = 0
    for elem_irr in Irrad_data:
        if (count >= 3 and count <= 11):
            Irradiance.append(float(elem_irr.string))
        count += 1

    count_Temp = 0
    for elem_temp in Temp_data:
        if (count_Temp <= 8):
            data = elem_temp.string
            count_data = 1
            data_new = data[count_data - 1]
            while (data[count_data] != '°'):
                data_new += data[count_data]
                count_data += 1
            Temp.append(float(data_new))
        count_Temp += 1

    # Definition of PV module characteristics:
    cec_mod_db = pvsystem.retrieve_sam('CECmod')
    print(cec_mod_db.iloc[2, 8458])
    pdc0 = float(cec_mod_db.iloc[2, 8458])  # STC power
    gamma_pdc = float(cec_mod_db.iloc[21, 8458]) / 100  # The temperature coefficient in units of 1/C
    # gamma_pdc= -0.0045
    # Calcul de la puissance théorique

    Irradiance_a = np.array(Irradiance)
    Temp_a = np.array(Temp)

    dc_power = pvlib.pvsystem.pvwatts_dc(Irradiance_a, Temp_a, pdc0, gamma_pdc, temp_ref=25.0)
    for i in range(len(dc_power)):
        dc_power[i] = dc_power[i] * 4
        # Let's visualize the DC power output as function of the effective irradiance
    Heures = [8, 9, 10, 11, 12, 13, 14, 15, 16]
    plt.scatter(Heures, dc_power, c=Temp, vmin=10, vmax=50, cmap='Reds')
    cbar = plt.colorbar()
    puissance_moyenne = 0
    for j in range(len(dc_power)):
        puissance_moyenne += dc_power[j]
    puissance_moyenne = puissance_moyenne / 9.0
    print("p_m = " + str(puissance_moyenne))

    cbar.set_label('Température air ambiante[$^\circ$C]')
    plt.title('Puissance théorique en sortie des panneaux solaire')
    plt.xlabel('Heures de la journée [H]')
    plt.ylabel('Puissance en courant continue  [W]')
    plt.show()
    return puissance_moyenne


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
    data = jsonLoad  # JSON
    batteries_data = data['batteries']  # Choisis les element relatif aux batteries
    # now we will open a file for writing
    data_file = open('batteries_data.csv', 'w')
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


def write_average_power(csv_name):
    # Ajoute la puissance théorique produite par les panneaux solaire
    average_power = int(Calcul_puissance_moyenne() ) # Calcul de puissance théorique voir DC_conv.py
    data = []
    lignes = []  # Liste des lignes
    with open(csv_name, 'r') as f:
        for ligne in f:
            data.append(ligne)
            if (ligne != 'sep=,\n'):
                mots = ''
                for i in range(len(ligne)):
                    if (ligne[i] != "\n"):
                        mots += ligne[i]
                lignes.append(mots)
    print(data)

    for dt in range(len(lignes)):
        if (dt == 0):
            lignes[dt] += ','
            lignes[dt] += 'Theoretical_power'
            lignes[dt] += '\n'
        if (dt == 1):
            lignes[dt] += ','
            lignes[dt] += str(average_power)
            lignes[dt] += '\n'
    print(lignes)
    f.close()
    fichier = open(csv_name, 'w')
    fichier.write("sep=,\n")

    for element in lignes:
        fichier.write(element)
    fichier.close()


def convertToCsvSolar(jsonLoad):
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
    write_average_power(csv_name)


def erased(file_name):
    file = file_name
    if (os.path.exists(file) and os.path.isfile(file)):
        os.remove(file)
        print("file deleted")
    else:
        print("file not found")


def checkFileExistance(filePath):
    try:
        with open(filePath, 'r') as f:
            return True
    except FileNotFoundError as e:
        return False
    except IOError as e:
        return False


def add_csv(old, new):
    if (checkFileExistance(new)):
        lignes_total = []
        lignes_add = []
        with open(new, 'r') as  f:

            for elem in f:
                lignes_total.append(elem)

        with open(old, 'r') as f2:
            for i in f2:
                lignes_add.append(i)

        for j in range(len(lignes_total)):
            if (j > 1):
                lignes_add.insert(len(lignes_add), lignes_total[j])
                print(j)
        # file = open('sola_data2.csv','r')

        fichier = open(new, 'w')

        print(lignes_add)
        print(lignes_total)

        for element in lignes_add:
            fichier.write(element)
        fichier.close()
    else:
        # file_oldname = os.path.join("C:\Users\tehoe\OneDrive\Documents\ULB_2022\Projet_cooperation_au_developpement\Code_python",old)
        # file_newname_newfile = os.path.join("C:\Users\tehoe\OneDrive\Documents\ULB_2022\Projet_cooperation_au_developpement\Code_python", new)

        os.rename(old, new)


def traitement(text):
    file_pv = "solar_data2.csv"
    file_pv_total = "solar_data_total.csv"
    file_batterie = "batteries_data2.csv"
    file_batterie_total = "batteries_data_total.csv"
    # f =  open(text_file)
    # text = f.read()
    jsonMessage = readJson(text)
    convertToCsvBatteries(jsonMessage)
    convertToCsvSolar(jsonMessage)
    correctBatteriesCSV(file_batterie)

    add_csv(file_pv, file_pv_total)
    add_csv(file_batterie, file_batterie_total)
    erased('solar_data.csv')
    erased('batteries_data.csv')
    erased('solar_data2.csv')
    erased('batteries_data2.csv')



def correctBatteriesCSV(CSVfile):
    nb_mesures = 6
    ext = "["
    ext2 = "]"
    f = open(CSVfile)
    liste = f.readlines()
    f.close()
    ext3 = "-"
    n = len(liste[2])
    date = liste[2][n - 10: n]
    print("date")
    print(date)
    liste[1] = liste[1].replace('S', 'State of charge')
    liste[1] = liste[1].replace('V', 'Voltage')
    liste[1] = liste[1].replace('I', 'Courant')
    liste[1] = liste[1].replace('H', 'Heure')
    liste[1] = liste[1].replace('D', 'Date')
    liste[1] = liste[1].replace('N', 'Batterie ID')
    liste[1] = liste[1].replace('T', 'Temperature')
    liste[1] = liste[1].replace('P', 'Puissance moyenne')
    string = liste[0] + liste[1]
    heures = []
    puissance = 0
    for k in range(2, len(liste)):
        liste[k] = liste[k].replace(', ', ' ')
        i = 3
        nb = k + 1
        values = []
        for j in range(3):
            lis = liste[k][liste[k].find(ext, i) + 1:liste[k].find(ext2, i)]
            values.append([])
            for elem in lis.split():
                values[j].append(elem)
            i = liste[k].find(ext2, i) + 1
        if k==2:
            lis = liste[k][liste[k].find(ext, i) + 1:liste[k].find(ext2, i)]
            for elem in lis.split():
                heures.append(elem)
            i = liste[k].find(ext2, i) + 1
            heures.append("moyenne")
        temperature = liste[k][liste[k].find(",", i) + 1:liste[k].find(",", i+2)]
        i = liste[k].find(",", i+2)
        if k==2:
            puissance = liste[k][liste[k].find(",", i) + 1:liste[k].find(",", i+2)]
            print("puissance")
            print(liste[k].find(",", i)+1)
            print(liste[k].find(",", i+2))
            print(puissance)
        for l in range(nb_mesures):
            string = string + str(k - 1) + "," + values[0][l] + "," + values[1][l] + "," + values[2][l] + ","+ heures[l] + ","+ temperature+ ","+ puissance + ","+ date  +"\n"

    data_file = open("batteries_data.csv", 'w')
    data_file.write(string)
    data_file.close()
    with open('batteries_data.csv', newline='') as in_file:
        with open('batteries_data2.csv', 'w', newline='') as out_file:
            writer = csv.writer(out_file)
            for row in csv.reader(in_file):
                if row:
                    writer.writerow(row)
    # On ferme les fichiers
    in_file.close()
    out_file.close()


def readJson(jsonMessage):
    # remplace les single quotes par des double quotes
    jsonMessage = jsonMessage.replace('\'', '\"')
    # parse le message JSON
    return json.loads(jsonMessage)


def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    if rc == 0:
        # subscribe to all topics
        client.subscribe("Cameskin", 1)


def on_message(client, userdata, msg):
    print(msg.topic + ' ' + str(msg.payload.decode("utf-8")))
    ms = str(msg.payload.decode("utf-8"))
    traitement(ms)
    # message = readJson(str(msg.payload.decode("utf-8")))
    # print(message)


client = mqtt.Client(client_id=clientid, clean_session=False)
client.on_connect = on_connect
client.on_message = on_message
client.username_pw_set(username, password)

# Insecure connection on port 1883 !!!
client.connect(host=hostname, port=1883, keepalive=600)
client.loop_forever()

