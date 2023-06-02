# Fiscit Remote Monitoring

## [Fiscit Remote Monitoring](https://discord.gg/tv3jbJW3RX) official discord server

## [Fiscit Web UI Source](https://github.com/GalaxyVOID/FRM-s-WebUI-Source)

This is a beta version of the mod, which may have some bugs. If you find any issues, please contact me via my [**discord**](https://discordapp.com/users/56086820147953664).

Have you ever left your base to explore or work on another project and wondered if the power at home was still good? Do you want a small gadget or website that allows you to remotely monitor your power on your dedicated server? Fiscit Remote Monitoring is here for you!

**Note:** This mod is not required to be on everyone's mod list. You may run this mod alone or with your group.

## Third-party Support/Apps

Please note that I did not develop these tools, and thus cannot be of much assistance. I would advise contacting the developer/engineer of the tool if no solution can be found: All references here are by permission of the author.

- [Ficsit Remote Monitoring Companion App](https://github.com/AP-Hunt/FicsitRemoteMonitoringCompanion/releases/) written by Badger.
- [Satisfactory Efficiency Terminal](https://github.com/kikookraft/satisfactory-efficiency-terminal) designed and engineered by u/kikookraft.
- [Ficsit Remote Monitoring Companion Bundle](https://github.com/featheredtoast/satisfactory-monitoring) written by FeatheredToast
- [Ficsit Remote Monitoring Dashboard](https://github.com/Jonathan-Hofmann/ficsit-remote-monitoring-dasboard) written by Arfla and JDesignV2

## How to get started

1. Read the documentation on [https://docs.ficsit.app/ficsitremotemonitoring/latest/](https://docs.ficsit.app/ficsitremotemonitoring/latest/).
2. The web server may be started/stopped by using `/frmweb {start/stop}` in the chat window.
3. The serial communication may be started/stopped by using `/frmserial {start/stop}` in the chat window.
4. For Serial, specify the Serial port associated (Windows Device Manager helps with this), then set the Baud Rate.
   - Due to the library used, only the baud rate can be changed; you must use 8-N-1 (8 bits, No parity, 1 stop bit).

## Features

- Power Monitoring on a selected circuit.
- Battery status, charge, capacity, time until out of power.
- Output Serial Devices like RaspberryPi, Arduino, and other RS-232 devices.
- Input support to remotely reset fuses or issue other commands.
- JSON Support for output to allow updates and additional input/output without disrupting current setups.
- Web Server Support, complete with Power Information as well as Real-Time Tracking of Player (not multiplayer tested, but should work).

## WIP / Future Features

- Input functionality, i.e., turn on/off switches.

## Known Bugs

See [Remote Monitoring Github Issues](https://github.com/porisius/FicsitRemoteMonitoring/issues).

## Feedback

If you have any options or features that you want, @ me in the modding Discord.

## Permissions granted

You are permitted to create your own UIs for either Web and/or Serial and display them where appropriate (i.e., Discord, Reddit, etc).

Please keep it in accordance with the social media rules and ToS.

## Special Thanks

- [**Feyko**](https://discordapp.com/users/227473074616795137): Support and guidance to a noob Unreal Engine modder.
- [**Robb**](https://discordapp.com/users/187385442549628928): Answering my dumb questions.
- [**Vilsol**](https://discordapp.com/users/135134753534771201): Also answering my dumb questions and helping with the documentation system.
- [**Nog**](https://discordapp.com/users/277050857852370944): Answering the dumbest of my questions.
- [**Archengius**](https://discordapp.com/users/163955176313585666): Helping with the UE Garbage Collection Issue.
- [**Deantendo**](https://discordapp.com/users/293484684787056640): Icon/Graphic for FRM.
- [**Andre Aquila**](https://discordapp.com/users/294943551605702667): Production Stats code for FRM (Seriously, that would have taken me forever to develop).
- [**Badger**](https://discordapp.com/users/186896287856197633): For the FRM Companion App.
- [**BLAndrew575**](https://discordapp.com/users/509759568037937152): For giving me a crazy world to brutally stress test the getFactory caching function.
- [**GalaxyVOID**](https://discordapp.com/users/212243828831289344): Contributions to FRM's native web UI.
- [**FeatheredToast**](https://discordapp.com/users/130401633564753920): Finding and helping resolve the dumb things I did.

## Thank you to all who have contributed and helped make Fiscit Remote Monitoring possible
