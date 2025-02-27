# 🚀 Embedded Sensor System - Anomaly Detection & Data Logging


## 📌 Overview
This project is an **Embedded Sensor System** that collects real-time environmental data from multiple sensors, detects anomalies using different algorithms, and logs the data efficiently.

### 📡 Supported Sensors
- **MQ-2 Gas Sensor** (Detects methane, butane, etc.)
- **DHT22 Temperature & Humidity Sensor**
- **Photoresistor (Light Intensity Sensor)**

### 🔍 Key Features
✅ **Real-time data collection** from multiple sensors  
✅ **Anomaly Detection** using three different algorithms  
✅ **Optimized data storage** with EEPROM logging  
✅ **Performance Analysis** with execution time tracking  
✅ **Robust validation & testing methods**

---

## 🛠️ Setup & Installation
### 📌 Prerequisites
Ensure you have the following installed:
- **Arduino IDE**
- **ESP8266 / ESP32 / Arduino Board**
- **Required sensor modules** (MQ-2, DHT22, Photoresistor)

### 🔧 Steps to Run the Project
1. **Clone the repository**:
   ```bash
   git clone https://github.com/yourusername/embedded-sensor-system.git
   cd embedded-sensor-system
   ```
2. **Open the project in Arduino IDE**
3. **Install required libraries**:
   - `DHT.h` (for temperature & humidity sensor)
   - `EEPROM.h` (for data logging)
4. **Upload the code** to your microcontroller
5. **Monitor sensor readings** via Serial Monitor (9600 baud rate)

---

## 📊 Anomaly Detection Algorithms
This project implements three methods:

| Algorithm                | Pros                                      | Cons                                     | Time Complexity |
|--------------------------|------------------------------------------|-----------------------------------------|----------------|
| **Threshold-Based**      | Simple & fast                            | Requires manual tuning                   | O(1)           |
| **Rolling Average**      | Adapts to gradual changes               | Slight lag in detection                  | O(1)           |
| **Statistical Outlier**  | Robust & noise-resistant                 | More computational overhead              | O(n log n)     |

---

## ⏱️ Execution Time Analysis

| Operation               | Execution Time (µs) | Time Complexity |
|-------------------------|--------------------|----------------|
| `calculateMedian()`     | 2-3                | O(n log n)     |
| `updateRollingAverage()`| 3                  | O(1)           |
| `checkDHTAnomalies()`   | 2                  | O(1)           |
| `Light processing`      | 100-101            | O(1)           |
| `MQ-2 processing`       | 383                | O(n)           |
| `storeDataToEEPROM()`   | 107923             | O(n)           |

