# Fiscit Remote Monitoring

This is a beta version of the mod, which may have some bugs. If you find any issues, please contact the developer.

Have you ever left your base to explore or work on another project and wondered if the power at home was still good? Do you want a small gadget or website that allows you to remotely monitor your power on your dedicated server? Fiscit Remote Monitoring is here for you!

**Note:** This mod is not required to be on everyone's mod list. You may run this mod alone or with your group.

## Third-party Support/Apps:

Please note that I did not develop these tools, and thus cannot be of much assistance. I would advise contacting the developer/engineer of the tool if no solution can be found: All references here are by permission of the author.

- [Ficsit Remote Monitoring Companion App](https://github.com/AP-Hunt/FicsitRemoteMonitoringCompanion/releases/) written by Badger.
- [Satisfactory Efficiency Terminal](https://github.com/kikookraft/satisfactory-efficiency-terminal) designed and engineered by u/kikookraft.

## How to get started:

1. Read the documentation on [https://docs.ficsit.app/ficsitremotemonitoring/latest/](https://docs.ficsit.app/ficsitremotemonitoring/latest/).
2. The web server may be started/stopped by using `/frmweb {start/stop}` in the chat window.
3. The serial communication may be started/stopped by using `/frmserial {start/stop}` in the chat window.
4. For Serial, specify the Serial port associated (Windows Device Manager helps with this), then set the Baud Rate.
   - Due to the library used, only the baud rate can be changed; you must use 8-N-1 (8 bits, No parity, 1 stop bit).

## Features:

- Power Monitoring on a selected circuit.
- Battery status, charge, capacity, time until out of power.
- Output Serial Devices like RaspberryPi, Arduino, and other RS-232 devices.
- Input support to remotely reset fuses or issue other commands.
- JSON Support for output to allow updates and additional input/output without disrupting current setups.
- Web Server Support, complete with Power Information as well as Real-Time Tracking of Player (not multiplayer tested, but should work).

## WIP / Future Features:

- Input functionality, i.e., turn on/off switches.

## Known Bugs:

See [Remote Monitoring Github Issues](https://github.com/porisius/FicsitRemoteMonitoring/issues).

## Feedback:

If you have any options or features that you want, @ me in the modding Discord.

## Permissions granted:

You are permitted to create your own UIs for either Web and/or Serial and display them where appropriate (i.e., Discord, Reddit, etc).

Please keep it in accordance with the social media rules and ToS.

## Special Thanks:

- Feyko: Support and guidance to a noob Unreal Engine modder.
- Robb: Answering my dumb questions.
- Vilsol: Also answering my dumb questions and helping with the documentation system.
- Nog: Answering the dumbest of my questions.
- Archengius: Helping with the UE Garbage Collection Issue.
- Deantendo: Icon/Graphic for FRM.
- Andre Aquila: Production Stats code for FRM (Seriously, that would have taken me forever to develop).
- Badger: For the FRM Companion App.
- BLAndrew575: For giving me a crazy world to brutally stress test the getFactory caching function.
- GalaxyVOID: Contributions to FRM's native web UI.
- FeatheredToast: Finding and helping resolve the dumb things I did.

## Thank you to all who have contributed and helped make Fiscit Remote Monitoring possible!