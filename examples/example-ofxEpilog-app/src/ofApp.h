#pragma once

#include "ofMain.h"
#include "ofxDatGui.h"
#include "ofxEpilog.h"
#include "ofxAssimpModelLoader.h"
#include "ofxDxfReader.h"
#include "ofxToolPathRender.h"


class ofApp : public ofBaseApp
{

public:
    
    void setup();
    void update();
    void draw();
    void exit();

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);

    ofxEpilog epilogLaser;
    
    ofBoxPrimitive epilog_table;
    
    ofxDatGui *gui;
    
    ofTrueTypeFont font;
    
    ofEasyCam easy_cam;
    
    // gui event handers
    void gui_button_handler(ofxDatGuiButtonEvent event);
    void gui_2dpad_handler(ofxDatGui2dPadEvent event);
    void gui_slider_hander(ofxDatGuiSliderEvent event);
    
    ofxAssimpModelLoader model;
    ofPoint model_origin;
    
    bool vsync;
    
    ofPath toolpath;
    ofxDxfReader dxf_reader;
    
    ofPolyline polyline_mouse_cursor;
    
    ofxToolPathRender toolpath_render;
    
    bool is_simulating;
    bool is_mouse_hiding;
    bool is_visible_dxf = true;
    bool is_visible_3d_model = true;
    
    bool live_mode;
};
