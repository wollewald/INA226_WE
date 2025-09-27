/*****************************************************************
* This is a library for the INA226 Current and Power Sensor Module
*
* You'll find an example which should enable you to use the library. 
*
* You are free to use it, change it or build on it. In case you like 
* it, it would be cool if you give it a star.
* 
* If you find bugs, please inform me!
* 
* Written by Wolfgang (Wolle) Ewald
* https://wolles-elektronikkiste.de/en/ina226-current-and-power-sensor (English)
* https://wolles-elektronikkiste.de/ina226 (German)
*
******************************************************************/

#include "INA226_WE.h"

bool INA226_WE::init(){
    _wire->beginTransmission(i2cAddress);
    if(_wire->endTransmission()){
        return 0;
    }
    reset_INA226();
    calVal = 2048; // default
    writeRegister(INA226_CAL_REG, calVal);
    setAverage(INA226_AVERAGE_1);
    setConversionTime(INA226_CONV_TIME_1100);
    setMeasureMode(INA226_CONTINUOUS);
    currentDivider_mA = 40.0;
    pwrMultiplier_mW = 0.625;
    convAlert = false;
    limitAlert = false;
    corrFactor = 1.0;
    i2cErrorCode = 0;
    return 1;
}

void INA226_WE::reset_INA226(){
    writeRegister(INA226_CONF_REG, INA226_RST); 
}

void INA226_WE::setCorrectionFactor(float corr){
    corrFactor = corr;
    uint16_t calValCorrected = static_cast<uint16_t>(calVal * corrFactor);
    writeRegister(INA226_CAL_REG, calValCorrected);
}

void INA226_WE::setAverage(INA226_AVERAGES averages){
    deviceAverages = averages;
    uint16_t currentConfReg = readRegister(INA226_CONF_REG);
    currentConfReg &= ~(0x0E00);  
    currentConfReg |= deviceAverages;
    writeRegister(INA226_CONF_REG, currentConfReg);
}

void INA226_WE::setConversionTime(INA226_CONV_TIME shuntConvTime, INA226_CONV_TIME busConvTime){
    uint16_t currentConfReg = readRegister(INA226_CONF_REG);
    currentConfReg &= ~(0x01C0);  
    currentConfReg &= ~(0x0038);
    uint16_t convMask = (static_cast<uint16_t>(shuntConvTime))<<3;
    currentConfReg |= convMask;
    convMask = busConvTime<<6;
    currentConfReg |= convMask;
    writeRegister(INA226_CONF_REG, currentConfReg);
}

void INA226_WE::setConversionTime(INA226_CONV_TIME convTime){
    setConversionTime(convTime, convTime);
}

void INA226_WE::setMeasureMode(INA226_MEASURE_MODE mode){
    deviceMeasureMode = mode;
    uint16_t currentConfReg = readRegister(INA226_CONF_REG);
    currentConfReg &= ~(0x0007);
    currentConfReg |= deviceMeasureMode;
    writeRegister(INA226_CONF_REG, currentConfReg);
}

/* DEPRECATED
void INA226_WE::setCurrentRange(INA226_CURRENT_RANGE range){ 
    deviceCurrentRange = range;      
}
*/

//set resistor and current range independant. resistor value in ohm, current range in A
void INA226_WE::setResistorRange(float resistor, float current_range){
    if(current_range < 0) {
        current_range = 0.0819175/resistor;
    }

    float current_LSB=current_range/32768.0;

    calVal = 0.00512/(current_LSB*resistor);
    currentDivider_mA = 0.001/current_LSB;
    pwrMultiplier_mW = 1000.0*25.0*current_LSB;

    writeRegister(INA226_CAL_REG, calVal);          
}

float INA226_WE::getShuntVoltage_V(){
    int16_t val;
    val = static_cast<int16_t>(readRegister(INA226_SHUNT_REG));
    return (val * 0.0000025 * corrFactor);  
}

float INA226_WE::getShuntVoltage_mV(){
    int16_t val;
    val = static_cast<int16_t>(readRegister(INA226_SHUNT_REG));
    return (val * 0.0025 * corrFactor); 
}

float INA226_WE::getBusVoltage_V(){
    uint16_t val;
    val = readRegister(INA226_BUS_REG);
    return (val * 0.00125);
}

float INA226_WE::getCurrent_mA(){
    int16_t val;
    val = static_cast<int16_t>(readRegister(INA226_CURRENT_REG));
    return (val / currentDivider_mA);
}

float INA226_WE::getCurrent_A() {
    return (getCurrent_mA()/1000);
}

float INA226_WE::getBusPower(){
    uint16_t val;
    val = readRegister(INA226_PWR_REG);
    return (val * pwrMultiplier_mW);
}

void INA226_WE::startSingleMeasurement(){
    uint16_t val = readRegister(INA226_MASK_EN_REG); // clears CNVR (Conversion Ready) Flag
    val = readRegister(INA226_CONF_REG);
    writeRegister(INA226_CONF_REG, val);        // Starts conversion
    uint16_t convReady = 0x0000;
    unsigned long convStart = millis();
    while(!convReady && ((millis()-convStart) < 2000)){
        convReady = ((readRegister(INA226_MASK_EN_REG)) & 0x0008); // checks if sampling is completed
    }
}

// Don't wait for conversion to complete
void INA226_WE::startSingleMeasurementNoWait(){
    uint16_t val = readRegister(INA226_MASK_EN_REG); // clears CNVR (Conversion Ready) Flag
    val = readRegister(INA226_CONF_REG);
    writeRegister(INA226_CONF_REG, val);        // Starts conversion
}

void INA226_WE::powerDown(){
    confRegCopy = readRegister(INA226_CONF_REG);
    setMeasureMode(INA226_POWER_DOWN);
}

void INA226_WE::powerUp(){
    writeRegister(INA226_CONF_REG, confRegCopy);
    delayMicroseconds(40);  
}

// Returns 1 if conversion is still ongoing
bool INA226_WE::isBusy(){
    return (!(readRegister(INA226_MASK_EN_REG) &0x0008));
}
    
void INA226_WE::waitUntilConversionCompleted(){
    readRegister(INA226_MASK_EN_REG); // clears CNVR (Conversion Ready) Flag
    uint16_t convReady = 0x0000;
    while(!convReady){
        convReady = ((readRegister(INA226_MASK_EN_REG)) & 0x0008); // checks if sampling is completed
    }
}

void INA226_WE::setAlertPinActiveHigh(){
    uint16_t val = readRegister(INA226_MASK_EN_REG);
    val |= 0x0002;
    writeRegister(INA226_MASK_EN_REG, val);
}

void INA226_WE::enableAlertLatch(){
    uint16_t val = readRegister(INA226_MASK_EN_REG);
    val |= 0x0001;
    writeRegister(INA226_MASK_EN_REG, val);
}

void INA226_WE::enableConvReadyAlert(){
    uint16_t val = readRegister(INA226_MASK_EN_REG);
    val |= 0x0400;
    writeRegister(INA226_MASK_EN_REG, val);
}
    
void INA226_WE::setAlertType(INA226_ALERT_TYPE type, float limit){
    deviceAlertType = type;
    uint16_t alertLimit = 0;
    
    switch(deviceAlertType){
        case INA226_SHUNT_OVER:
            alertLimit = limit * 400;           
            break;
        case INA226_SHUNT_UNDER:
            alertLimit = limit * 400; 
            break;
        case INA226_CURRENT_OVER:
            deviceAlertType = INA226_SHUNT_OVER;
            alertLimit = limit * 2048 * currentDivider_mA / calVal;
            break;
        case INA226_CURRENT_UNDER:
            deviceAlertType = INA226_SHUNT_UNDER;
            alertLimit = limit * 2048 * currentDivider_mA / calVal;
            break;
        case INA226_BUS_OVER:
            alertLimit = limit * 800;
            break;
        case INA226_BUS_UNDER:
            alertLimit = limit * 800;
            break;
        case INA226_POWER_OVER:
            alertLimit = limit / pwrMultiplier_mW;
            break;
    }
    
    writeRegister(INA226_ALERT_LIMIT_REG, alertLimit);
    
    uint16_t value = readRegister(INA226_MASK_EN_REG);
    value &= ~(0xF800);
    value |= deviceAlertType;
    writeRegister(INA226_MASK_EN_REG, value);
    
}

void INA226_WE::readAndClearFlags(){
    uint16_t value = readRegister(INA226_MASK_EN_REG);
    overflow = (value>>2) & 0x0001;
    convAlert = (value>>3) & 0x0001;
    limitAlert = (value>>4) & 0x0001;
}

uint8_t INA226_WE::getI2cErrorCode(){
    return i2cErrorCode;
}
    

/************************************************ 
    private functions
*************************************************/

void INA226_WE::writeRegister(uint8_t reg, uint16_t val){
  _wire->beginTransmission(i2cAddress);
  uint8_t lVal = val & 255;
  uint8_t hVal = val >> 8;
  _wire->write(reg);
  _wire->write(hVal);
  _wire->write(lVal);
  _wire->endTransmission();
}
  
uint16_t INA226_WE::readRegister(uint8_t reg){
  uint8_t MSByte = 0, LSByte = 0;
  uint16_t regValue = 0;
  _wire->beginTransmission(i2cAddress);
  _wire->write(reg);
  i2cErrorCode = _wire->endTransmission(false);
  _wire->requestFrom(static_cast<uint8_t>(i2cAddress),static_cast<uint8_t>(2));
  if(_wire->available()){
    MSByte = _wire->read();
    LSByte = _wire->read();
  }
  regValue = (MSByte<<8) + LSByte;
  return regValue;
}
    


