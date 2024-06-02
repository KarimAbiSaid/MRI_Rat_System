import serial
import csv
import time
import os

# Configure the serial port

ser = serial.Serial('COM5', 115200)
time.sleep(1)  # Wait for the serial connection to initialize

# Write initial lines to the Arduino
ser.write(b"0\n") #the delay in ms to the sensor
file_path = 'arduino_data_+_date.csv' #the name of the csv file we will write to





expected_header = ['Time Elapsed','MaxAngle','Delay Time','Servo Position', 'Contact Value','Hand Position']

def check_header(file_path, expected_header):
    file_exists = os.path.exists(file_path)
    if not file_exists:
        return False
    with open(file_path, mode='r', newline='') as file:
        reader = csv.reader(file)
        first_row = next(reader, None)
        return first_row == expected_header

# Check if the header matches
header_matches = check_header(file_path, expected_header)

# Open the CSV file
mode = 'a' if header_matches else 'w'
with open(file_path, mode=mode, newline='') as file:
    writer = csv.writer(file)
    
    # Write the header if necessary
    if not header_matches:
        writer.writerow(expected_header)

    try:
        counter = 0
        while True:
            
            if ser.in_waiting > 0:
                if counter < 20:
                    counter +=1
                    data = ser.readline().decode('utf-8').strip()
                    print("not logging these ones")
                    continue
                data = ser.readline().decode('utf-8').strip()
                print(data)  # Print the received data to the console
                row = data.split(',')
                writer.writerow(row)
    except KeyboardInterrupt:
        print("Data logging stopped.")

ser.close()
