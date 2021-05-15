/***************************************************************************
* Example sketch for the INA226_WE library
*
* This sketch shows how to use the limit and conversion alert of the INA226 module in parallel. 
*  
* Further information can be found on:
* https://wolles-elektronikkiste.de/ina226 (German)
* https://wolles-elektronikkiste.de/en/ina226-current-and-power-sensor (English)
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

  // Conversion will be ready after conversion time x number of averages x 2
  ina226.setAverage(AVERAGE_512); 
  ina226.setConversionTime(CONV_TIME_8244); 
  // ina226.setCorrectionFactor(0.95);
  
  Serial.println("INA226 Current Sensor Example Sketch - Limit_And_Conversion_Alert");
  
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
  ina226.setAlertType(CURRENT_UNDER, 45.0);
  ina226.enableConvReadyAlert(); // In this example we also enable the conversion ready alert interrupt
 
  attachInterrupt(digitalPinToInterrupt(interruptPin), alert, FALLING);
}

void loop() {
  if(event){
    ina226.readAndClearFlags();
    displayResults();
    attachInterrupt(digitalPinToInterrupt(interruptPin), alert, FALLING); 
    ina226.readAndClearFlags(); 
    event = false;
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
    
  if(ina226.limitAlert){
    Serial.println("Limit Alert !!!!");
  }
  if(ina226.convAlert){
    Serial.println("Conversion Alert!!!!");
  }
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
