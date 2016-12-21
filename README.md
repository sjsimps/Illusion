# Illusion - A Dynamic Audio Visualizer
![](https://github.com/sjsimps/Illusion/blob/master/example.gif)

Illusion is a dynamic audio visualizer tool for Ubuntu.
Illusion monitors sound from any source and edits a set
of images in real time in accordance to the noise.

##Usage:

###Build and run
In the Illusion directory, make and run the project:
```
make illusion
./illusion
```
If building fails, ensure you have all the required libraries installed.

###Selecting a Recording Source
Illusion monitors sound on a system using PulseAudio.
A sound source may be selected using the PulseAudio control GUI:

```
To open the GUI:
> pavucontrol
```
![](https://github.com/sjsimps/Illusion/blob/master/pavucontrol.png)

Select a recording source using the "record from" drop down menu.

###Setting an Image
Set what images Illusion uses by adding them into the Illusion/Images/ folder.

##Libraries Used:
* PulseAudio: libpulse-dev
* SDL 2 and SDL Image: libsdl2-dev, libsdl2-image-dev



