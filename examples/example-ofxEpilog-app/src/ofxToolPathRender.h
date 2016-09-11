//
//  ofxToolPathRender.h
//  ofxEpilog
//
//  Created by Mitsuhito Ando on 2016/02/29.
//
//

#pragma once

#include "ofMain.h"

class ofxToolPathRender
{
public:
//    ofPath toolpath;
    ofPolyline toolpath_line;// = ofPolyline();
    
    long current_pt_index = 0;
    long tool_move_count = 0;
    float progress = 0;
    
    //bool is_loaded = false;
    
    void draw();
    void update();
    void set_path(ofPath path);
    
    ofPoint get_current_pt();
    
    float get_progress();
    
    void set_current_pt_index(int idx);
};