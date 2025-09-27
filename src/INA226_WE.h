/******************************************************************************
 *
 * This is a library for the INA226 Current and Power Sensor Module
 *
 * You'll find several example sketches which should enable you to use the library. 
 *
 * You are free to use it, change it or build on it. In case you like it, it would
 * be cool if you give it a star.
 *
 * If you find bugs, please inform me!
 * 
 * Written by Wolfgang (Wolle) Ewald
 * https://wolles-elektronikkiste.de/en/ina226-current-and-power-sensor (English)
 * https://wolles-elektronikkiste.de/ina226 (German)
 *
 * 
 ******************************************************************************/

#ifndef INA226_WE_H_
#define INA226_WE_H_

#if (ARDUINO >= 100)
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif
#include "INA226_WE_legacy.h"

#include <Wire.h>

typedef enum INA226_AVERAGES{
    INA226_AVERAGE_1       = 0x0000, 
    INA226_AVERAGE_4       = 0x0200,
    INA226_AVERAGE_16      = 0x0400,
    INA226_AVERAGE_64      = 0x0600,
    INA226_AVERAGE_128     = 0x0800,
    INA226_AVERAGE_256     = 0x0A00,
    INA226_AVERAGE_512     = 0x0C00,
    INA226_AVERAGE_1024    = 0x0E00
} averageMode;

typedef enum INA226_CONV_TIME{ // Conversion time in microseconds
    INA226_CONV_TIME_140   = 0b00000000,
    INA226_CONV_TIME_204   = 0b00000001,
    INA226_CONV_TIME_332   = 0b00000010,
    INA226_CONV_TIME_588   = 0b00000011,
    INA226_CONV_TIME_1100  = 0b00000100,
    INA226_CONV_TIME_2116  = 0b00000101,
    INA226_CONV_TIME_4156  = 0b00000110,
    INA226_CONV_TIME_8244  = 0b00000111
} convTime;

typedef enum INA226_MEASURE_MODE{
    INA226_POWER_DOWN,
    INA226_TRIGGERED_CURRENT_ONLY,
    INA226_TRIGGERERD_BUS_ONLY,
    INA226_TRIGGERED,
    INA226_POWER_DOWN_2,
    INA226_CONTINUOUS_CURRENT_ONLY,
    INA226_CONTINUOUS_BUS_ONLY,
    INA226_CONTINUOUS
} INA226_measureMode;


typedef enum INA226_ALERT_TYPE{
    INA226_SHUNT_OVER    = 0x8000,
    INA226_SHUNT_UNDER   = 0x4000,
    INA226_BUS_OVER      = 0x2000,
    INA226_BUS_UNDER     = 0x1000,
    INA226_POWER_OVER    = 0x0800,
    INA226_CURRENT_OVER  = 0xFFFE,
    INA226_CURRENT_UNDER = 0xFFFF,
    //CONV_READY      = 0x0400   not implemented! Use enableConvReadyAlert()
} alertType;

class INA226_WE
{
    public:
        /* registers */
        static constexpr uint8_t INA226_ADDRESS          {0x40};
        static constexpr uint8_t INA226_CONF_REG         {0x00}; //Configuration Register
        static constexpr uint8_t INA226_SHUNT_REG        {0x01}; //Shunt Voltage Register
        static constexpr uint8_t INA226_BUS_REG          {0x02}; //Bus Voltage Register
        static constexpr uint8_t INA226_PWR_REG          {0x03}; //Power Register 
        static constexpr uint8_t INA226_CURRENT_REG      {0x04}; //Current flowing through Shunt
        static constexpr uint8_t INA226_CAL_REG          {0x05}; //Calibration Register 
        static constexpr uint8_t INA226_MASK_EN_REG      {0x06}; //Mask/Enable Register 
        static constexpr uint8_t INA226_ALERT_LIMIT_REG  {0x07}; //Alert Limit Register
        static constexpr uint8_t INA226_MAN_ID_REG       {0xFE}; //Contains Unique Manbufacturer Identification Number
        static constexpr uint8_t INA226_ID_REG           {0xFF}; //Contains unique ID

        /* parameters, flag bits */
        static constexpr uint16_t INA226_RST        {0x8000}; //Reset 
        static constexpr uint16_t INA226_AFF        {0x0010}; //Alert function flag
        static constexpr uint16_t INA226_CVRF       {0x0008}; //Conversion ready flag
        static constexpr uint16_t INA226_OVF        {0x0004}; //Overflow flags
        static constexpr uint16_t INA226_ALERT_POL  {0x0002}; //Alert pin polarity - if set then active-high
        //Latch enable - if set then alert flag remains until mask/enable register is read
        //if not set then flag is cleared after next conversion within limits
        static constexpr uint16_t INA226_LATCH_EN   {0x0001}; 

        // Constructors: if not passed, 0x40 / Wire will be set as address / wire object
        INA226_WE(const int addr = 0x40) : _wire{&Wire}, i2cAddress{addr} {}
        INA226_WE(TwoWire *w, const int addr = 0x40) : _wire{w}, i2cAddress{addr} {}
                
        bool init();
        void reset_INA226();
        void setCorrectionFactor(float corr);
        void setAverage(INA226_AVERAGES averages);
        void setConversionTime(INA226_CONV_TIME convTime);
        void setConversionTime(INA226_CONV_TIME shuntConvTime, INA226_CONV_TIME busConvTime);
        void setMeasureMode(INA226_MEASURE_MODE mode);
        // void setCurrentRange(INA226_CURRENT_RANGE range); // DEPRECATED
        void setResistorRange(float resistor, float range = -1.0);
        float getShuntVoltage_mV();
        float getShuntVoltage_V();
        float getBusVoltage_V();
        float getCurrent_mA();
        float getCurrent_A();
        float getBusPower();
        void startSingleMeasurement();
        void startSingleMeasurementNoWait();
        bool isBusy();
        void powerDown();
        void powerUp(); 
        void waitUntilConversionCompleted();
        void setAlertPinActiveHigh();
        void enableAlertLatch();
        void enableConvReadyAlert();
        void setAlertType(INA226_ALERT_TYPE type, float limit);
        void readAndClearFlags();
        uint8_t getI2cErrorCode();
        bool overflow;
        bool convAlert;
        bool limitAlert;    
    
    protected:
        INA226_AVERAGES deviceAverages;
        INA226_CONV_TIME deviceConvTime;
        INA226_MEASURE_MODE deviceMeasureMode;
        // INA226_CURRENT_RANGE deviceCurrentRange; DEPRECATED
        INA226_ALERT_TYPE deviceAlertType; 
        TwoWire *_wire;
        int i2cAddress;
        uint16_t calVal;
        float corrFactor;
        uint16_t confRegCopy;
        float currentDivider_mA;
        float pwrMultiplier_mW;
        uint8_t i2cErrorCode;
        void writeRegister(uint8_t reg, uint16_t val);
        uint16_t readRegister(uint8_t reg);
};

#endif

