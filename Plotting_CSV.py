import pandas as pd
import matplotlib.pyplot as plt
from datetime import timedelta
import csv
from matplotlib.ticker import MaxNLocator

# Load the CSV file
file_path = 'arduino_data_29_05_In_Vivo.csv'

#Experiment name:
exp_name = "May 29 In Vivo"

with open(file_path) as f:
    csvread = csv.reader(f)
    batch_read = list(csvread)

data = pd.DataFrame(batch_read)

# Define the expected number of columns
expected_columns = 6

# Filter out rows that do not have the expected number of columns
data = data.dropna(thresh=expected_columns)
data = data.dropna(axis=1)

# Reset the index after dropping rows
data.reset_index(drop=True, inplace=True)

# Assign proper column names
data.columns = ['Time Elapsed', 'MaxAngle', 'Delay Time', 'Servo Position', 'Contact Value', 'Hand Position']

# Convert 'Time Elapsed' to seconds and check the format
    
def convert_to_seconds_elapsed(time_str):
    parts = time_str.split(':')
    if len(parts) != 4:
        return None
    try:
        h, m, s, ms = map(int, parts)
        return (h*3600 + m * 60 + s + float(ms)/1000)

    except ValueError:
        return None
    
def correct_time_jumps(time_series):
    corrected_time = time_series.copy()
    
    for i in range(1, len(corrected_time)):
        # Check if there is a small downward jump
        if corrected_time[i] < corrected_time[i - 1] and (corrected_time[i - 1] - corrected_time[i]) < 1:
            corrected_time[i] = corrected_time[i - 1] + (corrected_time[i - 1] - corrected_time[i - 2])
    
    return corrected_time



data['Time Elapsed'] = data['Time Elapsed'].apply(convert_to_seconds_elapsed)
data['Time Elapsed'] = correct_time_jumps(data['Time Elapsed'])

# Drop rows where 'Time Elapsed' could not be converted
data = data.dropna(subset=['Time Elapsed'])
# time_elapsed_array = fix_negative_jumps(data['Time Elapsed'].to_numpy())
# data['Time Elapsed'] = time_elapsed_array


# Ensure 'Time Elapsed' is continuous
total_elapsed = 0
previous_time = 0
continuous_time = []
# print("timedelta() = ",total_elapsed)


for time in data['Time Elapsed']:
    if time is not None:
        if time < previous_time:
            total_elapsed += previous_time
        previous_time = time
        continuous_time.append(total_elapsed + time)
    else:
        continuous_time.append(None)

data['Continuous Time Elapsed'] = continuous_time


# Convert 'Continuous Time Elapsed' to seconds for plotting
# data['Continuous Time Elapsed'] = data['Continuous Time Elapsed']

# Convert 'Servo Position' and 'Hand Position' to numeric and drop rows with invalid values
data['Servo Position'] = pd.to_numeric(data['Servo Position'], errors='coerce')
data['Hand Position'] = pd.to_numeric(data['Hand Position'], errors='coerce')
data['MaxAngle'] = pd.to_numeric(data['MaxAngle'], errors='coerce')
data['Contact Value'] = pd.to_numeric(data['Contact Value'], errors='coerce')

data = data.dropna(subset=['Servo Position', 'Hand Position', 'MaxAngle','Contact Value'])
# print(data['Time Elapsed'][98420:98440])
# print(data['Continuous Time Elapsed'][98420:98440])


# plt.plot(data['Continuous Time Elapsed'])
# plt.show()

# '''
# Plotting
fig, ax = plt.subplots()

# Highlight sections with different shaded areas for each 'Delay Time'
delay_times = data['Delay Time'].unique()
colors = plt.cm.get_cmap('tab20', len(delay_times))

for i, delay_time in enumerate(delay_times):
    subset = data[data['Delay Time'] == delay_time]
    if not subset.empty:
        start_time = subset['Continuous Time Elapsed'].min()
        end_time = subset['Continuous Time Elapsed'].max()
        ax.axvspan(start_time, end_time, color=colors(i), alpha=0.3, label=f'Delay {delay_time}')

start_index = 0
end_index = -1

# Plot the data with the same color for lines
# ax.plot(data['Continuous Time Elapsed'][start_index:end_index],data['Servo Position'][start_index:end_index], label='Servo Position', color='blue')
# ax.stem(data['Continuous Time Elapsed'][start_index:end_index], data['Hand Position'][start_index:end_index], label='Hand Position')#, color='orange')
# ax.plot(data['Continuous Time Elapsed'][start_index:end_index], data['MaxAngle'][start_index:end_index], label='Maximum Servo Angle', color='red', linestyle = "--")
# ax.plot(data['Continuous Time Elapsed'][start_index:end_index], 0.4 * data['Contact Value'][start_index:end_index], label='Contact With Rat', color='green')


ax.plot(data['Continuous Time Elapsed'],data['Servo Position'], label='Servo Position', color='blue')
ax.plot(data['Continuous Time Elapsed'], data['Hand Position'], label='Hand Position')#, color='orange')
ax.plot(data['Continuous Time Elapsed'], data['MaxAngle'], label='Maximum Servo Angle', color='red', linestyle = "--")
ax.plot(data['Continuous Time Elapsed'], 0.4 * data['Contact Value'], label='Contact With Rat', color='green')

# Customize the number of ticks on the y-axis
ax.yaxis.set_major_locator(MaxNLocator(nbins=10))  # Change the number of ticks on the y-axis by setting nbins

# Customize the plot
ax.set_xlabel('Continuous Time Elapsed (s)')
ax.set_ylabel('Position')
ax.set_title('Servo Position and Hand Position Over Time - ' + exp_name)
ax.legend(loc='upper left', bbox_to_anchor=(1, 1), ncol=1)

# Show the plot
plt.tight_layout()
plt.show()

# '''
# import numpy as np
# from scipy import signal
# start_index = 1800
# end_index = 2300
# plt.stem( data['Continuous Time Elapsed'][start_index:end_index][0:250],(signal.correlate(data['Servo Position'][start_index:end_index],data['Hand Position'][start_index:end_index], 'same'))[250:500])
# plt.show()