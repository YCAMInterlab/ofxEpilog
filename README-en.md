# ofxEpilog

![header_image](https://cloud.githubusercontent.com/assets/222761/18414970/ecb88974-781a-11e6-9594-ba0818ba8341.jpg)

## Description
ofxEpilog is an openFrameworks addon for the Epilog laser cutters that runs on Mac OS X. Currently we are not supporting Windows and Linux.
ofxEpilog lets you control Epilog laser cutters interactive, generative in openFrameworks. It is possible to generate print jobs from familiar types of openFrameworks and send it through TCP connection. Once you have established connection, the x-y position and the parameter of power, speed, and frequency of a laser, as well as the height of a table could be controlled in the cutting process instead of the pre-configured setting of the proprietary driver. By alternating the generation and transmission of the command of cutting, the addon could sequentially control a laser cutter in real time. This addon enables you to make following examples.

- Process a object with sequentially control. Not only pre-configured setting and cutting data, you can use various input data sources such as mouse, body motion like gaze in real time.

- Process a structured or scanned 3D object. Epilog Fusion series has a function to control the focus with mapped to a color. Therefore, with the addon we could dynamically change the focus with its contour instead of dividing the data into different layers with the height with the proprietary driver. According to a given structured or scanned 3D object with a free-form surface, the addon could engrave 3D curves to the 3D object while keeping the focus by changing table height.

## Usage
For more details, please see [ofxEpilog wiki](https://github.com/YCAMInterlab/ofxEpilog/wiki) .

## Disclaimer
Yamaguchi Center for Arts and Media [YCAM] takes no responsibility regarding eventual damage resulting from your use of this text. In some cases of printing data structure may cause damage to people and laser cutters. Please be careful when you use this addon.

## License
GPL-3.0
