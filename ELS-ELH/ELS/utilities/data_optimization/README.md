## Data Strategy Analysis 
This utility simulates data throughput for a 250s mission profile at 100Hz, comparing Bitmask vs. ID-tag structures to optimize Flash saving efficiency.
## Validation and Reproducibility Steps
1. **Mission Profile:** The script uses a 250s mission profile at 100Hz.
2. **Input Verification:** The sensors used for the analysis were taken from the ones mentioned for the main OBC (The frequency was selected based on common ranges for these types of sensors, as there are no commercial references yet).
3. **Execution:** Run `python optimization_model.py`.
4. **Data Verification:** Compare the total byte count in `Data_Optimization_Analysis.xlsx` or the CSV files against the theoretical weights defined in the `dictionary_sensors` and confirm that low-frequency sensors only contribute data every X cycles, as expected in the ID-tag structure.
5. **Expected Output:** The ID-tag structure should demonstrate higher storage efficiency by omitting redundant data from lower-frequency sensors.
