<h3> This is an beta version, yes, I know it is unpolished and a little buggy. If you find one, @Darthporisius in the SMM discord or send me a DM. Screenshots are appreciated. Also, if you have questions, feel free to also ping me in the #helping-using-mods.</h3>
<p>Gone away from the base to explore or work on another project?
<p>Wondered if the power at home was good, but didn't have a power pole nearby?
<p>Want a small gadget or website that you could monitor your power on your dedicated server (SML Soon™) remotely?

<p><strong>Then Fiscit Remote Monitoring is here for you!!!</strong>

<ul><h3>Webhook notifications requires Disc-IT installed</h3></ul>
<ul><h3>Serial/Arduino Support requires ArduinoKit installed.</h3> Prior to FRM 0.8.0, UE4Duino is used.</ul>

This mod is <b><u>NOT REQUIRED</u></b> to be on everyone's mod list. You may run this mod alone or with your group.

<h3>Third-party Support/Apps:</h3> Please note that I did not develop these tools, and thus cannot be of much assistance. I would advise contacting the developer/engineer of the tool if no solution can be found: All references here are by permission of the author.

<p><a href="https://github.com/AP-Hunt/FicsitRemoteMonitoringCompanion/releases/">Ficsit Remote Monitoring Companion App</a> written by Badger</p>

<p><a href="https://github.com/kikookraft/satisfactory-efficiency-terminal">Satisfactory Efficiency Terminal</a> designed and engineered by <a href="https://www.reddit.com/user/kikookraft/">u/kikookraft</a>

<p><h3>How to get started:</h3>
<li>I would strongly recommend reading the documentation on <a href="https://docs.ficsit.app/ficsitremotemonitoring/latest/">https://docs.ficsit.app/ficsitremotemonitoring/latest/</a>
<li>The web server may be started/stopped by using "/frmweb {start/stop}" in the chat window.
<li>The serial communication may be started/stopped by using "/frmserial {start/stop}" in the chat window.
<li>For Serial, specify the Serial port associated (Windows Device Manager helps with this), then set the Baud Rate
<ul>
<li>Due to the library used, only the baud rate can be change, you must use 8-N-1 (8 bits, No parity, 1 stop bit) 
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

<p><h3>Known Bugs:</h3>
See: <a href="https://github.com/porisius/FicsitRemoteMonitoring/issues">Remote Monitoring Github Issues</a>

<p><h3>Feedback:</h3>
Do you have an option or feature that you want? @ me in the modding Discord.

<p><h3>Permissions granted:</h3>
You are permitted to create your own UIs for either Web and/or Serial and display them where appropriate (i.e. Discord (Official Satisfactory, Modded Satisfactory, Reddit, etc). 

I would strongly recommend the #work-in-progress section, but you are free to post where appropriate. 

Please keep it in accordance with the social media rules and ToS.

<p><h3>Special Thanks:</h3>
<li><b>Feyko</b>: Support and guidance to a noob Unreal Engine modder
<li><b>Robb</b>: Answering my dumb questions
<li><b>Vilsol</b>: Also answering my dumb questions and helping with the documentation system
<li><b>Nog</b>: Answering the dumbest of my questions
<li><b>Archengius</b>: Helping with the UE Garbage Collection Issue
<li><b>Deantendo</b>: Icon/Graphic for FRM
<li><b>Andre Aquila</b>: Production Stats code for FRM (Seriously, that would have taken me forever to develop.)
<li><b>Badger</b>: For the FRM Companion App.
<li><b>BLAndrew575</b>: For giving me a crazy world to brutally stress test the getFactory caching function
<li><b>GalaxyWolfYT</b>: Contributions to FRM's native web UI
<li><b>FeatheredToast</b>: Finding and helping resolve the dumb things I did dumb
<li>and to the rest of the Satisfactory Modding Discord for motivating me and letting me vent as I go through my day and also develop this mod.