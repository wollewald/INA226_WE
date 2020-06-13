/*****************************************
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
* https://wolles-elektronikkiste.de
*
*******************************************/

#include "INA226_WE.h"

INA226_WE::INA226_WE(int addr){
	i2cAddress = addr;
}

INA226_WE::INA226_WE(){
	i2cAddress = 0x40;
}
	
void INA226_WE::init(){	
	reset_INA226();
	setAverage(AVERAGE_1);
	setConversionTime(CONV_TIME_1100);
	setMeasureMode(CONTINOUS);
	setCurrentRange(MA_3200);
	convAlert = false;
	limitAlert = false;
}

void INA226_WE::reset_INA226(){
	writeRegister(INA226_CONF_REG, INA226_RST); 
}

void INA226_WE::setCorrectionFactor(float corr){
	calVal *= corr;
	writeRegister(INA226_CAL_REG, calVal);
}

void INA226_WE::setAverage(INA226_AVERAGES averages){
	deviceAverages = averages;
	uint16_t currentConfReg = readRegister(INA226_CONF_REG);
	currentConfReg &= ~(0x0E00);  
	currentConfReg |= deviceAverages;
	writeRegister(INA226_CONF_REG, currentConfReg);
}

void INA226_WE::setConversionTime(INA226_CONV_TIME convTime){
	deviceConvTime = convTime;
	uint16_t currentConfReg = readRegister(INA226_CONF_REG);
	currentConfReg &= ~(0x01C0);  
	currentConfReg &= ~(0x0038);
	uint16_t convMask = ((uint16_t)deviceConvTime)<<3;
	currentConfReg |= convMask;
	convMask = deviceConvTime<<6;
	currentConfReg |= convMask;
	writeRegister(INA226_CONF_REG, currentConfReg);
}

void INA226_WE::setMeasureMode(INA226_MEASURE_MODE mode){
	deviceMeasureMode = mode;
	uint16_t currentConfReg = readRegister(INA226_CONF_REG);
	currentConfReg &= ~(0x0007);
	currentConfReg |= deviceMeasureMode;
	writeRegister(INA226_CONF_REG, currentConfReg);
}

void INA226_WE::setCurrentRange(INA226_CURRENT_RANGE range){
	deviceCurrentRange = range; 
	
	switch(deviceCurrentRange){
		case MA_400:
			calVal = 2560;
			currentDivider_mA = 50.0;
			pwrMultiplier_mW = 0.5;
			break;
		case MA_800:
			calVal = 1280;
			currentDivider_mA = 25.0;
			pwrMultiplier_mW = 1.0;
			break;
		case MA_1600:
			calVal = 1024;
			currentDivider_mA = 20.0;
			pwrMultiplier_mW = 1.25;
			break;
		case MA_3200:
			calVal = 512;
			currentDivider_mA = 10.0;
			pwrMultiplier_mW = 2.5;
			break;
	}
	
	writeRegister(INA226_CAL_REG, calVal);			
}


float INA226_WE::getShuntVoltage_mV(){
	int16_t val;
	val = (int16_t) readRegister(INA226_SHUNT_REG);
	return (val * 0.0025);	
}

float INA226_WE::getBusVoltage_V(){
	uint16_t val;
	val = readRegister(INA226_BUS_REG);
	return (val * 0.00125);
}

float INA226_WE::getCurrent_mA(){
	int16_t val;
	val = (int16_t)readRegister(INA226_CURRENT_REG);
	return (val / currentDivider_mA);
}

float INA226_WE::getBusPower(){
	uint16_t val;
	val = readRegister(INA226_PWR_REG);
	return (val * pwrMultiplier_mW);
}

void INA226_WE::startSingleMeasurement(){
	uint16_t val = readRegister(INA226_MASK_EN_REG); // clears CNVR (Conversion Ready) Flag
	val = readRegister(INA226_CONF_REG);
	writeRegister(INA226_CONF_REG, val);
	uint16_t convReady = 0x0000;
	while(!convReady){
		convReady = ((readRegister(INA226_MASK_EN_REG)) & 0x0008); // checks if sampling is completed
	}
}

void INA226_WE::powerDown(){
	confRegCopy = readRegister(INA226_CONF_REG);
	setMeasureMode(POWER_DOWN);
}

void INA226_WE::powerUp(){
	writeRegister(INA226_CONF_REG, confRegCopy);
    delayMicroseconds(40);	
}

void INA226_WE::waitUntilConversionCompleted(){
	uint16_t val = readRegister(INA226_MASK_EN_REG); // clears CNVR (Conversion Ready) Flag
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
		case SHUNT_OVER:
			alertLimit = limit * 400;			
			break;
		case SHUNT_UNDER:
			alertLimit = limit * 400; 
			break;
		case CURRENT_OVER:
			deviceAlertType = SHUNT_OVER;
			alertLimit = limit * 2048 * currentDivider_mA / calVal;
			break;
		case CURRENT_UNDER:
			deviceAlertType = SHUNT_UNDER;
			alertLimit = limit * 2048 * currentDivider_mA / calVal;
			break;
		case BUS_OVER:
			alertLimit = limit * 800;
			break;
		case BUS_UNDER:
			alertLimit = limit * 800;
			break;
		case POWER_OVER:
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
	

/************************************************ 
	private functions
*************************************************/

void INA226_WE::writeRegister(uint8_t reg, uint16_t val){
  Wire.beginTransmission(i2cAddress);
  uint8_t lVal = val & 255;
  uint8_t hVal = val >> 8;
  Wire.write(reg);
  Wire.write(hVal);
  Wire.write(lVal);
  Wire.endTransmission();
}
  
uint16_t INA226_WE::readRegister(uint8_t reg){
  uint8_t MSByte, LSByte = 0;
  uint16_t regValue = 0;
  Wire.beginTransmission(i2cAddress);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(i2cAddress,2);
  if(Wire.available()){
    MSByte = Wire.read();
    LSByte = Wire.read();
  }
  regValue = (MSByte<<8) + LSByte;
  return regValue;
}
	


