# -*- coding: utf-8 -*-
"""
Created on Sat Jul 16 13:11:18 2022

@author: tehoe
"""
import requests
from pvlib import pvsystem
import pvlib
import numpy as np

from bs4 import BeautifulSoup
import matplotlib.pyplot as plt


def Calcul_puissance_moyenne():
    # Irradiance data 
    print("ok\n")
    url = "https://fr.tutiempo.net/radiation-solaire/kinshasa.html"
    page = requests.get(url)
    soup = BeautifulSoup(page.content, 'html.parser')
    Irradiance = []
    Irrad_data = soup.find_all("strong")
    
    
    # Température deffinition 
    url_temp_old ="https://fr.tutiempo.net/kinshasa.html?donnees=dernieres-24-heures"
    page_temp1 = requests.get(url_temp_old)
    soup_Temp = BeautifulSoup(page_temp1.content, 'html.parser')
    Temp = []
    Temp_data = soup_Temp.find_all("td",class_= "t Temp")
    
    #Traitement des données de Température et d'Irradiance 
    count =  0
    for elem_irr in  Irrad_data:
        if (count >=3 and count <=11 ):
            Irradiance.append(float(elem_irr.string))
        count+=1
    
    count_Temp =0 
    for elem_temp in  Temp_data:
        if (count_Temp <=8):
            data =  elem_temp.string
            count_data= 1
            data_new = data[count_data - 1]
            while (data[count_data] != '°'):
                data_new += data[count_data]
                count_data +=1
            Temp.append(float(data_new))
        count_Temp+=1
    
    # Definition of PV module characteristics:
    cec_mod_db = pvsystem.retrieve_sam('CECmod')
    print(cec_mod_db.iloc[2,8458])
    pdc0 = float(cec_mod_db.iloc[2,8458]) # STC power
    gamma_pdc = float(cec_mod_db.iloc[21,8458])/100 # The temperature coefficient in units of 1/C
    #gamma_pdc= -0.0045
    # Calcul de la puissance théorique 
    
    Irradiance_a = np.array(Irradiance)
    Temp_a = np.array(Temp)
    
    dc_power = pvlib.pvsystem.pvwatts_dc(Irradiance_a, Temp_a, pdc0, gamma_pdc, temp_ref=25.0)
    for i in range(len(dc_power)):
        dc_power[i] = dc_power[i ]* 4 
    # Let's visualize the DC power output as function of the effective irradiance
    Heures = [8,9,10,11,12,13,14,15,16]
    plt.scatter(Heures, dc_power, c=Temp, vmin=10, vmax=50, cmap='Reds')
    cbar = plt.colorbar()
    puissance_moyenne  =  0 
    for  j  in  range(len(dc_power)):
        puissance_moyenne += dc_power[j]
    puissance_moyenne = puissance_moyenne/9.0
    
    cbar.set_label('Température air ambiante[$^\circ$C]')
    plt.title('Puissance théorique en sortie des panneaux solaire')
    plt.xlabel('Heures de la journée [H]')
    plt.ylabel('Puissance en courant continue  [W]')
    plt.show()
    return puissance_moyenne

