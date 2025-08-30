import serial
import threading
from matplotlib import pyplot as plt
from collections import deque
import time

# Initialize serial communication
ser = serial.Serial('COM7', 9600, timeout=1)

# Global variables
rpm = 0.0
current = 0.0
duty = 0
time_data = deque(maxlen=100)
rpm_data = deque(maxlen=100)
current_data = deque(maxlen=100)
start_time = time.time()

# Function to read data from the serial port
def read_serial_data():
    global rpm, current, duty
    while True:
        if ser.in_waiting > 0:
            line = ser.readline().decode().strip()
            try:
                # Parse the data format: "RPM: 0, DUTY : 281 , Current :0.68"
                if "RPM:" in line and "DUTY :" in line and "Current :" in line:
                    # Normalize spaces around colons for consistent parsing
                    line = line.replace(" : ", ":").replace(" :", ":").replace(": ", ":")
                    parts = line.split(', ')
                    rpm = float(parts[0].split(':')[1])
                    duty = int(parts[1].split(':')[1])
                    current = float(parts[2].split(':')[1])

                    # Update the data for the plots
                    update_graph_data()
                else:
                    print(f"Unexpected data format: {line}")
            except (IndexError, ValueError) as e:
                print(f"Error parsing data: {e}. Line received: {line}")

# Function to update the graph data
def update_graph_data():
    current_time = time.time() - start_time
    time_data.append(current_time)
    rpm_data.append(rpm)
    current_data.append(current)

# Function to update the Matplotlib plots
def update_plots():
    ax1.clear()
    ax2.clear()

    # RPM Plot
    ax1.plot(time_data, rpm_data, label="RPM", color="blue", linewidth=1.5)
    ax1.set_title("RPM vs Time", color="white")
    ax1.set_xlabel("Time (s)", color="white")
    ax1.set_ylabel("RPM", color="white")
    ax1.set_ylim(0, 10000)  # Fixed max RPM is 10,000
    ax1.legend()
    ax1.grid(True, color="gray", linestyle="--", linewidth=0.5)
    ax1.tick_params(colors="white")

    # Current Plot
    ax2.plot(time_data, current_data, label="Current", color="red", linewidth=1.5)
    ax2.set_title("Current vs Time", color="white")
    ax2.set_xlabel("Time (s)", color="white")
    ax2.set_ylabel("Current (A)", color="white")
    ax2.set_ylim(0, 6)  # Fixed max Current is 6A
    ax2.legend()
    ax2.grid(True, color="gray", linestyle="--", linewidth=0.5)
    ax2.tick_params(colors="white")

    # Set black background
    fig.patch.set_facecolor("black")
    ax1.set_facecolor("black")
    ax2.set_facecolor("black")

    plt.pause(0.1)

# Start reading serial data in a separate thread
def start_serial_thread():
    serial_thread = threading.Thread(target=read_serial_data, daemon=True)
    serial_thread.start()

# Initialize Matplotlib Figure and Axes
plt.ion()
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(10, 5))

# Start serial thread
start_serial_thread()

# Real-time plot update loop
while True:
    update_plots()
