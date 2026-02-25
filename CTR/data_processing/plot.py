import numpy as np
import pandas as pd
import matplotlib.pyplot as plt


# ----
ROUTE = 'CTR/data_files_irec_2025/'

blueraven_file = "BlueRavenProcessedData.csv"
srad_file = "SRADProcessedData.csv"

df_blueraven = pd.read_csv(ROUTE + blueraven_file)
df_srad = pd.read_csv(ROUTE + srad_file)

times = df_blueraven['Time'].values
lats = np.deg2rad(df_srad['GPS_Lat'].values)
lons = np.deg2rad(df_srad['GPS_Lon'].values)
altitudes = df_blueraven['Altitude'].values

print(times)
print(lats)
print(lons)


def plot_data(label, activateShow=False):
    plt.figure(figsize=(10, 6))
    plt.plot(df_blueraven['Time'], df_blueraven[label], label=f"BlueRaven {label}")
    plt.plot(df_srad['Time'], df_srad[label], label=f"SRAD {label}")
    plt.xlabel('Time (s)')
    plt.ylabel(label)
    plt.title(f"{label} vs Time")
    plt.legend()
    plt.grid()
    if activateShow:
        plt.show()

""" plot_data('Altitude')
plot_data('AccelX')
plot_data('AccelY')
plot_data('AccelZ')
plot_data('GyroX')
plot_data('GyroY')
plot_data('GyroZ', activateShow=False) """

# ----- General Data -----
R = 6378137.0  # Earth radius in meters

# ----- Process -----
# Absolute pos in ENU
phi_0 = lats[0]
lambda_0 = lons[0]

east_m = R * np.cos(phi_0) * (lons - lambda_0)
north_m = R * (lats - phi_0)
up_m = altitudes

# Absolute vel and acc in ENU
vel_east = np.gradient(east_m, times)
vel_north = np.gradient(north_m, times)
vel_up = np.gradient(up_m, times)

acc_east = np.gradient(vel_east, times)
acc_north = np.gradient(vel_north, times)
acc_up = np.gradient(vel_up, times)


print("East Position (m):", east_m)
print("North Position (m):", north_m)


plt.figure(figsize=(10, 6))
plt.plot(times, north_m, label='East Position (m)', marker='o')
plt.legend()
plt.xlabel('North Position (km)')
plt.ylabel('East Position (km)')
plt.title('Position vs Time')
plt.grid()
plt.show()


""" 
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')
ax.plot(north_m/1e3, east_m/1e3, altitudes/1e3, label='Trajectory')
ax.set_xlabel('North Position (km)')
ax.set_ylabel('East Position (km)')
ax.set_zlabel('Up Position (km)')
ax.set_title('3D Trajectory')
plt.show()
 """

""" 
plt.figure(figsize=(10, 6))
plt.plot(times, acc_up, label='Up Acceleration (m/s²)')
plt.plot(df_blueraven['Time'], df_blueraven['AccelZ'], label='BlueRaven AccelZ (m/s²)')
plt.xlabel('Time (s)')
plt.ylabel('Acceleration (m/s²)')
plt.title('Up Acceleration vs Time')
plt.legend()
plt.grid()
plt.show() """