
![](https://github.com/sjsimps/Illusion/blob/master/example.gif)
# Illusion - Dynamic Audio Visualization

Illusion monitors sound from any source to modify and display a
set of images in real time.
The frequency spectrum and amplitude of the recorded sound are analyzed
in order to shift the colors of the image.

## Usage:

### Build and run
In the Illusion directory, make and run the project:
```
make illusion
./illusion
```
If build fails, ensure you have all the required libraries installed.

### Selecting a Recording Source
Illusion monitors sound on a system using PulseAudio.
A sound source may be selected using the PulseAudio control GUI:

```
To open the GUI:
> pavucontrol
```
![](https://github.com/sjsimps/Illusion/blob/master/pavucontrol.png)

Select a recording source using the "record from" drop down menu.

### Setting an Image
Set what images Illusion uses by adding them into the Illusion/Images/ folder.

## Libraries Used:
* PulseAudio: libpulse-dev
* SDL 2 and SDL Image: libsdl2-dev, libsdl2-image-dev



