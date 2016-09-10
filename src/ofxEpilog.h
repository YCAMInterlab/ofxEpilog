/*
 
 ofxEpilog - OpenFrameworks library for Epilog laser cutter.
 
 
 Copyright (C) 2002-2008 Andrews & Arnold Ltd <info@aaisp.net.uk>
 Copyright (C) 2008 AS220 Labs <brandon@as220.org>
 Copyright (C) 2011 Trammell Hudson <hudson@osresearch.net>
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

#ifndef _OFXEPILOG_H_
#define _OFXEPILOG_H_

#include "ofMain.h"
#include "ofThread.h"
#include "ofxNetwork.h"

//--------------------------------------------------------------------
//
// Constants
//
//--------------------------------------------------------------------

const double MM_PER_INCH = 25.4;

const string PCL_UNIVERSAL_EXIT_LANG             = "\e%-12345X";
const string PJL_JOB_NAME                        = "@PJL JOB NAME=%s\r\n";
const string PJL_SWITCH_LANG_TO_PCL              = "@PJL ENTER LANGUAGE=PCL\r\n";
const string PJL_EOJ                             = "@PJL EOJ \r\n";
const string PJL_FOOTER                          = "\e%-12345X@PJL EOJ \r\n";

const string PCL_COLOR_COMPONENT_ONE             = "\e*v1536A";
const string PCL_UNKNOWN_CMD1                    = "\e&y130001300003220S"; // ?
const string PCL_TIME_STAMP                      = "\e&y%sD";
const string PCL_UNKNOWN_CMD2                    = "\ey0V"; // ?
const string PCL_UNKNOWN_CMD3                    = "\ey0T"; // ?
const string PCL_UNKNOWN_CMD4                    = "\ey0Z"; // ?
const string PCL_OFFSET_X                        = "\e&l0U";
const string PCL_OFFSET_Y                        = "\e&l0Z";
const string PCL_PRINT_RESOLUTION                = "\e&u%dD"; // dpi
const string PCL_POS_X                           = "\e*p%dX";
const string PCL_POS_Y                           = "\e*p%dY";
const string PCL_RASTER_RESOLUTION               = "\e*t%dR"; // dpi
const string PCL_RASTER_ORIENTATION_PORTRAIT     = "\e*r0F";
const string PCL_UNKNOWN_CMD5                    = "\e&z0T"; // ?
const string PCL_UNKNOWN_CMD6                    = "\e&z0L"; // ?
const string PCL_UNKNOWN_CMD7                    = "\e&y0C"; // ?

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

// TODO(ando@ycam.jp): check is this fusion only?
const string PCL_RASTER_COMPRESSION1              = "\e*b2MLT";
const string PCL_RASTER_COMPRESSION2              = "\e*b%dM";
const string PCL_RASTER_DIRECTION                = "\e&y%dO"; // 0 = down, 1 = up

const string PCL_RASTER_START                    = "\e*r1A";
const string PCL_RASTER_END                      = "\e*rC";

// TODO(ando@ycam.jp) : check encoding format and finish implement raster output
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

// speed comp, fusion only?
const string HPGL_VECTOR_SPEED_CMP               = "XS%d";

// power comp, fusion only?
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
//--------------------------------------------------------------------
struct MachineProfile
{
    string name;
    double width;              // unit: mm
    double height;             // unit: mm
    double z_top;              // unit: mm
    double z_buttom;           // unit: mm
    double min_thickness;      // unit: mm
    double max_thickness;      // unit: mm
    int min_dpi;
    int max_dpi;
    double min_freq;           // unit: hz or percentage
    double max_freq;           // unit: hz or percentage
    bool is_freq_percentage;
    double focus_dist;         // depends on lens spech
};

//--------------------------------------------------------------------
//
// For default output configuration
//
//--------------------------------------------------------------------
struct OutputConfig
{
    int vec_speed = 10;
    int vec_power = 0;
    int ras_speed = 0;
    int ras_power = 10;
    int dpi = 600;
    int vec_freq = 0;
    int ras_freq = 0;
    
    double z_offset = 0.0;
    double z_thickness = 0.0;
    bool use_auto_fucus = false;
    
    enum RASTER_MODE { BMP16M = 7, BMPGRAY = 2 };
    RASTER_MODE raster_mode;
    
    bool use_bezier_cmd = false;
    bool enable_laser_emit = false;
};


//--------------------------------------------------------------------
//
// ofxEpilog class definition
//
//--------------------------------------------------------------------
// TODO(ando@ycam.jp) : replace threaded class to support non-blocking operation
class ofxEpilog
{
    
public:
    ofxEpilog();
    ofxEpilog(const ofxEpilog &obj);
    ~ofxEpilog();
        
    // TODO(ando@ycam.jp): add non-blocking feature.
    void setup(MachineProfile m_profile, OutputConfig out_conf, bool save_pjl=false);
    
    // connect to the laser cutter.
    bool connect(string ip, bool live=false);
    
    // close established connection.
    void disconnect();
    
    // return the status of connection.
    bool isConnected();

    // set epilog IP address.
    void setIPAddress(string ip);
    
    // return the epilog IP address.
    string getIPAddress() { return ip_address; };

    // set specific type of epilog
    // default machine profile is also defined at ofxEpilog::FUSION_32 and ofxEpilog::MINI_18.
    // TODO(ando@ycam.jp) : add load feature of machine profile by XML.
    void setMachineProfile(MachineProfile m_profile);
    
    // return the current machine profile.
    MachineProfile& getMachineProfile(){ return machine_profile; };
    
    // set parameters of job process, e.g. power, speed, freq, etc.
    void setOutputConfig(OutputConfig out_config);
    
    // return the current output configration.
    OutputConfig& getOutputConfig() { return output_conf; };
    
    // send vertexes os ofPolyline to epilog
    bool send(ofPolyline line);
    
    // send vertexes as ofPath to epilog
    // TODO(ando@ycam.jp) : finish implementation.
    bool send(ofPath path);
    
    // send vertexes as ofImage to epilog
    // TODO(ando@ycam.jp) : finish implementation.
    bool send(ofImage img, int w_mm, int h_mm);
    
    // const machine profiles
    static const struct MachineProfile FUSION_32;
    static const struct MachineProfile MINI_18;

protected:
    // epilog's ip address
    string ip_address;
    
    // own host name
    string hostname;
    
    // destination port
    static const uint PRINTER_SERVICE_PORT = 515;
    
    // tcp client object
    ofxTCPClient tcp_client;
    
    // current machine profile
    MachineProfile machine_profile;
    
    // current output config
    OutputConfig output_conf;
    
    // live mode switch
    bool keep_alive = false;
    
    // pjl file export switch
    bool save_pjl_file = false;
    
    // for pjl file output
    ofPtr<ofFile> pjl_file;

    // return header of PJL payload
    ofBuffer createPayloadHeader(MachineProfile &m_prof, OutputConfig &out_conf);
    
    // return raster part of PJL payload
    ofBuffer createPayloadRasterBody(ofImage raster_img, OutputConfig &out_conf);
    
    // return vector parameters part of PJL payload
    ofBuffer createPayloadVectorParams(OutputConfig &out_conf);
    
    // return vector part of PJL payload
    ofBuffer createPayloadVectorBody(ofPolyline vector_vertexes, OutputConfig &out_conf);
    
    // return raster and vector part as combined
    ofBuffer createPayloadCombinedBody(ofImage raster_img, ofPolyline vector_vertexes);
    
    // return footer part of PJL payload
    ofBuffer createPayloadFooter();
};


#endif // _OFXEPILOG_H_
