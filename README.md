# QuickWater / CannaWatery
Arduino (tested on 328 µC) compatible Hard- and Software to get any watery, to grow plants, done. 

<br>

**CannaWatery** is made to measure, monitor and regulate all elementary water values.  
 All of the following values having their own ports on the µC to do action if the value is higher or lower than your configuration allows. High/low values, timings and time-outs are free configurable.
- pH
- EC (conductivity)
- Temperature
- Redox quality
- Oxygen saturation
- Reservoir level

<br>

**CannaWatery** is functioning without the need of any network connections. All configuration can be made via an onBoard running HMI with support for VT-100 compatible Terminals. So, Linux, macOS and since a short while even Windows, having everything on board to manage and view **CannaWatery**.
Untested right now, but terminal-apps for mobile phones will be able to controls CannaWatery, too - as long they can manage VT-100 sequences!

- once set, it will reboot after power losses safe back into the right state without the need of manual actions. Even if CannaWatery is in ModBUS-Slave-Mode and the master is dead - CannaWatery will run your predefined emergency settings until the master is back.

- All water specific sensors/modules from <a href="https://atlas-scientific.com/"> Atlas-Scientific</a> are supported.

- On a 328-Arduino, **CannaWatery** can manage up to 10 of those Atlas sensors/modules. One 4-point level-sensor is on board of a 328.

<br>

**CannaWatery** has two siblings...
- <a href="https://github.com/PitWD/QuickTimer"> CannaClocky</a> to get any timings, to grow plants, done. As you can read in another repository from me: <a href="https://github.com/CannaParts/LetsGrowSmart/blob/main/FastVegaFlowerLowPower.md">To grow Cannabis based on 24h cycles is wasted time and energy.</a> To do such timings - you will need CannaClocky.

- <a href="https://github.com/PitWD/QuickAir"> CannaBreezy</a> to measure and regulate °C, humidity, CO2, O2. *All climate/air specific sensors/modules from <a href="https://atlas-scientific.com/"> Atlas-Scientific</a> are supported.*

All three together are the base to have a very solid control over your grow. Their ability to function as ModBUS slaves makes it possible to let (multiple of) them act in a bigger context under control of more complex controllers, HMIs and 3rd party products.

<br>

The **loop screen** during runtime. Visualization of all probes and their averaged value.  
The states "TooLow", "Low", "OK", "High", "TooHigh" get visualized in different colors. You're absolutely free to define of what "(Too)High" and "(Too)Low" is, so that it will fit best to your needs. In the last line you see, that the level action-port for "(Too)Low" is active, cause the timeout for "level" expired and a action-time is set for that case (see later screenshots).  
*The three lines with times are an debug-feature and will get removed soon from this screen.*
![Loop Screen](/images/Auswahl_001.png)

<br>

The *primary* **main settings menu**. This menu gives you access on generic settings and instaled probes/modules.  
You can individually setup the color and style of the menu-key and the dimmed text style, to have a nice appearance fitting to your terminal/desktop style. The screenshots are made with an "solarized terminal" just the green is a little greener than from the original solarized green.  
![Primary Main Screen](/images/Auswahl_002.png)
Key color changed:
![Primary Main Screen](/images/Auswahl_003.png)
Dim Color changed:
![Primary Main Screen](/images/Auswahl_004.png)

<br>

The *secondary* **main settings menu**. All probes/modules are selected.
![Secondary Main Screen](/images/Auswahl_005.png)

<br>

The **probes/modules menu**. One of two conductivity probes is selected.
![Probes Screen 1 selected](/images/Auswahl_006.png)
With both probes selected.
![Probes Screen 2 selected](/images/Auswahl_007.png)

<br>

The **calibration menu**. Two conductivity probes are selected.
![Calibration Screen](/images/Auswahl_008.png)
During the calibration for the dry-value.
![Calibration Screen Dry](/images/Auswahl_009.png)
During the calibration for the 2nd Value in a 2-Point calibration.
![Calibration Screen 2nd Point](/images/Auswahl_010.png)

<br>

The **Set Values Menu**. Here you define the values for "(Too)Low" and "(Too)High".
*On a 328 you can define up to three independent settings*
![Set Timings Menu](/images/Auswahl_011.png)

<br>

The **Set Timings Menu**. Here you define how the action-times are if a state is for at least the delay-time "TooLow", "Low", "High" or "TooHigh".  
*On a 328 you can define up to three independent settings*
![Set Timings Menu](/images/Auswahl_012.png)

<br>

The **Manual Menu**. Here you have an manual access to the action-ports. But you also can define up to 4 settings you can run in two different modes half-automatic. This way you can predefine to setup water + acid + nutrition for vega, flower, flush...
![Manual Menu](/images/Auswahl_013.png)

<br>

The **Manual Run Screen** is visualizing the process of running a manual setting (actually in "distributed-mode").
![Manual Run Menu](/images/Auswahl_014.png)

<br>

I can't declare this project actually as that useable like I do for <a href="https://github.com/PitWD/QuickTimer"> CannaClocky</a>. The communication-part with the Atlas-Scientific modules and visualization of the read values is 100% usable - some prototypes are working perfect since nearly three months. Just the actions are not that hard tested - but first real running prototypes exist and working pretty fine, too.  
Some planned extensions will break the actual structure of the internal eeprom - this will break the user made settings. There is actually no safe "fuse" to prevent that re-flashed µC's with a then broken eeprom will boot in a usable state.

