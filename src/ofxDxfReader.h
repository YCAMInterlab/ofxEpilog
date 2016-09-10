/*
 
 ofxDxfReader - DXF file reader.
 
 Copyright (C) 2016 Mitsuhito Ando <ando@ycam.jp, mitsuhito.ando@gmail.com>
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
 */

#ifndef _OFXDXFREADER_H_
#define _OFXDXFREADER_H_

#include "ofMain.h"

// includes from libdxf
#include "dl_dxf.h"
#include "dl_attributes.h"
#include "dl_creationadapter.h"

// dxf file reader class
class ofxDxfReader : public DL_CreationAdapter
{

public:
    ofxDxfReader();
    ~ofxDxfReader();
    
    ofPath getOfPath(string file_path)
    {
        ofLog(OF_LOG_VERBOSE) <<  "ofxDxfReader::getOfPath() " << file_path;
        
        path.clear();
        
        DL_Dxf *dxf = new DL_Dxf();
        
        if(!dxf->in(file_path, this))
        {
            ofLog(OF_LOG_ERROR) << "ofxDxfReader::getOfPath() : Could not retrieve data from input file, for now only DXF file format supported.";
        }
        
        delete dxf;
        
        return path;
    };
    
    void addLine(const DL_LineData &data)
    {
        // TODO: we have to flip y axis from rhino exported dxf coordination
        //    +y
        //     |
        //-x --+-- +x
        //     |
        //    -y
        
        path.lineTo(data.x1, data.y1, data.z1);
        path.lineTo(data.x2, data.y2, data.z2);
        
        //
        // TODO(ando@ycam.jp): grab color infomation to determine cutting path or just moving the head.
        //
        
        path.close();
        
        ofLog(OF_LOG_VERBOSE) << "addLine():: (x1:" << data.x1 << ", y1:" << data.y1 << ", z1:" << data.z1 << "), (x2:" << data.x2 << ", y2:" << data.y2 << ", z2:" << data.z2 << ")";
    };
    
    void addAttribute(const DL_AttributeData& data)
    {
        ofLog(OF_LOG_VERBOSE) << data.tag;
    };
    
    void addPolyline(const DL_PolylineData &data)
    {
        ofLog(OF_LOG_VERBOSE) << "addPolyLine()::" << " m:" << data.m << " n:" << data.n << " number:" << data.number;
        path.close();
    };
    
    void addSpline(const DL_SplineData &data)
    {
        ofLog(OF_LOG_VERBOSE) << "addSpline()::";
    };
    
    void addVertex(const DL_VertexData &data)
    {
        ofLog(OF_LOG_VERBOSE) << "addVertex()::" << "x:" << data.x << " y:" << data.y << " z:" << data.z << " bulge:" << data.bulge;
        path.lineTo(ofPoint(data.x, data.y, data.z));
        
    };
    
    void endSequence()
    {
        ofLog(OF_LOG_VERBOSE) << "endSequence()" ;
    }
    
    ofPath path;
    
    //
    // TODO(ando@ycam.jp): add load complete event dispatcher
    //
    //static ofEvent<ofxDxfReader> load_complete_event;
};

#endif // _OFXDXFREADER_H_
