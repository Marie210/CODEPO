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


def calcul_puissance_theorique( ):
    '''
    Calcul  la puissance théorique en sortie des panneaux solaires à l'aide de l'irradiance journalière 
    sur la ville de Kinshasa . 
    Le modèle du panneaux solaire se retrouve dans la base de données CECmod (base de donnée de la comission à 
    l'énergie Californienne reprenant énormement de modèle de panneaux photovoltaique différent). 

    Returns
    -------
    dc_power: liste float 
        liste des puissance théoriques fournies par les panneaux solaire entre 8h et 16h.

    '''
    # Irradiance data 
  
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
    
    pdc0 = float(cec_mod_db.iloc[2,8458]) # STC power
    gamma_pdc = float(cec_mod_db.iloc[21,8458])/100 # The temperature coefficient in units of 1/C
    #gamma_pdc= -0.0045
    # Calcul de la puissance théorique 
    
    Irradiance_a = np.array(Irradiance)
    Temp_a = np.array(Temp)
    
    dc_power = pvlib.pvsystem.pvwatts_dc(Irradiance_a, Temp_a, pdc0, gamma_pdc, temp_ref=25.0)
    
    for i in range(len(dc_power)):
        dc_power[i] = dc_power[i ]* 20 
    
    # Let's visualize the DC power output as function of the effective irradiance
  
    return dc_power.tolist()
    
