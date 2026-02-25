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
file_name_barometer = 'BlueRav Volta LR_06-12-2025_16_30_38.csv'
file_name_IMU = 'BlueRav Volta HR_06-12-2025_16_30_38_2025_IREC.csv'

df_barometer = pd.read_csv(ROUTE + file_name_barometer)
df_IMU = pd.read_csv(ROUTE + file_name_IMU)

time_data_barometer = df_barometer['Flight_Time_(s)'].values
time_data_IMU = df_IMU['Flight_Time_(s)'].values
press_data = df_barometer['Baro_Press_(atm)'].values
altitude_data = df_barometer['Baro_Altitude_AGL_(feet)'].values
tilt_angle_data = df_barometer['Tilt_Angle_(deg)'].values
roll_angle_data = df_barometer['Roll_Angle_(deg)'].values
accel_x_data = df_IMU['Accel_Z'].values
accel_y_data = df_IMU['Accel_Y'].values
accel_z_data = df_IMU['Accel_X'].values
gyro_x_data = df_IMU['Gyro_Z'].values
gyro_y_data = df_IMU['Gyro_Y'].values
gyro_z_data = df_IMU['Gyro_X'].values
quat1_data = df_IMU['Quat_1'].values
quat2_data = df_IMU['Quat_2'].values
quat3_data = df_IMU['Quat_3'].values
quat4_data = df_IMU['Quat_4'].values

liftoff_time, liftoff_idx = simple_liftoff_detection(time_data_IMU, accel_z_data)

if liftoff_time is None:
    raise ValueError("Liftoff not detected in the data. Please check the accel_z_data and the threshold.")

# Search exact time and idx in barometer data
tol = 0.01
liftoff_time_barometer_idx_tupple = np.where(np.isclose(time_data_barometer, liftoff_time, atol=tol))[0]
liftoff_time_barometer_idx = liftoff_time_barometer_idx_tupple[0]


time_data_barometer = time_data_barometer[liftoff_time_barometer_idx:]
time_data_barometer = time_data_barometer - time_data_barometer[0]
time_data_IMU = time_data_IMU[liftoff_idx:]
time_data_IMU = time_data_IMU - time_data_IMU[0]
    
press_data = press_data[liftoff_time_barometer_idx:]
altitude_data = altitude_data[liftoff_time_barometer_idx:]
tilt_angle_data = tilt_angle_data[liftoff_time_barometer_idx:]
roll_angle_data = roll_angle_data[liftoff_time_barometer_idx:]
accel_x_data = accel_x_data[liftoff_idx:]
accel_y_data = accel_y_data[liftoff_idx:]
accel_z_data = accel_z_data[liftoff_idx:]
gyro_x_data = gyro_x_data[liftoff_idx:]
gyro_y_data = gyro_y_data[liftoff_idx:]
gyro_z_data = gyro_z_data[liftoff_idx:]
quat1_data = quat1_data[liftoff_idx:]
quat2_data = quat2_data[liftoff_idx:]
quat3_data = quat3_data[liftoff_idx:]
quat4_data = quat4_data[liftoff_idx:]

interpolation_time = np.linspace(time_data_IMU[0], time_data_IMU[-1], 10000)

press = interplolate_data(time_data_barometer, press_data, interpolation_time)
altitude = interplolate_data(time_data_barometer, altitude_data, interpolation_time)
tilt_angle = interplolate_data(time_data_barometer, tilt_angle_data, interpolation_time)
roll_angle = interplolate_data(time_data_barometer, roll_angle_data, interpolation_time)
accelX = interplolate_data(time_data_IMU, accel_x_data, interpolation_time)
accelY = interplolate_data(time_data_IMU, accel_y_data, interpolation_time)
accelZ = interplolate_data(time_data_IMU, accel_z_data, interpolation_time)
gyroX = interplolate_data(time_data_IMU, gyro_x_data, interpolation_time)
gyroY = interplolate_data(time_data_IMU, gyro_y_data, interpolation_time)
gyroZ = interplolate_data(time_data_IMU, gyro_z_data, interpolation_time)
quat1 = interplolate_data(time_data_IMU, quat1_data, interpolation_time)
quat2 = interplolate_data(time_data_IMU, quat2_data, interpolation_time)
quat3 = interplolate_data(time_data_IMU, quat3_data, interpolation_time)
quat4 = interplolate_data(time_data_IMU, quat4_data, interpolation_time)


# Format units
altitude = altitude * 0.3048 # feet to meters


df = pd.DataFrame()

df = df.assign(Time=interpolation_time,
               Pressure=press,
               Altitude=altitude,
               TiltAngle=tilt_angle,
               RollAngle=roll_angle,
               AccelX=accelX,
               AccelY=accelY,
               AccelZ=accelZ,
               GyroX=gyroX,
               GyroY=gyroY,
               GyroZ=gyroZ,
               Quat1=quat1,
               Quat2=quat2,
               Quat3=quat3,
               Quat4=quat4)

df.to_csv(ROUTE + 'BlueRavenProcessedData.csv', index=False)