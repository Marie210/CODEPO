# -*- coding: utf-8 -*-
"""
Created on Mon Jul 18 16:25:10 2022

@author: tehoe
"""
import json
import os 
import csv
from DC_conv import Calcul_puissance_moyenne


def readJson(jsonMessage) :

    # remplace les single quotes par des double quotes
    jsonMessage = jsonMessage.replace('\'', '\"')
    jsonMessage = jsonMessage.replace('id', 'batterie')
    # parse le message JSON
    return json.loads(jsonMessage)

def convertToCsvBatteries(jsonLoad):
#Convertie la partie des JSON relative aux batteries dans  un format CSV
# input :  string json 
# output : rien mais création d'un fichier CSV
    data = jsonLoad #JSON
    batteries_data = data['batteries']# Choisis les element relatif aux batteries
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
    #On ferme les fichiers 
    in_file.close()
    out_file.close()
    
    
def write_average_power(csv_name):
# Ajoute la puissance théorique produite par les panneaux solaire 
    average_power  = Calcul_puissance_moyenne() #Calcul de puissance théorique voir DC_conv.py
    data= []
    lignes = [] #Liste des lignes 
    with open(csv_name, 'r') as f:
        for ligne in f : 
          data.append(ligne)
          if (ligne != 'sep=,\n'):
              mots=''
              for i in range(len(ligne)):
                  if(ligne[i] !="\n"):
                      mots+= ligne[i]
              lignes.append(mots)        
    print(data)
   
    for dt in range(len(lignes)):
        if (dt == 0):
            lignes[dt] +=','
            lignes[dt]+= 'Theoretical_power'
            lignes[dt]+= '\n'
        if (dt == 1):
            lignes[dt] +=','
            lignes[dt] += str(average_power)
            lignes[dt] +='\n'
    print (lignes)
    f.close()
    fichier = open(csv_name,'w')
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
    csv_name  = 'solar_data2.csv' 
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
    if(os.path.exists(file) and os.path.isfile(file)): 
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
        
        
def add_csv(old,new):
    if (checkFileExistance(new)):
        lignes_total =[]
        lignes_add =[]
        with open(new,'r') as  f: 
        
            for elem in f:
                lignes_total.append(elem)
                
        with open(old,'r') as f2:
            for i in f2:
            
                lignes_add.append(i)
    
        for  j in range(len(lignes_total)):
            if (j>1):
                lignes_add.insert(len(lignes_add),lignes_total[j])
                print(j)
    #file = open('sola_data2.csv','r') 
          
        fichier = open(new,'w')
   
        print(lignes_add)
        print(lignes_total)
          
        for element in lignes_add:
            fichier.write(element)
        fichier.close()
    else:
        #file_oldname = os.path.join("C:\Users\tehoe\OneDrive\Documents\ULB_2022\Projet_cooperation_au_developpement\Code_python",old)
        #file_newname_newfile = os.path.join("C:\Users\tehoe\OneDrive\Documents\ULB_2022\Projet_cooperation_au_developpement\Code_python", new)

        os.rename(old,new)
        
        
        
def traitement(text):       
    file_pv = "solar_data2.csv"
    file_pv_total ="solar_data_total.csv"
    file_batterie = "batteries_data2.csv"
    file_batterie_total= "batteries_data_total.csv"
   # f =  open(text_file)
    #text = f.read()
    jsonMessage = readJson(text)
    convertToCsvBatteries(jsonMessage)
    convertToCsvSolar(jsonMessage)



    add_csv(file_pv,file_pv_total)
    add_csv(file_batterie,file_batterie_total)
    erased('solar_data.csv')
    erased('batteries_data.csv')
    erased('solar_data2.csv')
    erased('batteries_data2.csv')