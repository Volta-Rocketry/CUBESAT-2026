# Anim
def Rz(psi):
    return np.array([
        [np.cos(psi), -np.sin(psi), 0],
        [np.sin(psi),  np.cos(psi), 0],
        [0, 0, 1]
    ])

def Ry(theta):
    return np.array([
        [np.cos(theta), 0, np.sin(theta)],
        [0, 1, 0],
        [-np.sin(theta), 0, np.cos(theta)]
    ])

import matplotlib.pyplot as plt
from matplotlib.widgets import Slider, Button
from scipy.spatial.transform import Rotation as Rot

# --- Datos en radianes ---
roll_rad = np.deg2rad(roll_angle_data.values)
aoa_rad = np.deg2rad(angle_of_attack_data.values)

# --- Setup ---
fig = plt.figure(figsize=(8,8))
ax = fig.add_subplot(projection='3d')
ax.set_xlim([-1,1])
ax.set_ylim([-1,1])
ax.set_zlim([-1,1])
ax.set_box_aspect([1,1,1])

# Marco fijo
ax.quiver(0,0,0,1,0,0,color='r')
ax.quiver(0,0,0,0,1,0,color='g')
ax.quiver(0,0,0,0,0,1,color='b')

# Marco móvil inicial
qx = ax.quiver(0,0,0,1,0,0,color='r')
qy = ax.quiver(0,0,0,0,1,0,color='g')
qz = ax.quiver(0,0,0,0,0,1,color='b')

current_index = 0
playing = False
speed = 1

# --- Actualización ---
def update_frame(i):
    global qx, qy, qz
    
    R_total = Rot.from_euler('zy', [roll_rad[i], aoa_rad[i]])
    mat = R_total.as_matrix()
    
    x_axis = mat @ np.array([1,0,0])
    y_axis = mat @ np.array([0,1,0])
    z_axis = mat @ np.array([0,0,1])
    
    qx.remove(); qy.remove(); qz.remove()
    qx = ax.quiver(0,0,0,*x_axis,color='r')
    qy = ax.quiver(0,0,0,*y_axis,color='g')
    qz = ax.quiver(0,0,0,*z_axis,color='b')
    
    fig.canvas.draw_idle()

# --- Slider ---
ax_slider = plt.axes([0.2, 0.02, 0.6, 0.03])
slider = Slider(ax_slider, 'Time Index', 0, len(roll_rad)-1, valinit=0, valstep=1)

def slider_update(val):
    global current_index
    current_index = int(slider.val)
    update_frame(current_index)

slider.on_changed(slider_update)

# --- Play/Pause ---
ax_button = plt.axes([0.02, 0.02, 0.1, 0.04])
button = Button(ax_button, 'Play/Pause')

def toggle(event):
    global playing
    playing = not playing

button.on_clicked(toggle)

# --- Timer Loop ---
def timer_callback():
    global current_index
    
    if playing:
        current_index += speed
        if current_index >= len(roll_rad):
            current_index = 0
        
        slider.set_val(current_index)

timer = fig.canvas.new_timer(interval=30)
timer.add_callback(timer_callback)
timer.start()

plt.show()