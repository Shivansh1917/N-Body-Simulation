import pandas as pd
import numpy as np

# Set the number of bodies
num_bodies = 5000

# Generate random float data for each parameter
masses = np.random.uniform(1.0, 10.0, num_bodies)           # Mass between 1.0 and 10.0
initial_x = np.random.uniform(-100.0, 100.0, num_bodies)         # Initial x coordinate between 0.0 and 100.0
initial_y = np.random.uniform(-100.0, 100.0, num_bodies)         # Initial y coordinate between 0.0 and 100.0
initial_x_velocity = np.random.uniform(-10.0, 10.0, num_bodies) # Initial x velocity between -10.0 and 10.0
initial_y_velocity = np.random.uniform(-10.0, 10.0, num_bodies)   # Final x velocity between -10.0 and 10.0

# Create a DataFrame with all float values
data = pd.DataFrame({
    'mass': masses,
    'initial_x': initial_x,
    'initial_y': initial_y,
    'initial_x_velocity': initial_x_velocity,
    'initial_y_velocity': initial_y_velocity
})

# Save the DataFrame to a CSV file without the index
data.to_csv('nbody_data.csv', index=False)

print("CSV file 'nbody_data.csv' generated successfully.")
