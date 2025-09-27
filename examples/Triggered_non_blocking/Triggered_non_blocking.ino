/***************************************************************************
* Example sketch for the INA226_WE library
*
* This sketch shows how to use the INA226 module in triggered mode without 
* without blocking. The function startSingleMeasurement() makes the program 
* wait until the conversion is ready, whereas startSingleMeasurementNoWait() 
* will only initiate the conversion. 
*  
* Further information can be found on:
* https://wolles-elektronikkiste.de/en/ina226-current-and-power-sensor (English)
* https://wolles-elektronikkiste.de/ina226 (German)
* 
***************************************************************************/
#include <Wire.h>
#include <INA226_WE.h>
#define I2C_ADDRESS 0x40
#define SAMPLING_PERIOD 1000
int interruptPin = 2;
volatile bool convReady = false;

/* There are several ways to create your INA226 object:
 * INA226_WE ina226 = INA226_WE(); -> uses I2C Address = 0x40 / Wire
 * INA226_WE ina226 = INA226_WE(I2C_ADDRESS);   
 * INA226_WE ina226 = INA226_WE(&Wire); -> uses I2C_ADDRESS = 0x40, pass any Wire Object
 * INA226_WE ina226 = INA226_WE(&Wire, I2C_ADDRESS); 
 */
INA226_WE ina226 = INA226_WE(I2C_ADDRESS);

void setup() {
  Serial.begin(115200);
  Wire.begin();
  if(!ina226.init()){
    Serial.println("Failed to init INA226. Check your wiring.");
    while(1){}
  }

  /* Set Number of measurements for shunt and bus voltage which shall be averaged
        * Mode *         * Number of samples *
    INA226_AVERAGE_1            1 (default)
    INA226_AVERAGE_4            4
    INA226_AVERAGE_16           8
    INA226_AVERAGE_64          64
    INA226_AVERAGE_128        128
    INA226_AVERAGE_256        256
    INA226_AVERAGE_512        512
    INA226_AVERAGE_1024      1024
  */
  //ina226.setAverage(INA226_AVERAGE_1); // choose mode and uncomment for change of default

  /* Set conversion time in microseconds
    One set of shunt and bus voltage conversion will take: 
    number of samples to be averaged x conversion time x 2
     
    * Mode *                 * conversion time *
    INA226_CONV_TIME_140          140 µs
    INA226_CONV_TIME_204          204 µs
    INA226_CONV_TIME_332          332 µs
    INA226_CONV_TIME_588          588 µs
    INA226_CONV_TIME_1100         1.1 ms (default)
    INA226_CONV_TIME_2116       2.116 ms
    INA226_CONV_TIME_4156       4.156 ms
    INA226_CONV_TIME_8244       8.244 ms  
  */
  //ina226.setConversionTime(INA226_CONV_TIME_1100); //choose conversion time and uncomment for change of default
  
  /* Set measure mode
    INA226_POWER_DOWN              - INA226 switched off
    INA226_TRIGGERED               - on demand, both current and bus voltage
    INA226_TRIGGERED_CURRENT_ONLY  - on demand, current only
    INA226_TRIGGERERD_BUS_ONLY     - on demand, bus voltage only
    INA226_CONTINUOUS              - continuous, both current and bus voltage (default)
    INA226_CONTINUOUS_CURRENT_ONLY - continuous, current only
    INA226_CONTINUOUS_BUS_ONLY     - continuous, bus voltage only
  */
  ina226.setMeasureMode(INA226_TRIGGERED); // choose mode and uncomment for change of default
  
  /* If the current values delivered by the INA226 differ by a constant factor
    from values obtained with calibrated equipment you can define a correction factor.
    Correction factor = current measured with calibrated device / current measured by INA226
    Be aware that Imax depends on the real shunt size.
  */
  // ina226.setCorrectionFactor(0.95);
  
  Serial.println("INA226 Current Sensor Example Sketch - Triggered, non-blocking");
  attachInterrupt(digitalPinToInterrupt(interruptPin), convReadyISR, FALLING);
  ina226.enableConvReadyAlert();
   
  // ina226.waitUntilConversionCompleted(); //makes no sense - in triggered mode we wait anyway for comleted conversion
}

void loop() {
  static unsigned long lastMeasurement = 0;
  
  if(millis() - lastMeasurement > SAMPLING_PERIOD) {
    ina226.startSingleMeasurementNoWait();
    lastMeasurement = millis();
  }
  
  if(convReady) { 
    getAndPrintSample();
    ina226.readAndClearFlags();
    convReady = false;
    attachInterrupt(digitalPinToInterrupt(interruptPin), convReadyISR, FALLING);
  }  
}

void getAndPrintSample() {
  float shuntVoltage_mV = 0.0;
  float loadVoltage_V = 0.0;
  float busVoltage_V = 0.0;
  float current_mA = 0.0;
  float power_mW = 0.0; 

  shuntVoltage_mV = ina226.getShuntVoltage_mV();
  busVoltage_V = ina226.getBusVoltage_V();
  current_mA = ina226.getCurrent_mA();
  power_mW = ina226.getBusPower();
  loadVoltage_V  = busVoltage_V + (shuntVoltage_mV/1000);
    
  Serial.print("Shunt Voltage [mV]: "); Serial.println(shuntVoltage_mV);
  Serial.print("Bus Voltage [V]: "); Serial.println(busVoltage_V);
  Serial.print("Load Voltage [V]: "); Serial.println(loadVoltage_V);
  Serial.print("Current[mA]: "); Serial.println(current_mA);
  Serial.print("Bus Power [mW]: "); Serial.println(power_mW);
  if(!ina226.overflow) {
    Serial.println("Values OK - no overflow");
  }
  else {
    Serial.println("Overflow! Seems you need a smaller shunt.");
  }
  Serial.println();
}

void convReadyISR() {
  convReady = true;
  detachInterrupt(digitalPinToInterrupt(interruptPin));
}

