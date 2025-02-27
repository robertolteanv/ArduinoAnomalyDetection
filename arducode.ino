#include <DHT.h>
#include <EEPROM.h>

const int MQ2_PIN = A0;            // Analog pin for MQ-2 gas sensor (AO)
const int PHOTO_PIN = A1;          // Analog pin for photoresistor
const int DHT_PIN = 2;             // Digital pin for DHT sensor (Out pin)

#define DHTTYPE DHT22              // DHT sensor type (DHT22)

DHT dht(DHT_PIN, DHTTYPE);

float mq2Threshold = 300;  // Threshold for gas concentration anomaly detection
unsigned long lastMQ2Read = 0;  // Time of last MQ2 reading
unsigned long lastTempHumRead = 0;  // Time of last Temp/Hum reading
unsigned long lastLightRead = 0;  // Time of last Light reading

const unsigned long mq2Interval = 3000; // 3 seconds for MQ-2 sensor
const unsigned long tempHumInterval = 20000; // 20 seconds for Temp/Hum sensor
const unsigned long lightInterval = 1000; // 8 seconds for Light intensity sensor

// Variables to store previous readings and smoothed values
float prevGasValue = 0;
float prevTemp = 0;
float prevHum = 0;
float prevLight = 0;

// Moving average for temperature and humidity
float tempSum = 0;
float humSum = 0;
int tempCount = 0;
int humCount = 0;

// Dynamic array to store all preceding gas values (for median filter)
float gasHistory[50];  // Array to store gas readings (size of 50 for example)
int gasIndex = 0;  // To keep track of the current index in gasHistory array

// Helper function to calculate median from a dynamic array
float calculateMedian(float arr[], int size) {
  for (int i = 0; i < size - 1; i++) {
    for (int j = i + 1; j < size; j++) {
      if (arr[i] > arr[j]) {
        // Swap elements
        float temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
      }
    }
  }
  return arr[size / 2];
}

// Store data in EEPROM
void storeDataToEEPROM(int &address, float gasValue, unsigned long timeStamp, bool gasAnomaly) {
  // Store gas concentration value (4 bytes)
  EEPROM.put(address, gasValue);
  address += sizeof(gasValue); // Move address pointer for next value

  // Store timestamp (4 bytes)
  EEPROM.put(address, timeStamp);
  address += sizeof(timeStamp); // Move address pointer for next value

  // Store gas anomaly flag (1 byte)
  EEPROM.write(address, gasAnomaly);
  address += sizeof(gasAnomaly); // Move address pointer for next value

  // Output EEPROM usage
  long usedEEPROM = address; // Current EEPROM address is the used space
  float percentUsed = (float)usedEEPROM / EEPROM.length() * 100;
  Serial.print("EEPROM Usage: ");
  Serial.print(percentUsed);
  Serial.print("% (");
  Serial.print(usedEEPROM);
  Serial.print(" bytes of ");
  Serial.print(EEPROM.length());
  Serial.println(" bytes)");
}

void setup() {
  Serial.begin(9600);
  dht.begin();
}

void loop() {
  unsigned long currentMillis = millis();  // Current time in milliseconds

  // MQ-2 Sensor: Read every 3 seconds
  if (currentMillis - lastMQ2Read >= mq2Interval) {
    int mq2Value = analogRead(MQ2_PIN);
    float gasConcentration = mq2Value * 1.25; // Approximate concentration in ppm (adjust as necessary)

    // Store gas reading in the history array
    gasHistory[gasIndex] = gasConcentration;
    gasIndex = (gasIndex + 1) % 50;  // Wrap around the array when reaching the limit

    // Calculate median using all preceding values
    float smoothedGasValue = calculateMedian(gasHistory, 50);

    // Anomaly detection for gas concentration
    bool gasAnomaly = abs(smoothedGasValue - prevGasValue) > mq2Threshold;

    if (gasAnomaly) {
      Serial.print("****Gas Anomaly Detected!**** ");
      Serial.print("Time: ");
      Serial.print(getFormattedTime(currentMillis));
      Serial.print("\t");
      Serial.print("Methane PPM: ");
      Serial.println(smoothedGasValue);
    } else {
      Serial.print("Gas Sensor (MQ-2) Value: ");
      Serial.print("Time: ");
      Serial.print(getFormattedTime(currentMillis));
      Serial.print("\t");
      Serial.print("Methane PPM: ");
      Serial.println(smoothedGasValue);
    }

    prevGasValue = smoothedGasValue;  // Store for next comparison

    // Store the gas value and anomaly detection result to EEPROM
    static int eepromAddress = 0; // Start at address 0
    storeDataToEEPROM(eepromAddress, smoothedGasValue, currentMillis, gasAnomaly);

    lastMQ2Read = currentMillis; // Update the last read time
  }

  // Temperature and Humidity Sensor: Read every 20 seconds
  if (currentMillis - lastTempHumRead >= tempHumInterval) {
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    // Handle DHT sensor failure
    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("Failed to read from DHT sensor!");
    } else {
      // Update moving average for temperature and humidity
      tempSum += temperature;
      humSum += humidity;
      tempCount++;
      humCount++;

      if (tempCount > 10) {
        float avgTemp = tempSum / tempCount;
        float avgHum = humSum / humCount;

        // Anomaly detection for temperature and humidity
        if (abs(temperature - avgTemp) > 2 || abs(humidity - avgHum) > 10) {
          Serial.print("****Temperature/Humidity Anomaly Detected!**** ");
          Serial.print("Time: ");
          Serial.print(getFormattedTime(currentMillis));
          Serial.print("\t");
          Serial.print("Temperature: ");
          Serial.print(temperature);
          Serial.print(" °C\t");
          Serial.print("Humidity: ");
          Serial.println(humidity);
        } else {
          Serial.print("Temperature: ");
          Serial.print("Time: ");
          Serial.print(getFormattedTime(currentMillis));
          Serial.print("\t");
          Serial.print("Temperature: ");
          Serial.print(temperature);
          Serial.print(" °C\t");
          Serial.print("Humidity: ");
          Serial.println(humidity);
        }

        tempSum = 0;  // Reset for next set of readings
        humSum = 0;
        tempCount = 0;
        humCount = 0;
      }

      prevTemp = temperature;
      prevHum = humidity;
    }
    lastTempHumRead = currentMillis;  // Update the last read time
  }

  // Light Intensity Sensor: Read every 5-10 seconds
  if (currentMillis - lastLightRead >= lightInterval) {
    int lightLevel = analogRead(PHOTO_PIN);
    float lux = map(lightLevel, 0, 1023, 0, 1000);  // Convert to a reasonable lux range (adjust as necessary)

    // Apply range limiting to avoid extreme values
    if (lightLevel < 900) {
      lightLevel -= 400;  // Too dark, treat as no light
    } else if (lightLevel > 900) {
      lightLevel = 1000;  // Too bright, cap at maximum value
    }

    // Anomaly detection for light intensity
    if (abs(lightLevel - prevLight) > 200) {  // Set threshold for sudden jumps in light level
      Serial.print("****Light Intensity Anomaly Detected!**** ");
      Serial.print("Time: ");
      Serial.print(getFormattedTime(currentMillis));
      Serial.print("\t");
      Serial.print("Light Level: ");
      Serial.println(lightLevel);
    } else {
      Serial.print("Light Level: ");
      Serial.print("Time: ");
      Serial.print(getFormattedTime(currentMillis));
      Serial.print("\t");
      Serial.print("Light Level: ");
      Serial.println(lightLevel);
    }

    prevLight = lightLevel;  // Store for next comparison
    lastLightRead = currentMillis;  // Update the last read time
  }
}

// Helper function to get formatted time in hh:mm:ss format
String getFormattedTime(unsigned long currentMillis) {
  unsigned long seconds = currentMillis / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;

  minutes = minutes % 60;
  seconds = seconds % 60;

  String formattedTime = String(hours) + ":" + String(minutes) + ":" + String(seconds);
  return formattedTime;
}
