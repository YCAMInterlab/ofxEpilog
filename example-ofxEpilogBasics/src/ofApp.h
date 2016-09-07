#pragma once

#include "ofMain.h"

#include "ofxDatGui.h"

#include "ofxEpilog.h"

#include "ofxAssimpModelLoader.h"
#include "ofxGizmo.h"

#include "ofxToolPathReader.h"

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
    
    ofBoxPrimitive table;
    
    ofxDatGui *gui;
    
    ofTrueTypeFont font;
    
    ofEasyCam easy_cam;
    
    // gui event handers
    void gui_button_handler(ofxDatGuiButtonEvent event);
    void gui_2dpad_handler(ofxDatGui2dPadEvent event);
    void gui_slider_hander(ofxDatGuiSliderEvent event);
    
    ofxAssimpModelLoader model;
    ofPoint origin;
    
    ofxGizmo gizmo;
    
    bool vsync;
    
    ofxToolPathReader toolpath_reader;
    ofPath toolpath;
    
    void dxf_file_load_complete_hander(bool complete);
    
    ofxToolPathReader reader;
    
    ofPolyline laser_traced_line;
    
    ofxToolPathRender toolpath_render;
    
    bool is_simulating;
    
    bool is_mouse_hiding;
    
    bool is_visible_dxf = true;
    bool is_visible_3d_model = true;
};
