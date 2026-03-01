# ----- FUNCTIONS ------ 

import csv
import pandas as pd

def time_to_cycles(time_needed:float, general_frequency:float):

    '''
        Function: Converts the time needed for data recording to cycles

        Args:
            time_needed (float): The time needed to be recorded
            general_frequency (float): The frequency at which the processor retrieves data

        Raises:
            N/A

        Returns:
            float: Total number of cycles required to cover the specified time.
    '''

    cycles_total=time_needed*general_frequency

    return cycles_total

def write_excel_file(names_files_csv:list):

    '''
        Function: Creates an Excel file with different CSV files

        Args:
            names_files_csv (list): The names of the CSV files to be joined

        Raises:
            N/A

        Returns:
            None: Saves 'Data_Optimization_Analysis.xlsx' to the current directory.
    '''

    list_data = []

    for file in names_files_csv:

        data = pd.read_csv(file)
        data["structure"] = file.replace(".csv", "") 
        list_data.append(data)

    chart = pd.concat(list_data, ignore_index=True)
    chart.to_excel("Data_Optimization_Analysis.xlsx", sheet_name="full_data", index=False)

def write_csv_file(file:str, file_data:dict):

    '''
        Function: Creates a CSV file 

        Args:
            file (str): The name of the file to be created
            file_data (dict): The data to be stored in the file

        Raises:
            N/A

        Returns:
            None: Saves the CSV file to the current directory.
    '''

    with open(file, mode='w', newline='') as csvfile:
        writer = csv.writer(csvfile)
        header = ["sensor", "cycles with new data", "total cycles", "bytes"]
        writer.writerow(header)
        for row in file_data.values():
            writer.writerow(row)


def loops_for_new_data(dict_sensor: dict, general_frequency: float):

    '''
        Function: Appends to the dictionary of sensors the number of loops until getting new data from the sensor

        Args:
            dict_sensor (dict): A dictionary where the sensor data is stored
            general_frequency (float): The frequency at which the processor retrieves data

        Raises:
            N/A

        Returns:
            dict: The updated sensor dictionary with calculated wait cycles.
    '''

    for key in list(dict_sensor.keys()):

        lower_condition=True
        cycles_until_data=1

        while lower_condition==True:
            if  cycles_until_data*(dict_sensor[key][1]/general_frequency)<1:
                cycles_until_data+=1
            else:
                lower_condition=False
                dict_sensor[key][4]=cycles_until_data    

    return dict_sensor

def calculate_data_per_cycles(cycles:float, dict_sensor:dict, struct_id:str): 

    '''
        Function: Calculate the weight in bytes of the sensor's data packages in a number of cycles and saves it in a csv file

        Args:
            cycles (float): Number of cycles you want to simulate 
            dict_sensor (dict): A dictionary where the sensor data is stored
            struct_id: A tag to name the csv file

        Raises:
            N/A

        Returns:
            tuple: (total_bytes, file_name) containing the weight and the filename.
    '''
    
    dict_csv={}
    total_bytes=0

    for key in list(dict_sensor.keys()):
        print("\n")
        new_data=cycles//dict_sensor[key][4]

        if new_data==cycles:
            print("Sensor:", dict_sensor[key][0])
            print("New data in every cycle")
            calc_bytes=cycles*dict_sensor[key][3]
            print("Bytes in", cycles,"cycles:",calc_bytes)
            dict_csv[key]=[dict_sensor[key][0],new_data,cycles,calc_bytes]
        else:
            print("Sensor:", dict_sensor[key][0])
            print("New data in", new_data, "cycles of", cycles)
            calc_bytes=new_data*dict_sensor[key][3]
            print("Bytes in", cycles,"cycles:",calc_bytes)
            dict_csv[key]=[dict_sensor[key][0],new_data,cycles,calc_bytes]

        total_bytes+=calc_bytes
        print("Total data weight:", total_bytes)

    file_name = str(cycles)+struct_id+".csv"
    write_csv_file(file_name,dict_csv)

    return total_bytes, file_name

# ----- GENERAL SETUP -----

general_frequency=100 
list_cycles=[]

list_cycles.append(time_to_cycles(250,general_frequency))

difference_weight=[]
percentage=[]
less_efficient_structure=[]
more_efficient_structure=[]
list_file_names=[]

# ----- DEFINING SENSORS -----

# Sensors Names
name_sensor_A = "GPS"
name_sensor_B = "IMU 9DOF Fused"
name_sensor_C = "IMU 9DOF Raw"
name_sensor_D = "Altimeter"

# Sensors Frequency / Hz
freq_sensor_A = 10
freq_sensor_B = 100
freq_sensor_C = 1000
freq_sensor_D = 110

# Quantity Sensors / Unit
quant_sensor_A = 1
quant_sensor_B = 1
quant_sensor_C = 1
quant_sensor_D = 1

# Data Weight / Bytes
weight_sensor_A = 30
weight_sensor_B = 28
weight_sensor_C = 18
weight_sensor_D = 14

# Cycles Until Data / Unit
cycles_until_data_A = 1
cycles_until_data_B = 1
cycles_until_data_C = 1
cycles_until_data_D = 1

# Dictionary of Sensors

dictionary_sensors={
        "A":[name_sensor_A,freq_sensor_A,quant_sensor_A,weight_sensor_A,cycles_until_data_A], 
        "B":[name_sensor_B,freq_sensor_B,quant_sensor_B,weight_sensor_B,cycles_until_data_B], 
        "C":[name_sensor_C,freq_sensor_C,quant_sensor_C,weight_sensor_C,cycles_until_data_C], 
        "D":[name_sensor_D,freq_sensor_D,quant_sensor_D,weight_sensor_D,cycles_until_data_D],
        }

# ----- CODE -----

for cycle_counter in list_cycles:

    print("\n-----------",cycle_counter,"CYCLES -----------\n")

    for key in dictionary_sensors.keys():
        dictionary_sensors[key][4]=1

    # Repeated data structure
    print("\n----------- REPEATED DATA STRUCTURE -----------")
    total_bytes_repeated, file_name=calculate_data_per_cycles(cycle_counter, dictionary_sensors,"R")
    bitmask=2
    timestamp=4
    final_weight_R=(bitmask+timestamp)*cycle_counter+total_bytes_repeated
    list_file_names.append(file_name)
    print("\n----------- PACKAGE WEIGHT:",final_weight_R,"BYTES -----------\n")

    # ID data structure
    print("\n----------- ID DATA STRUCTURE -----------")
    dictionary_sensors=loops_for_new_data(dictionary_sensors, general_frequency)
    bitmask=0
    timestamp=4
    ID=1
    for key in list(dictionary_sensors.keys()):
        dictionary_sensors[key][3]+=(timestamp+ID)
    total_bytes_ID, name_file=calculate_data_per_cycles(cycle_counter, dictionary_sensors, "ID")
    final_weight_ID=total_bytes_ID
    list_file_names.append(name_file)
    print("\n----------- PACKAGE WEIGHT:",final_weight_ID,"BYTES -----------\n")

    # Conclusion
    print("\n----------- CONCLUSION -----------\n")

    print("cycles:", cycle_counter)

    if final_weight_ID<final_weight_R:
        print("The ID structure is lighter than the Repeated structure")
        print(final_weight_R-final_weight_ID, "bytes less")
        difference_weight.append(final_weight_R-final_weight_ID)
        percentage.append(str(final_weight_ID*100/final_weight_R)+"%")
        more_efficient_structure.append("ID")
        less_efficient_structure.append("Repeated")

    elif final_weight_R<final_weight_ID:
        print("The Repeated structure is lighter than the ID structure")
        print(final_weight_ID-final_weight_R, "bytes less")
        more_efficient_structure.append("Repeated")
        less_efficient_structure.append("ID")

    else:
        print("Both structures are equal")
        more_efficient_structure.append("both")
        less_efficient_structure.append("both")
    print("\n")

write_excel_file(list_file_names)
print("----------- COMPARISON -----------")
for counter in range(len(list_cycles)):
    print("In", list_cycles[counter],"cycles the difference was",difference_weight[counter],"bytes, the",more_efficient_structure[counter],"structure represents", percentage[counter], "of the",less_efficient_structure[counter],"structure's weight")
