ofxEpilog
=========

# Description
ofxEpilog is an Open Frameworks addon for the Epilog laser cutter that runs on Mac OS X. Currently we are not supporting Windows and Linux.
ofxEpilog lets you control Epilog laser cutters interactive, generative in Open Frameworks. It is possible to generate print jobs from familiar types of Open Frameworks and send it through TCP connection. Once you have established connection, you can send jobs to control head position and turn on or off laser beam continuously in almost real time.


Installation
---
* If you haven't donwloaded Open Frameworks, get `of_v0.8.0_osx_release.zip` from oF site.
  We are using v0.8.0 to develop and test.  
  **<http://www.openframeworks.cc/download/>**

* Download newest version of ofxEpilog from github and extract it.  
  **<https://github.com/mitsuhito/ofxEpilog/archive/master.zip>**
  
  If you are familiar with console, type `git clone git://github.com/mitsuhito/ofxEpilog.git` .  

* Copy ofxEpilog directory into `of_v0.8.0_osx_release/addons/` .
  

Usage
---
* __Create new project from projectGenerator__  
    Check ofxNetwork and ofxEpilog at addons. 
    ![projectGenerator screen shot](https://raw.github.com/mitsuhito/ofxEpilog/master/ofxEpilogAddonUsage1.png)

* __Adding an existing project__
    * If you are not using ofxNetwork, create a new group named "ofxNetwork" inside addons group.
    * Drag ofxNetwork/src directory into this new group.
    * Create a new group named "ofxEpilog" inside addons group.
    * Drag ofxEpilog/src directory into this new group.
    <img src="https://raw.github.com/mitsuhito/ofxEpilog/master/ofxEpilogAddonUsage3.png"/>


* __Usage of ofxEpilog class__  

    * Include header file.
    ```cpp
    #include "ofxEpilog.h"
    ```  

    * Create ofxEpilog instance.
    ```cpp
    ofxEpilog epilogLaser;
    ```  

    * Set maximum bed size of your laser cutter. 
    ```cpp
    WorkareaSize fusion;
    fusion.width = 812;				    		// width  mm
    fusion.height = 508;			    		// height mm
    epilogLaser.setWorkareaSize(fusion);
    ```  

    * Set output parameters (Vector speed, Vector power, Raster speed, Raster power, Frequency, DPI, Auto focus).
    ```cpp
    OutputConfig config;
    config.vspeed = 80;				    		// Vector speed   1-100%
    config.vpower = 10;				    		// Vector power   1-100%
    config.rspeed = 10;				    		// Raster speed   1-100%
    config.rpower = 20;				    		// Raster power   1-100%
    config.dpi = 200;				        	// DPI            75-1200
    config.freq = 100;				        	// Frequency      1-100%
    config.autoFocusEnabled = false;				// Auto focus
    epilogLaser.setOutputConfig(config);
    ```  

    * Connect to the laser cutter.
    ```cpp
    bool isConnected = epilogLaser.connect("192.168.3.5");
    ```  

    * After establishing connection, you can send jobs to the laser cutter. Supported object types are `ofPolyline` , `ofImage` , `GMLBuffer (Graffiti Markup Language)` only.
    ```cpp
    // Cut 100mm * 100mm rectangle
    ofPolyline line;
    line.addVertex(0, 0);
    line.addVertex(100, 0);
    line.addVertex(100, 100);
    line.addVertex(0, 100);
    line.addVertex(0, 0);
    line.close();
    
    ofPtr<HPGLBuffer> hpglBuffer = HPGLBuffer::create(line, epilogLaser.getOutputConfig());
    
    if(epilogLaser.isConnected())
    {
    	bool isSent = epilogLaser.send(hpglBuffer, VECTOR);
    	ofLog(OF_LOG_NOTICE, "isSent=%d", isSent);
    }
    ```  

    _If you want to send other types of object, overload `ofPtr<HPGLBuffer> HPGLBuffer::create()` method, or create subclasses of `HPGLBuffer` class then implement factory method._  

    * Disconnect.    
    ```cpp
    epilogLaser.disconnect();
    ```  

Examples
---
* example-ofxEpilogBasicExample
* example-ofxEpilogGMLExample
* example-ofxEpilogLiveExample


---
mitsuhito.ando@gmail.com
