/*Arduino/Energia sketch for testing an Amphenol DLCL02DD4 pressure sensor
 * for use in metering air flow +/-2"WC max pressure - medical & HVAC
 * applications - by Todd Miller 2020
 */


#include <Wire.h>

/*global variables*/
byte pressureSensorStatus; //The first byte from the sensor via I2C is status. A complete temperature and pressure reading without error is 64 (0x40). From DS-0365 REV A data sheet page 8.
float pressure;
float pressureSensorCalibrationError;
float temperature;

/*functions to make setup() and loop() cleaner*/
void getPressureSensorData() {
  Wire.beginTransmission(41);
  Wire.write(byte(0xAF));
  Wire.endTransmission();
  delay(300);
  Wire.requestFrom(41, 7);    //Request 7 bytes from slave device #41. From DS-0365 REV A data sheet page 8 and page 5.
  while(Wire.available()) {
    pressureSensorStatus = Wire.read();
    if (pressureSensorStatus == 64) {  
      unsigned long bufferForPressure = Wire.read();
      bufferForPressure = bufferForPressure << 8;
      bufferForPressure |= Wire.read();
      bufferForPressure = bufferForPressure << 8;
      bufferForPressure |= Wire.read();
      pressure = 1.25 * ( bufferForPressure - 8388608 ) / 1677216 *4;  //Convert sensor data to inches of water column [in.WC]. Unique conversion for model #DLCL02D(All Packages). From DS-0365 REV A data sheet page 5.
    
      unsigned long bufferForTemperature = Wire.read();
      bufferForTemperature = bufferForTemperature << 8;
      bufferForTemperature |= Wire.read();
      bufferForTemperature = bufferForTemperature << 8;
      bufferForTemperature |= Wire.read();
      Wire.endTransmission();
      temperature = ( ( bufferForTemperature * 150 / 16777216 ) ) - 40;  //Convert sensor data to temperature [C]. Unique conversion for model #DLCL02D(All Packages). From DS-0365 REV A data sheet page 5.
      temperature = ( 1.8 * temperature ) + 32;  //Convert to freedom units [F]   
    }
  }
}

/*code to be run once before entering loop()*/
void setup() {
  // put your setup code here, to run once:
  delay(3000);
  Wire.begin();
  Serial.begin(9600);
  getPressureSensorData();
  pressureSensorCalibrationError = pressure;
}

/*your code here loops forever*/
void loop() {
  getPressureSensorData();
  if (pressureSensorStatus == 64) {
    pressure = pressure - pressureSensorCalibrationError;
    Serial.println("DLC Sensor status OK");
    Serial.print(pressure);
    Serial.print("inWC\n");
    Serial.print(temperature);
    Serial.print("F\n\n");
    delay(2000);
  } else {
    Serial.print("Pressure sensor error: ");
    Serial.print(pressureSensorStatus);
    Serial.print("\n");
    delay(2000);
  }
}
