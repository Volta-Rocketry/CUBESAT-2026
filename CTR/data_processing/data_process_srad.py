'''
Convention when processing data:
Z -> Up
X -> 
Y -> 

'''

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

from general_funcs import simple_liftoff_detection, interplolate_data

ROUTE = 'CTR/data_files_irec_2025/'
file_name = 'SRAD_FLIGHT_DATA_2025_IREC.csv'

df = pd.read_csv(ROUTE + file_name)

time_data = df['Time'].values
press_data = df['Press_hPa'].values
altitude_data = df['Alt_m'].values
accel_x_data = df['AccelZ'].values
accel_y_data = -df['AccelX'].values
accel_z_data = df['AccelY'].values
gyro_x_data = df['gyroZ'].values
gyro_y_data = -df['gyroX'].values
gyro_z_data = df['gyroY'].values
euler_x_data = df['EulerZ'].values
euler_y_data = -df['EulerX'].values
euler_z_data = df['EulerY'].values
gps_lat_data = df['KF_Likelihood'].values
gps_lon_data = df['KF_LogLikelihood'].values

# Format units
accel_x_data = accel_x_data / 9.81
accel_y_data = accel_y_data / 9.81
accel_z_data = accel_z_data / 9.81

liftoff_time, liftoff_idx = simple_liftoff_detection(time_data, accel_z_data)

if liftoff_time is None:
    raise ValueError("Liftoff not detected in the data. Please check the accel_z_data and the threshold.")

time_data = time_data[liftoff_idx:]
time_data = time_data - time_data[0]
press_data = press_data[liftoff_idx:]
altitude_data = altitude_data[liftoff_idx:]
accel_x_data = accel_x_data[liftoff_idx:]
accel_y_data = accel_y_data[liftoff_idx:]
accel_z_data = accel_z_data[liftoff_idx:]
gyro_x_data = gyro_x_data[liftoff_idx:]
gyro_y_data = gyro_y_data[liftoff_idx:]
gyro_z_data = gyro_z_data[liftoff_idx:]
euler_x_data = euler_x_data[liftoff_idx:]
euler_y_data = euler_y_data[liftoff_idx:]
euler_z_data = euler_z_data[liftoff_idx:]
gps_lat_data = gps_lat_data[liftoff_idx:]
gps_lon_data = gps_lon_data[liftoff_idx:]


interpolation_time = np.linspace(time_data[0], time_data[-1], 10000)

press = interplolate_data(time_data, press_data, interpolation_time)
altitude = interplolate_data(time_data, altitude_data, interpolation_time)
accelX = interplolate_data(time_data, accel_x_data, interpolation_time)
accelY = interplolate_data(time_data, accel_y_data, interpolation_time)
accelZ = interplolate_data(time_data, accel_z_data, interpolation_time)
gyroX = interplolate_data(time_data, gyro_x_data, interpolation_time)
gyroY = interplolate_data(time_data, gyro_y_data, interpolation_time)
gyroZ = interplolate_data(time_data, gyro_z_data, interpolation_time)
eulerX = interplolate_data(time_data, euler_x_data, interpolation_time)
eulerY = interplolate_data(time_data, euler_y_data, interpolation_time)
eulerZ = interplolate_data(time_data, euler_z_data, interpolation_time)
gps_lat = interplolate_data(time_data, gps_lat_data, interpolation_time)
gps_lon = interplolate_data(time_data, gps_lon_data, interpolation_time)




df = pd.DataFrame()

df = df.assign(Time=interpolation_time,
               Pressure=press,
               Altitude=altitude,
               AccelX=accelX,
               AccelY=accelY,
               AccelZ=accelZ,
               GyroX=gyroX,
               GyroY=gyroY,
               GyroZ=gyroZ,
               EulerX=eulerX,
               EulerY=eulerY,
               EulerZ=eulerZ,
               GPS_Lat=gps_lat,
               GPS_Lon=gps_lon)

df.to_csv(ROUTE + 'SRADProcessedData.csv', index=False)
