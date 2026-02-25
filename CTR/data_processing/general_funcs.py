from scipy.interpolate import interp1d

def simple_liftoff_detection(time, accelz):
    accelz_threshold = 2 # gs

    for i in range(len(accelz)):    
        if accelz[i] > accelz_threshold:
            return time[i], i
    
    return None, None

def interplolate_data(time, data, new_time):
    interpolation_func = interp1d(time, data, kind='linear', fill_value='extrapolate')
    return interpolation_func(new_time)