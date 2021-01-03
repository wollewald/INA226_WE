# INA226_WE
An Arduino library for the INA226 current and power sensor module using a shunt of 0.1 Ohm. You can also use it for the INA226 IC and a resistor of your choice (example: Continuous_With_Resistor_Value.ino).

It allows to set current ranges, number of samples per measurement, conversion ranges. You can chose between continous, triggered and power down mode. Conversion ready and limit alert modes are also implemented. 

I attached a list of public functions and a fritzing scheme for the example sketches.

If you work through the examples I recommend to do it in the order of Continous -> Triggered -> PowerDown -> Continous_Alert_Controlled -> Limit_Alert -> Limit_And_Conversion_Alert 

You find further details here:

https://wolles-elektronikkiste.de/ina226  (German)

https://wolles-elektronikkiste.de/en/ina226-current-and-power-sensor  (English)

