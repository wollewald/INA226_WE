# INA226_WE
An Arduino library for the INA226 current and power sensor module using a shunt of 0.1 Ohm. You can also use it for the INA226 IC and a resistor of your choice (example: Continuous_With_Resistor_Value.ino).

It allows to set current ranges, number of samples per measurement, conversion ranges. You can chose between continous, triggered and power down mode. Conversion ready and limit alert modes are also implemented. 

I attached a list of public functions and a fritzing scheme for the example sketches.

If you work through the examples I recommend to do it in the order of Continous -> Triggered -> PowerDown -> Continous_Alert_Controlled -> Limit_Alert -> Limit_And_Conversion_Alert 

You find further details here:

https://wolles-elektronikkiste.de/ina226  (German)

https://wolles-elektronikkiste.de/en/ina226-current-and-power-sensor  (English)

<h2>New since version 1.2.8: compatibility mode</h2>
<p>When I developed the library I did not choose the best enum names for the measure modes. E.g. "POWER_DOWN" is also used in INA219_WE. If you have problems then uncomment the line:
<code> #define INA226_WE_COMPATIBILITY_MODE_ </code>
in INA226_WE_config.h and use "INA226_POWER_DOWN" instead of "POWER_DOWN". Do the same for "CONTINUOUS" (-> INA226_CONTINUOUS) AND TRIGGERED (-> INA226_TRIGGERED).
</p>

<h2>Quality of small shunt INA226 modules</h2>
<p>I have made mixed experience with INA226 modules which have a shunt smaller than 0.1 ohms. I experienced significant deviations from expected values. The reason is that some modules have poor traces and connections which result in an unwanted extra resistance. The most extreme I tested had an R010 shunt (= 0.01 ohms), but showed a resistance of 0.021 ohms. Accordingly, the measured values were more than double compared to the real currents. I short-circuit the shunt (see photo below), so that, theoretically, I should not measure any shunt voltages anymore. But I still measured shunt voltages and currents and these represented exactly the deviations my experiments before I short-cut the shunt.</p>

![20220816_114858](https://user-images.githubusercontent.com/41305162/184959284-186a5e20-1ff5-4c46-a6d9-7c0c68575f14.jpg)


<p>But there are also good modules. The one below has a shunt of 0.002 ohms and the results of my measurements were very close to the expected values.</p>

![INA226_R002](https://user-images.githubusercontent.com/41305162/184960401-85d419fa-e6c0-47f7-8dfe-79863ef0cc8b.png)


