# Mach1 Spatial Decode Plugin [Unreal Engine]

### Contents:
 - Binary Plugin (Blueprint)
 - Source Plugin (Cpp & Blueprint)

## Instructions:
It is recommended to install the source code plugin as supplied. The binary staged plugins are difficult to install to the engine from version to version so the source code is supplied for use with the UAT.

### General
Copy required plugin (from within `Source` or `Binary`) "Mach1DecodePlugin" directory to your project's `Plugins` directory at the root of your project.
Then add "Mach1 Decode Plugin" through browser class panel.

### Source / Cpp
1. Create cpp project
2. Copy Plugin folder to project folder
3. Check that plugin activated . go to menu "edit" -> "plugins" -> "audio" -> "Mach1 Decode Plugin"
4. In class/objects browser find "Mach1Spatial Actor" and add to scene


For bluprint project,
1. Yon can convert it to cpp : http://allarsblog.com/2015/11/05/Converting-BP-Project-To-CPP/
2. Or you fisrt create base cpp project, make 4 steps
and then copy plugin (with compiled dll) folder to your blueprint project

### Unreal Engine 4 Setup Notes
- Set Project setting to play sound with volume = 0 (Virtualize Silent Sounds)

![UE-VoiceSettings](.readme/UE_Virtualize_Issue.jpg)

### Unreal Engine 5 Setup Notes
- Apply the following settings to all audio files used in the M1SpatialActor object
- Change the `Virtualization Mode` to `Play when Silent`
- Check the `Priority` `Bypass Volume Scale for Priority` to true

![UE5-VoiceSettings](../.readme/UE5-VoiceSettings.png)

## QA:

### Source
QA to final Packaging of project completed on:
4.17, 4.18, 4.19, 4.22, 4.24, 5.0, 5.1

### Binary
QA to final Packaging of project completed on:
5.1

### CONTACT:
Mach1:  
whatsup@mach1.tech
