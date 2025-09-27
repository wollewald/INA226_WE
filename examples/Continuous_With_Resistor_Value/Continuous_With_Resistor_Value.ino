/***************************************************************************
* Example sketch for the INA226_WE library
*
* This sketch is based on the continuous mode example but uses the function setResistorRange to set a different resistor value. 
*
* More information on this board can be found here: https://github.com/generationmake/stromsensor6mm
*
* More information on the INA226_WE library:
* https://wolles-elektronikkiste.de/en/ina226-current-and-power-sensor (English)
* https://wolles-elektronikkiste.de/ina226 (German)
* 
***************************************************************************/
#include <Wire.h>
#include <INA226_WE.h>
#define I2C_ADDRESS 0x40

/* There are several ways to create your INA226 object:
 * INA226_WE ina226 = INA226_WE(); -> uses I2C Address = 0x40 / Wire
 * INA226_WE ina226 = INA226_WE(I2C_ADDRESS);   
 * INA226_WE ina226 = INA226_WE(&Wire); -> uses I2C_ADDRESS = 0x40, pass any Wire Object
 * INA226_WE ina226 = INA226_WE(&Wire, I2C_ADDRESS); 
 */
INA226_WE ina226 = INA226_WE(I2C_ADDRESS);

void setup() {
  Serial.begin(115200);
  while(!Serial); // wait until serial comes up on Arduino Leonardo or MKR WiFi 1010
  Wire.begin();
  ina226.init();

  /* Set Number of measurements for shunt and bus voltage which shall be averaged
      * Mode *           * Number of samples *
    INA226_AVERAGE_1            1 (default)
    INA226_AVERAGE_4            4
    INA226_AVERAGE_16          16
    INA226_AVERAGE_64          64
    INA226_AVERAGE_128        128
    INA226_AVERAGE_256        256
    INA226_AVERAGE_512        512
    INA226_AVERAGE_1024      1024
    */
  //ina226.setAverage(INA226_AVERAGE_16); // choose mode and uncomment for change of default

  /* Set conversion time in microseconds
    One set of shunt and bus voltage conversion will take: 
    number of samples to be averaged x conversion time x 2
     
    * Mode *                  * conversion time *
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
  //ina226.setMeasureMode(INA226_CONTINUOUS); // choose mode and uncomment for change of default
  
  /* Set Resistor and Current Range
    Most INA226 modules use a 0.1 ohms shunt. If you have a board with a different shunt you need 
    to set it with setResistorRange(). You pass the shunt value in ohms. The maximum current you 
    can measure is Imax [A] = 0.0819175 / shuntSize. 
    If you only expect currents lower than this Imax, you can also pass the expected Imax as a 
    second parameter. The resolution is determined by the shunt register LSB (2.5 µV). However,
    passing a smaller Imax may result in a little higher resolution when the current is calculated.
  */
  //ina226.setResistorRange(0.005,10.0); // Example: shunt is 5 mOhm and expected Imax is 10 A
  ina226.setResistorRange(0.005); // Shunt is 5 mOhm, Imax is 16.3835 by default
  
  /* If the current values delivered by the INA226 differ by a constant factor
    from values obtained with calibrated equipment you can define a correction factor.
    Correction factor = current measured with calibrated device / current measured by INA226
    Be aware that Imax depends on the real shunt size.
  */
  // ina226.setCorrectionFactor(0.95);
  
  Serial.println("INA226 Current Sensor Example Sketch - Continuous");
  
  ina226.waitUntilConversionCompleted(); //if you comment this line the first data might be zero
}

void loop() {
  float shuntVoltage_mV = 0.0;
  float loadVoltage_V = 0.0;
  float busVoltage_V = 0.0;
  float current_mA = 0.0;
  float power_mW = 0.0; 

  ina226.readAndClearFlags();
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
  
  delay(3000);
}
