/*
 
 ofxEpilog - OpenFrameworks library for Epilog laser cutter.
 
 
 Copyright (C) 2002-2008 Andrews & Arnold Ltd <info@aaisp.net.uk>
 Copyright (C) 2008 AS220 Labs <brandon@as220.org>
 Copyright (C) 2011 Trammell Hudson <hudson@osresearch.net>
 Copyright (C) 2013 Mitsuhito Ando <ando@ycam.jp, mitsuhito.ando@gmail.com>
 
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

#pragma once

//#ifndef __ofxEpilog__ofxEpilog__
//#define __ofxEpilog__ofxEpilog__

#include "ofMain.h"
#include "ofThread.h"
#include "ofxNetwork.h"
#include "ofxAssimpModelLoader.h"


//
//#include "dl_dxf.h"
//#include "dl_attributes.h"
//#include "dl_creationadapter.h"
//
////--------------------------------------------------------------------
////
//// TEST: dxf file raeder for dynamic focus mapping
////
//class DXFPathReader : public DL_CreationAdapter
//{
//public:
//    ofPath getOfPath(string file_path)
//    {
//        DL_Dxf *dxf = new DL_Dxf();
//        if(!dxf->in(file_path, this))
//        {
//            ofLog(OF_LOG_ERROR) << "PathReader::getOfPath() : Could not retrieve data from input file.";
//        }
//        delete dxf;
//        //ofLog() << path.getOutline().size();
//        return path;
//    };
//    void addLine(const DL_LineData &data)
//    {
//        // TODO: we have to flip y axis from rhino exported dxf coordination
//        //    +y
//        //     |
//        //-x --+-- +x
//        //     |
//        //    -y
//        path.lineTo(data.x1, data.y1, data.z1);
//        path.lineTo(data.x2, data.y2, data.z2);
//        int c = attributes.getColor();
//        path.setColor(
//                      ofColor(
//                              (c & 0x00ff0000) >> 16 ,
//                              (c & 0x0000ff00) >> 8,
//                              (c & 0x00ff0000)
//                              )
//                      );
//        //ofColor c = ofColor();
//        path.close();
//        
//        
//        //ofLog() << "addLine():: (x1:" << data.x1 << ", y1:" << data.y1 << ", z1:" << data.z1 << "), (x2:" << data.x2 << ", y2:" << data.y2 << ", z2:" << data.z2 << ")";
//    };
//    
//    void addAttribute(const DL_AttributeData& data)
//    {
//    };
//    
//    void addPolyline(const DL_PolylineData &data)
//    {
//        ofLog() << "addPolyLine()::" << " m:" << data.m << " n:" << data.n << " number:" << data.number;
//        path.close();
//    };
//    
//    void addSpline(const DL_SplineData &data)
//    {
//        ofLog() << "addSpline()::";
//    };
//    
//    void addVertex(const DL_VertexData &data)
//    {
//        ofLog() << "addVertex()::" << "x:" << data.x << " y:" << data.y << " z:" << data.z << " bulge:" << data.bulge;
//        path.lineTo(ofPoint(data.x, data.y, data.z));
//        
//    };
//    
//private:
//    ofPath path;
//};

//--------------------------------------------------------------------
//
// Constants
//
#define MM_PER_INCH 25.4

const string PJL_HEADER                          = "\e%%-12345X@PJL JOB NAME=%s\r\n\eE@PJL ENTER LANGUAGE=PCL\r\n";
const string PJL_FOOTER                          = "\e%-12345X@PJL EOJ \r\n";

const string PCL_COLOR_COMPONENT_ONE             = "\e*v1536A";
const string PCL_UNKNOWN_CMD1                    = "\e&y130001300003220S"; // ?
const string PCL_TIME_STAMP                      = "\e&y%sD"; //"\e&y20150913212514D";
const string PCL_UNKNOWN_CMD2                    = "\ey0V"; // ?
const string PCL_UNKNOWN_CMD3                    = "\ey0T"; // ?
const string PCL_UNKNOWN_CMD4                    = "\ey0Z"; // ?
const string PCL_OFFSET_X                        = "\el0U";
const string PCL_OFFSET_Y                        = "\el0X";
const string PCL_PRINT_RESOLUTION                = "\eu%dD"; // 600 dpi
const string PCL_POS_X                           = "\e*p%dX";
const string PCL_POS_Y                           = "\e*p%dY";
const string PCL_RASTER_RESOLUTION               = "\e*%dR"; // 600 dpi
const string PCL_RASTER_ORIENTATION              = "\e*r0F"; // 0
const string PCL_UNKNOWN_CMD5                    = "\e&z0T"; // ?
const string PCL_UNKNOWN_CMD6                    = "\e&z0L"; // ?

// raster power 1~100%
const string PCL_RASTER_POWER                    = "\e&y%dP";
// raster speed 1~100%
const string PCL_RASTER_SPEED                    = "\e&z%dS";
// raster freq? fusion 1~100%, mini 1hz~5000hz
const string PCL_RASTER_FREQ                     = "\e&y%dR";
// raster offset z -12.7~76.2mm, fusion only?
const string PCL_RASTER_DEFAULT_OFFSET_Z         = "\e&y%dE";
// raster thickness z 0~177.8mm, fusion only? e.g.3.3mm->130*0.001in, round( (1 / 0.0254mm) * (z_offset or thickness mm) = in (0.001) )
const string PCL_RASTER_DEFAULT_THICKNESS_Z      = "\e&y%dA";
// raster offset z -12.7~76.2mm, fusion only?
const string PCL_RASTER_COLORMAP_OFFSET_Z        = "\e&y%dF";
// raster thickness z 0~177.8mm, fusion only? e.g.3.3mm->130*0.001in, round( (1 / 0.0254mm) * (z_offset or thickness mm) = in (0.001) )
const string PCL_RASTER_COLORMAP_THICKNESS_Z     = "\e&y%dB";

const string PCL_RASTER_AIR_ASSIST               = "\e&z%dA"; // 0 = off, 1 = raster air assist on, 2 = global air assist on

const string PCL_PAGE_WIDTH                      = "\e*r%dS"; // page width : e.g. %d = page_w_mm * (600dpi / 25.4mm)
const string PCL_PAGE_HEIGHT                     = "\e*r%dT"; // page height : e.g. %d = page_h_mm * (600dpi / 25.4mm)

const string PCL_RASTER_COMPRESSION1              = "\eb2MLT"; // TODO: check is this fusion only?
const string PCL_RASTER_COMPRESSION2              = "\eb2M"; // ?
const string PCL_RASTER_DIRECTION                = "\e&y%dO"; // 0 = down, 1 = up

const string PCL_RASTER_START                    = "\e*r1A";
const string PCL_RASTER_END                      = "\e*rC";

// TODO : check encoding format and finish implement raster output
const string PCL_ROW_UNPACKED_BYTES              = "\e*b%dA";
const string PCL_ROW_PACKED_BYTES                = "\e*b%dW";

const string PCL_RESET                           = "\eE";

const string HPGL_CMD_DELIMITER                  = ";";

const string HPGL_START                          = "\e%1B";

const string HPGL_VECTOR_INIT                    = "IN";

const string HPGL_UNKNOWN                        = "ZL0"; // laser type selection? co2 or fiber

// raster offset z -12.7~76.2mm, fusion only?
const string HPGL_VECTOR_DEFAULT_OFFSET_Z        = "WO%d";

// thickness z 0~177.8mm, fusion only? e.g.3.3mm->130*0.001in, round( (1 / 0.0254mm) * (z_offset or thickness mm) = in (0.001) )
const string HPGL_VECTOR_DEFAULT_THICKNESS_Z     = "WF%d";

const string HPGL_VECTOR_POWER                   = "YP%d";//"YP%03d";
const string HPGL_VECTOR_SPEED                   = "ZS%d";//"ZS%03d";

const string HPGL_VECTOR_FREQ_FUSION             = "XR%d";//"XR%03d";
const string HPGL_VECTOR_FREQ_MINI               = "XR%d";//"XR%04d";

// speed comp.? fusion only?
const string HPGL_VECTOR_SPEED_CMP               = "XS%d";

// power comp.? fusion only?
const string HPGL_VECTOR_POWER_CMP               = "XP%d";

const string HPGL_VECTOR_AIR_ASSIST              = "WA%d"; // 0 = off, 1 = raster air assist on, 2 = global air assist on

const string HPGL_VECTOR_PEN_UP                  = "PU";
const string HPGL_VECTOR_PEN_DOWN                = "PD";

// fusion only?
const string HPGL_VECTOR_PEN_DOWN_BEZIER         = "PDBZ"; // quadratic bezier

// raster offset z -12.7~76.2mm, fusion only?
const string HPGL_VECTOR_COLORMAP_OFFSET_Z       = "WD%d";

// thickness z 0~177.8mm, fusion only? e.g.3.3mm->130*0.001in, round( (1 / 0.0254mm) * (z_offset or thickness mm) = in (0.001) )
const string HPGL_VECTOR_COLORMAP_THICKNESS_Z    = "WC%d";

const string HPGL_END                            = "\e%0B";


//--------------------------------------------------------------------
//
// Machine profile
//
struct MachineProfile
{
    string name;
    float width;              // unit: mm
    float height;             // unit: mm
    float z_top;              // unit: mm
    float z_buttom;           // unit: mm
    float min_thickness;      // unit: mm
    float max_thickness;      // unit: mm
    int min_dpi;
    int max_dpi;
    float min_freq;           // unit: hz or percentage
    float max_freq;           // unit: hz or percentage
    bool is_freq_percentage;
    float focus_dist;         // depends on lens spech
};

//--------------------------------------------------------------------
//
// For default output configuration
//
struct OutputConfig
{
    int vec_speed = 10;
    int vec_power = 0;
    int ras_speed = 0;
    int ras_power = 10;
    int dpi = 600;
    int vec_freq = 0;
    int ras_freq = 0;
    
    float z_offset = 0.0;
    float z_thickness = 0.0;
    bool use_auto_fucus = false;
    
    enum RASTER_MODE { BMP16M = 7, BMPGRAY = 2 };
    RASTER_MODE raster_mode;
    
    bool use_bezier_cmd = false;
};

/*
//
// HPGLBuffer creates HPGL formatted string from ofPath, ofPolyline, ofPixels, ofImage, pdf, eps, jpg, png, gif ...
// If you want to create from other types, make sub class and overload static method.
//
class HPGLBuffer : public ofBuffer
{
public:
    HPGLBuffer() : ofBuffer()
    {
        //ofLog(OF_LOG_VERBOSE, "HPGLBuffer() is called." + ofToString(this));
    };
    
    virtual ~HPGLBuffer()
    {
        //ofLog(OF_LOG_VERBOSE, "~HPGLBuffer() is called." + ofToString(this));
    };
    
    static ofPtr<HPGLBuffer> create(ofPolyline line, OutputConfig config);
    static ofPtr<HPGLBuffer> create(ofPath path, OutputConfig config);
    static ofPtr<HPGLBuffer> create(ofImage img, ofPoint offset, OutputConfig config); // offset(mm)
    static ofPtr<HPGLBuffer> create(ofPixels pixelsRef, ofPoint offset, OutputConfig config); // offset(mm)
    static ofPtr<HPGLBuffer> createFromPaylaodFile(const string path, const string delimiter, ofPoint offset, OutputConfig config); // For debug
};
*/

/*
//
// GMLBuffer creates HPGL formatted string from Graffiti Markup Language.
// To know GML detail, see http://www.graffitimarkuplanguage.com/g-m-l-spec/ .
//
class GMLBuffer : public HPGLBuffer
{
public:
    GMLBuffer() : HPGLBuffer()
    {
        //ofLog(OF_LOG_VERBOSE, "GMLBuffer() is called." + ofToString(this));
    };
    
    virtual ~GMLBuffer()
    {
        //ofLog(OF_LOG_VERBOSE, "~GMLBuffer() is called." + ofToString(this));
    };
    static ofPtr<GMLBuffer> create(string gmlFilePath, OutputConfig config);
};
*/


//--------------------------------------------------------------------
//class ofxEpilog : public ofThread
class ofxEpilog
{
    
public:
    ofxEpilog();
    ofxEpilog(const ofxEpilog &obj);
    ~ofxEpilog();
    
    void debug();
    
    // TODO: add non blocking feature
    void setup(MachineProfile m_profile, OutputConfig out_conf, bool save_pjl=false);
    
    bool connect(string ip, bool live=false);
    void disconnect();
    
    bool isConnected();

    void setIPAddress(string ip);
    string getIPAddress() { return ip_address; };

    void setMachineProfile(MachineProfile m_profile);
    MachineProfile& getMachineProfile(){ return machine_profile; };
    
    void setOutputConfig(OutputConfig out_config);
    OutputConfig& getOutputConfig() { return output_conf; };
    
    //bool send(const ofPtr<HPGLBuffer> &buffer, JOB_TYPE type);
    
    bool send(ofPolyline line, OutputConfig config);
    bool send(ofPath path, OutputConfig config);
    bool send(ofImage img, int w_mm, int h_mm, OutputConfig config, bool add_start_header=true, bool add_end_footer=true);
    bool send(ofXml gml_file, int w_mm, int h_mm, OutputConfig config);
    
    static const struct MachineProfile FUSION_32;
    static const struct MachineProfile MINI_18;
    
    //bool loadModel(string f_name);
    
protected:
    string ip_address;
    string hostname;
    
    static const uint PRINTER_SERVICE_PORT = 515;
    ofxTCPClient tcp_client;
    
    MachineProfile machine_profile;
    OutputConfig output_conf;
    
    //int DEFAULT_RESAMPLE_CNT = 200;
    
    bool is_live_mode = false;
    bool pjl_header_sent = false;
    bool save_pjl_file = false;
    
    //ofFile pjl_file;
    ofPtr<ofFile> pjl_file;
    
    ofBuffer getPCLHeader();
    
    ofBuffer getPCLDefaultOutputConfig();
    ofBuffer getPCLColorMappingOutputConfig(OutputConfig color_mapped_config);
    ofBuffer getPCLRasterBody(ofImage img, int w_mm, int h_mm, bool add_start_header=true, bool add_end_footer=true);
    
    ofBuffer getHPGLHeader();
    ofBuffer getHPGLDefaultOutputConfig();
    ofBuffer getHPGLColorMappingOutputConfig(OutputConfig config);
    ofBuffer getHPGLVectorBody(ofPolyline line, OutputConfig config, int resample_count=0);
    ofBuffer getHPGLVectorBody(ofPath path, OutputConfig config, int resample_count=0);
    
//    ofBuffer getHPGLVectorBody(ofPath path, OutputConfig config, bool use_bezier_output=false, int resample_count=200);
    
    ofBuffer getHPGLFooter();
    ofBuffer getPCLFooter();
    
    //ofxAssimpModelLoader model;
    //void threadedFunction();
    
    
private:
    //virtual void threadedFunction();
};


//#endif
