//
//  ofxToolPathRender.cpp
//  ofxEpilog
//
//  Created by Mitsuhito Ando on 2016/02/29.
//
//

#include "ofxToolPathRender.h"


void ofxToolPathRender::update()
{
    if(toolpath_line.size() > 0)
    {
        current_pt_index = (current_pt_index + 1) % toolpath_line.size();
    }
}

void ofxToolPathRender::draw()
{
    if(toolpath_line.size() <= 0)
        return;
    
    if(current_pt_index < toolpath_line.size() -1)
    {
        ofPushStyle();
        
        ofSetLineWidth(1.5);
        ofSetColor(255, 10, 22);
        toolpath_line.draw();
        
        ofSetLineWidth(5);
        ofSetColor(255, 0, 0);
        ofDrawLine(toolpath_line[current_pt_index].x, toolpath_line[current_pt_index].y, toolpath_line[current_pt_index].z,
                   toolpath_line[current_pt_index+1].x, toolpath_line[current_pt_index+1].y, toolpath_line[current_pt_index+1].z);
        ofPopStyle();
    }
}

void ofxToolPathRender::set_path(ofPath path)
{
    tool_move_count = 0;
    current_pt_index = 0;
    toolpath_line.close();
    
    if(toolpath_line.size() > 0)
    {
        toolpath_line.clear();
        ofLog() << "toolpath_line.size()::" << toolpath_line.size();
    }
    
    vector<ofPolyline> lines = path.getOutline();
    for(int i=0; i<lines.size(); i++)
    {
        vector<ofPoint> verts = lines[i].getVertices();
        tool_move_count += verts.size();
        
        for (int j=0; j<verts.size() ; j++)
        {
            toolpath_line.addVertex(ofPoint(verts[j].x, verts[j].y, verts[j].z));
        }
    }
}

ofPoint ofxToolPathRender::get_current_pt()
{
    if(toolpath_line.size() > 0)
        return toolpath_line[current_pt_index];
    
    else
        return ofPoint(0,0,0);
}

float ofxToolPathRender::get_progress()
{
    if(tool_move_count == 0)
        return 0;
    else
        progress = (float)current_pt_index / (float)tool_move_count;
    
    return progress;
}

void ofxToolPathRender::set_current_pt_index(int idx)
{
    if(toolpath_line.size() == 0)
        return;
    
    if(current_pt_index < 0)
    {
        current_pt_index = 0;
    }
    else if(current_pt_index <= tool_move_count)
    {
        current_pt_index = tool_move_count;
    }
    else
    {
        current_pt_index = idx;
    }
}