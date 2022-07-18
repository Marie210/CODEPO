import json
import csv


def readJson(jsonMessage) :

    # remplace les single quotes par des double quotes
    jsonMessage = jsonMessage.replace('\'', '\"')
    # parse le message JSON
    return json.loads(jsonMessage)

def convertToCsvBatteries(jsonLoad):
    data = jsonLoad
    batteries_data = data['batteries']
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

    with open('batteries_data.csv', newline='') as in_file:
        with open('batteries_data2.csv', 'w', newline='') as out_file:
            writer = csv.writer(out_file)
            for row in csv.reader(in_file):
                if row:
                    writer.writerow(row)
    in_file.close()
    out_file.close()


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

    with open('solar_data.csv', newline='') as in_file:
        with open('solar_data2.csv', 'w', newline='') as out_file:
            writer = csv.writer(out_file)
            for row in csv.reader(in_file):
                if row:
                    writer.writerow(row)
    in_file.close()
    out_file.close()

f =  open('ex_V2.txt')
text = f.read()
jsonMessage = readJson(text)
convertToCsvBatteries(jsonMessage)
convertToCsvSolar(jsonMessage)