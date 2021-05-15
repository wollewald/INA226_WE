/***************************************************************************
* Example sketch for the INA226_WE library
*
* This sketch shows how to use the limit alert of the INA226 module.
*  
* Further information can be found on:
* https://wolles-elektronikkiste.de/en/ina226-current-and-power-sensor (English)
* https://wolles-elektronikkiste.de/ina226 (German)
* 
***************************************************************************/
#include <Wire.h>
#include <INA226_WE.h>
#define I2C_ADDRESS 0x40

int interruptPin = 2;
volatile bool event = false;

/* There are several ways to create your INA226 object:
 * INA226_WE ina226 = INA226_WE()              -> uses Wire / I2C Address = 0x40
 * INA226_WE ina226 = INA226_WE(ICM20948_ADDR) -> uses Wire / I2C_ADDRESS
 * INA226_WE ina226 = INA226_WE(&wire2)        -> uses the TwoWire object wire2 / I2C_ADDRESS
 * INA226_WE ina226 = INA226_WE(&wire2, I2C_ADDRESS) -> all together
 * Successfully tested with two I2C busses on an ESP32
 */
INA226_WE ina226 = INA226_WE(I2C_ADDRESS);

void setup() {
  Serial.begin(9600);
  Wire.begin();
  ina226.init();

  /* Set Number of measurements for shunt and bus voltage which shall be averaged
  * Mode *     * Number of samples *
  AVERAGE_1            1 (default)
  AVERAGE_4            4
  AVERAGE_16          16
  AVERAGE_64          64
  AVERAGE_128        128
  AVERAGE_256        256
  AVERAGE_512        512
  AVERAGE_1024      1024
  */
  // ina226.setAverage(AVERAGE_1024); 

  /* Set conversion time in microseconds
     One set of shunt and bus voltage conversion will take: 
     number of samples to be averaged x conversion time x 2
     
     * Mode *         * conversion time *
     CONV_TIME_140          140 µs
     CONV_TIME_204          204 µs
     CONV_TIME_332          332 µs
     CONV_TIME_588          588 µs
     CONV_TIME_1100         1.1 ms (default)
     CONV_TIME_2116       2.116 ms
     CONV_TIME_4156       4.156 ms
     CONV_TIME_8244       8.244 ms  
  */
  // ina226.setConversionTime(CONV_TIME_8244); 
  
  /* Set measure mode
  POWER_DOWN - INA226 switched off
  TRIGGERED  - measurement on demand
  CONTINUOUS  - continuous measurements (default)
  */
  //ina226.setMeasureMode(CONTINUOUS); // choose mode and uncomment for change of default
  
  /* Set Current Range
    * Mode *   * Max Current *
     MA_400          400 mA
     MA_800          800 mA (default)
  */
  //ina226.setCurrentRange(MA_800); // choose gain and uncomment for change of default
  
  /* If the current values delivered by the INA226 differ by a constant factor
     from values obtained with calibrated equipment you can define a correction factor.
     Correction factor = current delivered from calibrated equipment / current delivered by INA226
  */
  // ina226.setCorrectionFactor(0.95);
  
  Serial.println("INA226 Current Sensor Example Sketch - Limit_Alert");
  
  /* In the default mode the limit interrupt flag will be deleted after the next measurement within limits. 
     With enableAltertLatch(), the flag will have to be deleted with readAndClearFlags(). 
  */
    ina226.enableAlertLatch();
  
  /* Set the alert type and the limit
      * Mode *        * Description *           * limit unit *
    SHUNT_OVER     Shunt Voltage over limit          mV
    SHUNT_UNDER    Shunt Voltage under limit         mV
    CURRENT_OVER   Current over limit                mA
    CURRENT_UNDER  Current under limit               mA
    BUS_OVER       Bus Voltage over limit            V
    BUS_UNDER      Bus Voltage under limit           V
    POWER_OVER     Power over limit                  mW
  */
  ina226.setAlertType(POWER_OVER, 230.0);
 
  attachInterrupt(digitalPinToInterrupt(interruptPin), alert, FALLING);
}

void loop() {
  if(event){
    ina226.readAndClearFlags(); // reads interrupt and overflow flags and deletes them 
    displayResults();
    attachInterrupt(digitalPinToInterrupt(interruptPin), alert, FALLING); 
    event = false;
    ina226.readAndClearFlags();
  }  
  delay(1000);
}

void displayResults(){
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
  if(!ina226.overflow){
    Serial.println("Values OK - no overflow");
  }
  else{
    Serial.println("Overflow! Choose higher current range");
  }
  Serial.println();
}

void alert(){
  event = true;
  detachInterrupt(2);
}
