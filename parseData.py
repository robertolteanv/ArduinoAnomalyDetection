import serial
import matplotlib.pyplot as plt
import time
import re
import matplotlib.animation as animation

# Open the serial port
ser = serial.Serial('COM3', 9600, timeout=1)

# Lists to store the data
times = []
mq2_raw = []
mq2_smoothed = []
mq2_rolling_avg = []
light_raw = []
light_rolling_avg = []
dht_temp = []
dht_humidity = []

# Anomalies
mq2_anomalies = []
light_anomalies = []
dht_anomalies = []

# Timing for performance analysis
mq2_timings = []
light_timings = []
dht_timings = []
rolling_anomaly_timings = []
simple_anomaly_timings = []

# Updated regex pattern for DHT data
dht_pattern = r'Temperature: ([0-9.]+)\s*(?:\(.*\))?\s*Humidity: ([0-9.]+)'

# Initialize the plot
fig, (ax1, ax2, ax3, ax4, ax5) = plt.subplots(5, 1, figsize=(10, 12))  # Added ax5 for new graph


# Function to update the plot
def update_plot(frame):
    try:
        start_time = time.time()  # Start time for performance tracking

        line = ser.readline().decode('utf-8').strip()
        if line:
            print(f"Received line: {line}")

            # Check for MQ2 data
            # Check for MQ2 data
            if '[MQ-2 Sensor]' in line:
                try:
                    # Only process lines with sensor data
                    if 'Execution time for rolling anomaly check' not in line and 'Execution time for simple anomaly check' not in line:
                        mq2_start = time.time()  # Initialize the start time for MQ2 processing

                        parts = line.split('\t')
                        raw_ppm = float(parts[1].split(': ')[1])
                        smoothed_ppm = float(parts[2].split(': ')[1])
                        rolling_avg = float(parts[3].split(': ')[1])

                        mq2_raw.append(raw_ppm)
                        mq2_smoothed.append(smoothed_ppm)
                        mq2_rolling_avg.append(rolling_avg)

                        if 'ANOMALY' in line:
                            mq2_anomalies.append(len(mq2_raw) - 1)
                        else:
                            mq2_anomalies.append(None)

                        # Measure MQ2 sensor processing time
                        mq2_timings.append(time.time() - mq2_start)

                    # Capture rolling anomaly time
                    if 'Execution time for rolling anomaly check' in line:
                        rolling_time = int(re.search(r"(\d+) µs", line).group(1))
                        rolling_anomaly_timings.append(rolling_time)

                    # Capture simple anomaly time
                    if 'Execution time for simple anomaly check' in line:
                        simple_time = int(re.search(r"(\d+) µs", line).group(1))
                        simple_anomaly_timings.append(simple_time)

                except IndexError:
                    print(f"Error parsing MQ2 data: {line}")


            # Check for Light sensor data
            elif '[Light Sensor]' in line:
                try:
                    # Use regex to extract numeric values
                    match = re.search(r'Current:\s*([0-9]+)\s*Rolling Avg:\s*([0-9.]+)', line)
                    if match:
                        light_level = int(match.group(1))
                        light_avg = float(match.group(2))

                        # Append the data
                        light_raw.append(light_level)
                        light_rolling_avg.append(light_avg)

                        # Check for anomaly in Light
                        if 'ANOMALY' in line:
                            light_anomalies.append(len(light_raw) - 1)  # Mark anomaly point by index
                        else:
                            light_anomalies.append(None)
                    else:
                        raise ValueError("Could not extract Light sensor data.")

                except (IndexError, ValueError) as e:
                    print(f"Error parsing Light sensor data: {line} | Error: {e}")

            # Check for DHT sensor data
            elif '[DHT Sensor]' in line:
                dht_start = time.time()
                match = re.search(dht_pattern, line)
                if match:
                    temp = float(match.group(1))
                    humidity = float(match.group(2))

                    dht_temp.append(temp)
                    dht_humidity.append(humidity)

                    if 'ANOMALY' in line:
                        dht_anomalies.append(len(dht_temp) - 1)
                    else:
                        dht_anomalies.append(None)

                    dht_timings.append(time.time() - dht_start)
                else:
                    print(f"Error parsing DHT sensor data: {line}")

    except Exception as e:
        print(f"Error reading serial data: {e}")

    # Update MQ2 plot
    ax1.clear()
    ax1.plot(mq2_raw, label='Raw PPM')
    ax1.plot(mq2_smoothed, label='Smoothed PPM')
    ax1.plot(mq2_rolling_avg, label='Rolling Avg')

    for anomaly_index in mq2_anomalies:
        if anomaly_index is not None:
            ax1.plot(anomaly_index, mq2_raw[anomaly_index], 'ro')

    ax1.legend(loc='upper right')
    ax1.set_title('MQ2 Sensor Data')
    ax1.set_ylabel('PPM')

    # Update Light Sensor plot
    ax2.clear()
    ax2.plot(light_raw, label='Light Level')
    ax2.plot(light_rolling_avg, label='Light Rolling Avg')

    for anomaly_index in light_anomalies:
        if anomaly_index is not None:
            ax2.plot(anomaly_index, light_raw[anomaly_index], 'ro')

    ax2.legend(loc='upper right')
    ax2.set_title('Light Sensor Data')
    ax2.set_ylabel('Light Level')

    # Update DHT Sensor plot
    ax3.clear()
    ax3.plot(dht_temp, label='Temperature (°C)')
    ax3.plot(dht_humidity, label='Humidity (%)')

    for anomaly_index in dht_anomalies:
        if anomaly_index is not None:
            ax3.plot(anomaly_index, dht_temp[anomaly_index], 'ro')

    ax3.legend(loc='upper right')
    ax3.set_title('DHT Sensor Data')
    ax3.set_ylabel('Temperature / Humidity')



    # Plot for comparing rolling anomaly and simple anomaly times
    ax5.clear()
    ax5.plot(rolling_anomaly_timings, label='Rolling Anomaly Time (µs)', marker='o')
    ax5.plot(simple_anomaly_timings, label='Simple Anomaly Time (µs)', marker='o')

    ax5.legend(loc='upper right')
    ax5.set_title('Comparison of Anomaly Check Times')
    ax5.set_ylabel('Time (µs)')
    ax5.set_xlabel('Sample Index')

    plt.tight_layout()


# Create animation
ani = animation.FuncAnimation(fig, update_plot, interval=100)

plt.show()
