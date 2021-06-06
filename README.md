<h3> This is an beta version, yes, I know it is unpolished and a little buggy. If you find one, @Darthporisius in the SMM discord or send me a DM. Screenshots are appreciated. Also, if you have questions, feel free to also ping me in the #helping-using-mods.</h3>
<p>Gone away from the base to explore or work on another project?
<p>Wondered if the power at home was good, but didn't have a power pole nearby?
<p>Want a small gadget or website that you could monitor your power on your dedicated server (CSS's Soon ™) remotely?

<p><strong>Then FiscIt Remote Monitoring is here for you!!!</strong>

<p><h3>How to get started:</h3>
<li>I would strongly recommend reading the documentation on <a href="https://docs.ficsit.app/ficsitremotemonitoring/latest/">https://docs.ficsit.app/ficsitremotemonitoring/latest/</a>
<li>The web server may be started/stopped by using "/frmweb {start/stop}" in the chat window.
<li>The serial communication may be started/stopped by using "/frmserial {start/stop}" in the chat window.
<li>For Serial, specify the Serial port associated (Windows Device Manager helps with this), then set the Baud Rate
<ul>
<li>Due to the library used, only the baud rate can be change, you must use 8-N-1 (8 bits, no parity, 1 stop bit) 
</ul>

<p><h3>Features:</h3>
<li>Power Monitoring on a selected circuit
<li>Battery status, charge, capacity, time until out of power
<li>Output Serial Devices like RaspberryPi, Arduino, and other RS-232 devices
<li>Input support to remotely reset fuses or issue other commands
<li>JSON Support for output to allow updates and additional input/output without disrupting current setups
<li>Web Server Support, complete with Power Information as well as Real-Time Tracking of Player (not multiplayer tested, but should work)

<p><h3>WIP / Future Features:</h3>
<li>Input functionality, (i.e. allow button to reset fuse, or turn on/off switches)
<li>Better design of Web UI, #help-wanted
<li>Reverse-proxy support for multiplayer

<p><h3>Known Bugs:</h3>
<li>Issues with the Player GeoJSON Coordinates for anyone but the host. Investigations on-going.

<p><h3>Feedback:</h3>
Do you have an option or feature that you want? @ me in the modding Discord.

<h3>Third-party Support/Apps:</h3> Please note that I did not develop this tool, and thus cannot be of much assistance. I would advise contacting the developer of the tool if no solution can be found:

<p><a href="https://github.com/AP-Hunt/FicsitRemoteMonitoringCompanion/releases/">Ficsit Remote Monitoring Companion App</a> written by Badger</p>

<p><h3>Permissions granted:</h3>
You are permitted to create your own UIs for either Web and/or Serial and display them on the Official Satisfactory Modding Discord (further known as Discord Server). 

I would strongly recommend the #work-in-progress section, but you are free to post where appropriate. 

Please keep it in accordance with the Discord Server and Discord's rule and ToS.

<p><h3>Special Thanks:</h3>
<li><b>Feyko</b>: Support and guidance to a noob Unreal Engine modder
<li><b>Robb</b>: Answering my dumb questions
<li><b>Vilsol</b>: Also answering my dumb questions and helping with the documentation system
<li><b>Nog</b>: Answering the dumbest of my questions
<li><b>Archengius</b>: Helping with the UE Garbage Collection Issue that is most likely resolved
<li><b>Deantendo</b>: Icon/Graphic for FRM
<li><b>Andre Aquila</b>: Production Stats code for FRM (Seriously, that would have taken me forever to develop.)
<li><b>Badger</b>: For lending assistance in making a decent UI in the form of the Companion App.
<li>and to the rest of the Satisfactory Modding Discord for motivating me and letting me vent as I go through my day and also develop this mod.

<h3>Serial JSON Output Example:</h3>
<img src="https://github.com/porisius/RS232_SF_Project/blob/main/images/smr-output-example.png?raw=true">
