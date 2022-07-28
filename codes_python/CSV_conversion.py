import json
import os
import csv
from DC_conv import calcul_puissance_theorique
from tkinter import * 
import numpy as np
import matplotlib.pyplot as plt


''' Les fonctions ci-dessous permettent de traiter les données provenant du BMS 
L'envoi  des données se fait sous un format JSON qui est ensuite convertie en un format 
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




def convertToCsvSolar(jsonLoad,power):
    '''
    Fonction traduisant les données contenue dans le format json en un fichier CSV 
    

    Parameters
    ----------
    jsonLoad : json text 
        fichier json obtenue à partir du text json.
    power : liste de float 
        liste des puissances théoriques prédite par la fonction calcul_puissance_theorique( ).

    Returns
    -------
    None.

    '''
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
    '''
    Fonction permettant la déstruction des fichiers CSV partiel 

    Parameters
    ----------
    file_name : string
        nom du fichier à effacer .

    Returns
    -------
    None.

    '''
    file = file_name
    if (os.path.exists(file) and os.path.isfile(file)):
        os.remove(file)
        
    
        


def checkFileExistance(filePath):
    '''
    Vérifie l'existance d'un fichier 

    Parameters
    ----------
    filePath : string
        nom du fichier à tester .

    Returns
    -------
    bool
        valeur de retour de la fonction 'true' si le fichier existe 'false' sinon .

    '''
    try:
        with open(filePath, 'r') as f:
            return True
    except FileNotFoundError as e:
        return False
    except IOError as e:
        return False


def add_csv(old, new):
    '''
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

    '''
    
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
    '''
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

    '''
    nb_mesures = 6
    ext = "["
    ext2 = "]"
    f = open(CSVfile)
    liste = f.readlines()
    f.close()
    
    n = len(liste[2])
    date = liste[2][n - 10: n-1]
   

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
 
    dico =  []
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
        if k==2:
            lis = liste[k][liste[k].find(ext, i) + 1:liste[k].find(ext2, i)]
            for elem in lis.split():
                heures.append(elem)
            i = liste[k].find(ext2, i) + 1
            heures.append("moyenne")
        temperature = liste[k][liste[k].find(",", i) + 1:liste[k].find(",", i+2)]
        i = liste[k].find(",", i+2)
        
            
        for l in range(nb_mesures):
            string = string + str(k - 1) + "," + values[0][l] + "," + values[1][l] + "," + values[2][l] + ","+values[3][l] + ","+ heures[l] + ","+ temperature + ","+ date  +"\n"
            if (k-1 == 1 and heures[l] != 'moyenne'):
                soc_tuple = (heures[l] ,k-1,float(values[0][l]),date)
           
                dico.append(soc_tuple)
            
    data_file = open("batteries_data2.csv", 'w')
    data_file.write(string)
    data_file.close()
    
    return dico
def commande( power, mode  ):
    print ("ok\n")
    X= [8,9,10,11,12,13,14,15,16]
    if (mode == 1):
        plt.title('Puissance théorique en sortie des panneaux solaire')
        plt.xlabel('Heures de la journée [H]')
        plt.ylabel('Puissance en courant continue  [W]')
        plt.plot(X, power)   
        plt.show()
    elif (mode == 2): 
        plt.title('Puissance théorique mesurée en sortie des panneaux solaire sans le MPPT')
        plt.xlabel('Heures de la journée [H]')
        plt.ylabel('Puissance en courant continue  [W]')
        plt.plot(X, power)   
        plt.show()
    elif (mode == 3):
        plt.title('Puissance théorique du mppt  ')
        plt.xlabel('Heures de la journée [H]')
        plt.ylabel('Puissance en courant continue  [W]')
        plt.plot(X, power)   
        plt.show()
       
       
    
    
def test_power(liste_bat,liste_power,power_theo):
    '''
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

    '''
    power_MPPT = 1305
    
    for elem in range(len(liste_bat)):
        
        hours =  int (liste_bat[elem][0])
        
        sun_power =0
        SOC =  liste_bat[elem][2]
       
        if (SOC >= 0.9 and SOC<=0.96 and hours >=8 and hours <=16):
            
            sun_power =  power_theo[hours-8]
           
           
            power = float(liste_power[hours - 8])
            
            if (power < power_MPPT and sun_power > power_MPPT ):
                    mot = "Problème d'encrassement détecté à " + str(hours) +'H le '+ str(liste_bat[0][3])
                    print(mot)
                    fenetre = Tk()
                    # canvas
                    canvas = Canvas(fenetre, width=600, height=300, background='yellow')
                    
                    
                    txt = canvas.create_text(300, 60, text=mot, font="Arial 16 italic", fill="red")
                    canvas.pack()
                    button1 = Button(fenetre, text ='Puissance théorique', command= lambda: commande(liste_power,1))
                    button2 = Button(fenetre, text ='Puissance mesurée', command= lambda: commande(power_theo,2) )
                    
                    button1.pack(side=LEFT, padx=5, pady=5)
                    button2.pack(side=RIGHT, padx=5, pady=5)
                    
                        
                    #Button(fenetre, text ='graphique puissance mesurée ')
                    
                    fenetre.mainloop()

def correctSolarCSV(CSVfile,power):
    '''
    Corrige le CSV partiel de manière à afficher la puissance théorique des panneaux ansi que la puissance réel ,  à chaque heure
    entre 8h et 16h.  

    Parameters
    ----------
    CSVfile : str
        nom du fichier partiel .
    power : liste float 
       puissance théorique des panneaux entre 8h et 16h .

    Returns
    -------
    None.

    '''
    
    heures = ["8h", "9h","10h","11h","12h","13h","14h","15h","16h"]
    f = open(CSVfile)
    liste = f.readlines()
    f.close()
    liste[1] = liste[1].replace('D', 'Date')
    liste[1] = liste[1].replace('V', 'Voltage')
    liste[1] = liste[1].replace('I', 'Courant')
    liste[1] = liste[1].replace('P', 'Puissance')
    
    liste[1]= liste[1][:len(liste[1])-1]+",Heure"+"\n"
    liste[1]= liste[1][:len(liste[1])-1]+",Puissance_theorique"+"\n"
    string = liste[0] + liste[1]
    
    j = liste[2].find(',')
    j2 = liste[2].find(',', j+1)
    j3 = liste[2].find(',', j2 + 1)
    date = liste[2][:9]
    i= liste[2][j2+1:j3]
    v= liste[2][j+1:j2]
    var = liste[2].find(']')
    pt= liste[2][var+3:len(liste[2])-1]
    puissance = liste[2][liste[2].find('[') + 1:var].replace(', ', ' ')
    puissance = puissance.split()
    for l in range(len(heures)):
        s = date + "," + i + "," + v +"," + puissance[l] +  ","+heures[l]+ "," + str(power[l]) + "\n"
        string = string+s
    f = open(CSVfile, "w")
    f.write(string)
    f.close()


def traitement (text):
    '''
    Lance le protocol de traitement des données provenant du BMS 

    Parameters
    ----------
    text : str
        JSON correspondant aux données fournies par le BMS .

    Returns
    -------
    None.

    '''
    #text = "{'batteries':[{'id':0, 'S':[0.8, 0.92, 0.71, 0.94, 0.92, 0.74],'V':[0.0, 0.0, 0.0, 0.0, 0.0, 0.0],'I':[0.5, 0.3, 0.0, 0.0, 0.3, -1.8], 'P':[260.6, 400.1, 340.4, 367.3, 234.8, 321.4], 'H':[1, 3, 6, 9, 12], 'T':5.0, 'D': '25-7-2022'},{'id':1, 'S':[0.6, 0.4, 0.8, 0.1, 0.92, 0.5],'V':[0.0, 0.0, 0.0, 0.0, 0.0, 0.0],'I':[0.5, 0.3, 0.0, 0.0, 0.3, -1.8],'P':[260.6, 400.1, 340.4, 367.3, 234.8, 321.4], 'T':3.0},{'id':2, 'S':[1.0, 1.0, 1.0, 1.0, 1.0, 5.0],'V':[0.0, 0.0, 0.0, 0.0, 0.0, 0.0],'I':[0.5, 0.3, 0.0, 0.0, 0.3, -1.8], 'P':[260.6, 400.1, 340.4, 367.3, 234.8, 321.4], 'T':0.0},{'id':3, 'S':[1.0, 1.0, 1.0, 1.0, 1.0, 5.0],'V':[0.0, 0.0, 0.0, 0.0, 0.0, 0.0],'I':[0.5, 0.3, 0.0, 0.0, 0.3, -1.8], 'P':[260.6, 400.1, 340.4, 367.3, 234.8, 321.4], 'T':0.0},{'id':4, 'S':[1.0, 1.0, 1.0, 1.0, 1.0, 5.0],'V':[0.0, 0.0, 0.0, 0.0, 0.0, 0.0],'I':[0.5, 0.3, 0.0, 0.0, 0.3, -1.8], 'P':[260.6, 400.1, 340.4, 367.3, 234.8, 321.4], 'T':0.0},{'id':5, 'S':[0.0, 0.0, 0.0, 0.0, 0.1, 5.0],'V':[0.0, 0.0, 0.0, 0.0, 0.0, 0.0],'I':[0.5, 0.3, 0.0, 0.0, 0.3, -1.8], 'P':[260.6, 400.1, 340.4, 367.3, 234.8, 321.4], 'T':0.0}], 'solarPannels': [{'D': '25-7-2022', 'I': -6.9, 'V': 0.0, 'P': [1.1, 1.1, 1.1, 1.1, 1.1, 5.3, 14, 15, 16]}]}"
    file_pv = "solar_data2.csv"
    file_pv_total = "solar_data_total.csv"
    file_batterie = "batteries_data2.csv"
    file_batterie_total = "batteries_data_total.csv"
    # f =  open(text_file)
    # text = f.read()
    string = text[text.find('solarPannels') +19:]
    liste =string[string.find('[')+1:string.find(']')].replace(', ', ' ')
    liste = liste.split()
    
    jsonMessage = readJson(text)
    
    convertToCsvBatteries(jsonMessage)
    power_theo = calcul_puissance_theorique()
    convertToCsvSolar(jsonMessage,power_theo )
    
    dico = correctBatteriesCSV(file_batterie)
    correctSolarCSV(file_pv,power_theo)
    test_power(dico,liste,power_theo) 
    add_csv(file_pv, file_pv_total)
    add_csv(file_batterie, file_batterie_total)
    erased('solar_data.csv')
    erased('batteries_data.csv')
    erased('solar_data2.csv')
    erased('batteries_data2.csv')



