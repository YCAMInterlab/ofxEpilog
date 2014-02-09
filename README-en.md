ofxEpilog
=========

# Description

ofxEpilog is an Open Frameworks addon for the Epilog laser cutter.

# Installation

# Usage
```
#include "ofxEpilog.h"
```

```
void testApp::setup() {
    // Workarea of Epilog laser fuzion
    WorkareaSize fusion; 
    fusion.width = 812;
    fusion.height = 508;
    epilogLaser.setWorkareaSize(fusion);
    
    // Output Configuration
    OutputConfig config;
    config.vspeed = 80;
    config.vpower = 10;
    config.rspeed = 10;
    config.rpower = 1;
    config.dpi = 200;
    config.freq = 100; // percentage
    config.autoFocusEnabled = false;
    epilogLaser.setOutputConfig(config);
    
    bool isConnected = epilogLaser.connect("192.168.3.5");

    // Cut 100mm * 100mm rectangle
    ofPolyline line;
    line.addVertex(0, 0);
    line.addVertex(100, 0);
    line.addVertex(100, 40);
    line.addVertex(0, 100);
    line.addVertex(0, 0);
    line.close();
    ofPtr<HPGLBuffer> hpglBuffer = HPGLBuffer::create(line, epilogLaser.getOutputConfig());
    
    if(epilogLaser.isConnected())
    {
        bool isSent = epilogLaser.send(hpglBuffer, VECTOR);
        ofLog(OF_LOG_NOTICE, "isSent=%d", isSent);
	epilogLaser.disconnect();
    }
}
```

# Examples
* basic
* GML
* live


--
mitsuhito.ando@gmail.com
