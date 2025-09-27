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
 * INA226_WE ina226 = INA226_WE(); -> uses I2C Address = 0x40 / Wire
 * INA226_WE ina226 = INA226_WE(I2C_ADDRESS);   
 * INA226_WE ina226 = INA226_WE(&Wire); -> uses I2C_ADDRESS = 0x40, pass any Wire Object
 * INA226_WE ina226 = INA226_WE(&Wire, I2C_ADDRESS); 
 */
INA226_WE ina226 = INA226_WE(I2C_ADDRESS);

void setup() {
  Serial.begin(115200);
  pinMode(interruptPin, INPUT_PULLUP); // for modules without internal pullup
  Wire.begin();
  ina226.init();

  // Conversion will be ready after conversion time x number of averages x 2
  ina226.setAverage(INA226_AVERAGE_512); 
  ina226.setConversionTime(INA226_CONV_TIME_4156);
  // ina226.setCorrectionFactor(0.95);
  
  Serial.println("INA226 Current Sensor Example Sketch - Limit_And_Conversion_Alert");
  
  /* In the default mode the limit interrupt flag will be deleted after the next measurement within limits. 
    With enableAltertLatch(), the flag will have to be deleted with readAndClearFlags(). 
  */
  ina226.enableAlertLatch();
  
  /* Set the alert type and the limit
      * Mode *              * Description *           * limit unit *
    INA226_SHUNT_OVER     Shunt Voltage over limit          mV
    INA226_SHUNT_UNDER    Shunt Voltage under limit         mV
    INA226_CURRENT_OVER   Current over limit                mA
    INA226_CURRENT_UNDER  Current under limit               mA
    INA226_BUS_OVER       Bus Voltage over limit            V
    INA226_BUS_UNDER      Bus Voltage under limit           V
    INA226_POWER_OVER     Power over limit                  mW
  */
  ina226.setAlertType(INA226_CURRENT_UNDER, 3.0); // alert, if current is below 3.0 mA
  ina226.enableConvReadyAlert(); // In this example we also enable the conversion ready alert interrupt
 
  attachInterrupt(digitalPinToInterrupt(interruptPin), alert, FALLING);
}

void loop() {
  static unsigned long lastLimitAlert = 0;
  if(event){
    ina226.readAndClearFlags();
    if(ina226.convAlert){
      Serial.println("Conversion Alert!!!!");
      displayResults();
    }
    
  /* 
    The limit alert is fired after every single measurement. I does not
    wait until the averaged value is ready. Therefore, I reduce the number
    of outputs. 
  */
    if(ina226.limitAlert){
      if (millis() - lastLimitAlert >= 1000) {
        Serial.println("Limit Alert !!!!");
        lastLimitAlert = millis();
      }
    }
    
    event = false;
    attachInterrupt(digitalPinToInterrupt(interruptPin), alert, FALLING); 
    ina226.readAndClearFlags(); 
  }  
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
  detachInterrupt(digitalPinToInterrupt(interruptPin));
}
