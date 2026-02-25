import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

files_names = [
    '162_Universidad de Antioquia_cubesat_DATA.csv',
    'BlueRav Volta HR_06-12-2025_16_30_38_2025_IREC.csv',
    'BlueRav Volta LR_06-12-2025_16_30_38.csv',
    'BlueRav Volta_summary_06-12-2025_16_30_38_.csv',
    'RRC3_FLIGHT_DATA_2025_IREC.csv',
    'SRAD_FLIGHT_DATA_2025_IREC.csv'
]

ROUTE = 'CTR/data_files_irec_2025/'

def print_file_info(file_name):
    print("=" * 50)
    try:
        df = pd.read_csv(ROUTE + file_name)
        print(f"File: {file_name}")
        print(f"Shape: {df.shape}")
        print(f"Columns: {df.columns.tolist()}")        
    except Exception as e:
        print(f"Error reading {file_name}: {e}")    
    

for file_name in files_names:
    print_file_info(file_name)