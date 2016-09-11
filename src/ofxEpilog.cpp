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

#include "ofxEpilog.h"


// Default profiles from tech spechs.
// http://support.epiloglaser.com/article/8205/42831/focus-lens-101
//
// TODO(ando@ycam.jp) : load from external configuration file, not hard coded.

// 2 inch lens
// https://www.epiloglaser.com/products/fusion-laser-series.htm#techspecs
const struct MachineProfile ofxEpilog::FUSION_32 =
{
    "Fusion_32", 813, 508, -12.7, 76.2, 0, 177.8, 75, 1200, 0, 100, true, 50.8
};

// 1.5 inch lens
// https://www.epiloglaser.com/products/legend-laser-series.htm#techspecs
const struct MachineProfile ofxEpilog::MINI_18 =
{
    "Mini_18", 457, 305, 0, 0, 0, 177.8, 75, 1200, 10, 5000, false, 38.1
};

// Init ofxEpilog instance and set default values.
ofxEpilog::ofxEpilog()
{
    ip_address = "000.000.000.000"; // dummy
    hostname = "UNKOWN HOSTNAME";
    
    char localhost[128] = "";
    gethostname(localhost, sizeof(localhost));
    hostname = string(localhost);
    
    keep_alive = false;
    
    cout << "ofxEpilog::ofxEpilog() hostname=" << hostname << endl;
}

ofxEpilog::ofxEpilog(const ofxEpilog &obj)
{
    ofLog(OF_LOG_VERBOSE, "ofxEpilog::ofxEpilog(const ofxEpilog &obj)");
}

// close TCP connection and save pjl file if necessary.
ofxEpilog::~ofxEpilog()
{
    if(tcp_client.isConnected())
        disconnect();
    
    if(save_pjl_file)
    {
        if(pjl_file.get() != NULL)
            pjl_file->close();
    }
}

void ofxEpilog::setup(MachineProfile m_profile, OutputConfig out_conf, bool save_pjl)
{
    setMachineProfile(m_profile);
    setOutputConfig(out_conf);
    save_pjl_file = save_pjl;
    
    if(save_pjl_file)
    {
        pjl_file = ofPtr<ofFile>(new ofFile(ofToDataPath(ofGetTimestampString()+"_pjl.txt"), ofFile::WriteOnly));
        pjl_file->create();
    }
}

bool ofxEpilog::connect(string ip, bool live)
{
    setIPAddress(ip);
    
    ofLog(OF_LOG_VERBOSE, "ofxEpilog::connect() start cnnecting to the laser cutter.");
    
    if(tcp_client.isConnected())
    {
        ofLog(OF_LOG_WARNING, "ofxEpilog: the connection is created already.You should disconnect first." );
        disconnect();
    }
    
    keep_alive = live;
    bool is_connected = tcp_client.setup(ip_address, PRINTER_SERVICE_PORT, true);
    
    if(!is_connected)
        return false;
    
    ofLog() << "ofxEpilog:connect(): start sending LPR handshake packets.";
    
    //
    // Send LPR Handshake Packets
    // See http://www.rfc-editor.org/rfc/rfc1179.txt
    //
    std::ostringstream stream;
    /*
     +----+-------+----+
     | 02 | Queue | LF |
     +----+-------+----+
     Command code - 2
     Operand - Printer queue name
     */
    tcp_client.sendRaw("\002Legend\n");
    
    char response;
    tcp_client.receiveRawBytes(&response, 1);
    if(response == 0)
    {
        /*
         +----+-------+----+------+----+
         | 02 | Count | SP | Name | LF |
         +----+-------+----+------+----+
         Command code - 2
         Operand 1 - Number of bytes in control file
         Operand 2 - Name of control file
         */
        stream << "\002" << hostname.length()+2 << " cfA" << hostname << "\n";
        tcp_client.sendRaw(stream.str());
        
        tcp_client.receiveRawBytes(&response, 1);
    }
    else
        return false;
    
    if(response == 0)
    {
        /*
         +---+------+----+
         | H | Host | LF |
         +---+------+----+
         Command code - 'H'
         Operand - Name of host
         */
        stream.str("");
        stream.clear();
        stream << "H" << hostname << "\n" << '\0';
        tcp_client.sendRaw(stream.str());
        
        tcp_client.receiveRawBytes(&response, 1);
    }
    else
        return false;
    
    if(response == 0)
    {
        /*
         +----+-------+----+------+----+
         | 03 | Count | SP | Name | LF |
         +----+-------+----+------+----+
         Command code - 3
         Operand 1 - Number of bytes in data file
         Operand 2 - Name of data file
         */
        stream.str("");
        stream.clear();
        if(machine_profile.name == ofxEpilog::FUSION_32.name)
            stream << "\003" << "125899906843000" << " dfA" << hostname << "\n"; // <- buffer size is correct?
        
        if(machine_profile.name == ofxEpilog::MINI_18.name)
            stream << "\003" << "65536" << " dfA" << hostname << "\n"; // <- 64MB?
        
        tcp_client.sendRaw(stream.str());
        
        tcp_client.receiveRawBytes(&response, 1);
    }
    else
        return false;
    
    ofLog() << "ofxEpilog::connect(): LPR handshake complete.";
    
    return true;
}

ofBuffer ofxEpilog::createPayloadHeader(MachineProfile &m_prof, OutputConfig &out_conf)
{
    // payload buffer
    ofBuffer buffer;
    
    // temporary buffer for sprintf
    char tmp_str[256] = {0};
    
    // add UEL command
    buffer.append(PCL_UNIVERSAL_EXIT_LANG);
    
    // add job name
    // mybe we don't need to send every time
    memset(tmp_str, 0, strlen(tmp_str));
    sprintf(tmp_str, PJL_JOB_NAME.c_str(), ofToString("ofx_" + ofToString(ofGetFrameNum())).c_str());
    buffer.append(tmp_str);
    
    if(m_prof.name == ofxEpilog::FUSION_32.name)
    {
        // TODO(ando@ycam.jp) : check the meaning again
        buffer.append(PCL_COLOR_COMPONENT_ONE);
        
        // unknown command, but we need to send
        buffer.append(PCL_UNKNOWN_CMD1);
        
        // embed time stamp string
        memset(tmp_str, 0, strlen(tmp_str));
        sprintf(tmp_str, PCL_TIME_STAMP.c_str(), ofGetTimestampString("%Y%m%d%H%M%S").c_str());
        buffer.append(tmp_str);
        
        // unknown command, but we need to send
        buffer.append(PCL_UNKNOWN_CMD2);
        buffer.append(PCL_UNKNOWN_CMD3);
        buffer.append(PCL_UNKNOWN_CMD4);
    }
    
    // add printer reset command
    buffer.append(PCL_RESET);
    
    // switch to PCL mode
    buffer.append(PJL_SWITCH_LANG_TO_PCL);
    
    if(m_prof.name == ofxEpilog::MINI_18.name)
    {
        // set auto focus, 0:disable, 1:enable (for mini/helix series)
        memset(tmp_str, 0, strlen(tmp_str));
        sprintf(tmp_str, PCL_RASTER_DEFAULT_THICKNESS_Z.c_str(), (out_conf.use_auto_fucus));
        buffer.append(tmp_str);
    }
    
    // add printer reset command
    buffer.append(PCL_RESET);
    
    // left offset registration command
    buffer.append(PCL_OFFSET_X);
    
    // top offset registration command
    buffer.append(PCL_OFFSET_Y);
    
    // unit of measure command (dpi)
    memset(tmp_str, 0, strlen(tmp_str));
    sprintf(tmp_str, PCL_PRINT_RESOLUTION.c_str(), out_conf.dpi);
    buffer.append(tmp_str);
    
    // horizontal cursor positioning (PCL units) command, default: 0
    memset(tmp_str, 0, strlen(tmp_str));
    sprintf(tmp_str, PCL_POS_X.c_str(), 0);
    buffer.append(tmp_str);
    
    // vertical cursor positioning (PCL units) command, default: 0
    memset(tmp_str, 0, strlen(tmp_str));
    sprintf(tmp_str, PCL_POS_Y.c_str(), 0);
    buffer.append(tmp_str);
    
    // raster resolution (dpi)
    memset(tmp_str, 0, strlen(tmp_str));
    sprintf(tmp_str, PCL_RASTER_RESOLUTION.c_str(), out_conf.dpi);
    buffer.append(tmp_str);
    
    // unknown command, but we need to send
    buffer.append(PCL_UNKNOWN_CMD7);
    
    // raster orientation (portrait)
    buffer.append(PCL_RASTER_ORIENTATION_PORTRAIT);
    
    if(m_prof.name == ofxEpilog::FUSION_32.name)
    {
        // unknown command, but we need to send
        buffer.append(PCL_UNKNOWN_CMD7);
    }
    
    // raster height command
    memset(tmp_str, 0, strlen(tmp_str));
    sprintf(tmp_str, PCL_PAGE_HEIGHT.c_str(), int(machine_profile.height * (out_conf.dpi/MM_PER_INCH)));
    buffer.append(tmp_str);
    
    // raster width command
    memset(tmp_str, 0, strlen(tmp_str));
    sprintf(tmp_str, PCL_PAGE_WIDTH.c_str(), int(machine_profile.width * (out_conf.dpi/MM_PER_INCH)));
    buffer.append(tmp_str);

    // raster compression type
    memset(tmp_str, 0, strlen(tmp_str));
    sprintf(tmp_str, PCL_RASTER_COMPRESSION2.c_str(), out_conf.raster_mode);
    buffer.append(tmp_str);
    //buffer.append("\e*b7M");

    // raster direction (0:down)
    buffer.append(PCL_RASTER_DIRECTION);
    //buffer.append("\e&y1O"); // Raster direction (1 = up)
    
    // reset pcl
    buffer.append(PCL_RESET + PJL_SWITCH_LANG_TO_PCL);
    //buffer.append("\eE@PJL ENTER LANGUAGE=PCL\r\n");
    
    // set layout as portrait
    buffer.append(PCL_RASTER_ORIENTATION_PORTRAIT);
    //buffer.append("\e*r0F");
    
    // raster height command
    memset(tmp_str, 0, strlen(tmp_str));
    sprintf(tmp_str, PCL_PAGE_HEIGHT.c_str(), int(machine_profile.height * (out_conf.dpi/MM_PER_INCH)));
    buffer.append(tmp_str);
    
    // raster width command
    memset(tmp_str, 0, strlen(tmp_str));
    sprintf(tmp_str, PCL_PAGE_WIDTH.c_str(), int(machine_profile.width * (out_conf.dpi/MM_PER_INCH)));
    buffer.append(tmp_str);

    // start raster param part
    buffer.append(PCL_RASTER_START);
    //buffer.append("\e*r1A"); // Start Raster Graphics Command 0 or 1
    
    // set raster power
    memset(tmp_str, 0, strlen(tmp_str));
    sprintf(tmp_str, PCL_RASTER_POWER.c_str(), out_conf.ras_power);
    buffer.append(tmp_str);
    //buffer.append("\e&y100P");
    
    // set raster speed
    memset(tmp_str, 0, strlen(tmp_str));
    sprintf(tmp_str, PCL_RASTER_SPEED.c_str(), out_conf.ras_speed);
    buffer.append(tmp_str);
//    buffer.append("\e&z100S");

    // raster frequency
    int freq = 0;
    if(machine_profile.name == ofxEpilog::FUSION_32.name)
    {
        freq = floor(output_conf.vec_freq);
    }
    else if(machine_profile.name == ofxEpilog::MINI_18.name)
    {
        freq = floor((output_conf.vec_freq / (double)100.0) * ofxEpilog::MINI_18.max_freq);
    }
    memset(tmp_str, 0, strlen(tmp_str));
    sprintf(tmp_str, PCL_RASTER_FREQ.c_str(), freq);
    buffer.append(tmp_str + HPGL_CMD_DELIMITER);
    
    return buffer;
}

ofBuffer ofxEpilog::createPayloadFooter()
{
    ofBuffer buffer;
    
    // go to home position
    buffer.append(HPGL_VECTOR_PEN_UP + "0,0" + HPGL_CMD_DELIMITER);
    
    // reset pcl status
    buffer.append(HPGL_END + HPGL_START + HPGL_VECTOR_PEN_UP + PCL_RESET);
    
    // add pjl footer
    buffer.append(PCL_UNIVERSAL_EXIT_LANG + PJL_EOJ);
    
//    const string PJL_FOOTER                          = "\e%-12345X@PJL EOJ \r\n";
    
    // close session
    for(int i = 0; i < 4090; i++)
    {
        buffer.append(" ");
    }
    
    buffer.append(getMachineProfile().name+"\n");
    return buffer;
}

//
// TODO(ando@ycam.jp): finish implementation
//
ofBuffer ofxEpilog::createPayloadRasterBody(ofImage raster_img, OutputConfig &out_conf)
{
    ofLog(OF_LOG_WARNING) << "bool ofxEpilog::createPayloadRasterBody(ofImage raster_img, OutputConfig &out_conf) is not implemented yet.";
    
    // payload buffer
    ofBuffer buffer;
    
    // add raster image part
    if(raster_img.isAllocated())
    {
        //
        // convert to PCL raster format from ofImage
        //
    }
    return buffer;
}

ofBuffer ofxEpilog::createPayloadVectorParams(OutputConfig &out_conf)
{
    ofBuffer buffer;
    char tmp_str[256] = {0};

    if(machine_profile.name == ofxEpilog::FUSION_32.name)
    {
        buffer.append(HPGL_UNKNOWN + HPGL_CMD_DELIMITER);
        
        // dynamic focus control
        if(out_conf.use_auto_fucus)
        {
            // offset
            // convert unit 0.001 inch
            memset(tmp_str, 0, strlen(tmp_str));
            sprintf(tmp_str, HPGL_VECTOR_DEFAULT_OFFSET_Z.c_str(), (int)round((1/(MM_PER_INCH/1000)) * output_conf.z_offset));
            buffer.append(tmp_str + HPGL_CMD_DELIMITER);
            
            // thickness
            // convert unit 0.001 inch
            memset(tmp_str, 0, strlen(tmp_str));
            sprintf(tmp_str, HPGL_VECTOR_DEFAULT_THICKNESS_Z.c_str(), (int)round((1/(MM_PER_INCH/1000)) * (output_conf.z_thickness)));
            buffer.append(tmp_str + HPGL_CMD_DELIMITER);
        }
        
        // freq
        memset(tmp_str, 0, strlen(tmp_str));
        sprintf(tmp_str, HPGL_VECTOR_FREQ_FUSION.c_str(), output_conf.vec_freq);
        buffer.append(tmp_str + HPGL_CMD_DELIMITER);
        
        // enable speed, power comp.
        memset(tmp_str, 0, strlen(tmp_str));
        sprintf(tmp_str, HPGL_VECTOR_SPEED_CMP.c_str(), 1);
        buffer.append(tmp_str + HPGL_CMD_DELIMITER);
        
        memset(tmp_str, 0, strlen(tmp_str));
        sprintf(tmp_str, HPGL_VECTOR_POWER_CMP.c_str(), 1);
        buffer.append(tmp_str + HPGL_CMD_DELIMITER);
    }
    else if(machine_profile.name == ofxEpilog::MINI_18.name)
    {
        // freq
        int freq = floor((output_conf.vec_freq / (double)100.0) * ofxEpilog::MINI_18.max_freq);
        memset(tmp_str, 0, strlen(tmp_str));
        sprintf(tmp_str, HPGL_VECTOR_FREQ_MINI.c_str(), freq);
        //sprintf(tmp_str, HPGL_VECTOR_FREQ_MINI.c_str(), output_conf.vec_freq);
        buffer.append(tmp_str + HPGL_CMD_DELIMITER);
    }
    
    // power
    memset(tmp_str, 0, strlen(tmp_str));
    sprintf(tmp_str, HPGL_VECTOR_POWER.c_str(), output_conf.vec_power);
    buffer.append(tmp_str + HPGL_CMD_DELIMITER);
    
    // speed
    memset(tmp_str, 0, strlen(tmp_str));
    sprintf(tmp_str, HPGL_VECTOR_SPEED.c_str(), output_conf.vec_speed);
    buffer.append(tmp_str + HPGL_CMD_DELIMITER);
    
    return buffer;
}

ofBuffer ofxEpilog::createPayloadVectorBody(ofPolyline vector_vertexes, OutputConfig &out_conf)
{
    ofBuffer buffer;
    
    // add vector vertexes
    if(vector_vertexes.size() > 0)
    {
        for(int i=0; i<vector_vertexes.size(); i++)
        {
            // update parameters of vector process
            buffer.append(createPayloadVectorParams(out_conf));
            
            if(out_conf.enable_laser_emit && i != 0)
                buffer.append(HPGL_VECTOR_PEN_DOWN); // PD
            else
                buffer.append(HPGL_VECTOR_PEN_UP); // PU
            
            // convert to HPGL vector format from ofPolyline (unit:inch)
            ofPoint p = vector_vertexes[i];
            buffer.append(ofToString( floor((out_conf.dpi/MM_PER_INCH) * p.x) ) + "," + ofToString( floor((out_conf.dpi/MM_PER_INCH) * p.y)) + ";");
        }
    }
    return buffer;
}

ofBuffer ofxEpilog::createPayloadCombinedBody(ofImage raster_img, ofPolyline vector_vertexes)
{
    if(!raster_img.isAllocated())
        return;
    if(vector_vertexes.size() == 0)
        return;

    ofBuffer buffer;
    buffer.append(createPayloadRasterBody(raster_img, getOutputConfig()));
    
    // end raster part regardless of it's empty
    buffer.append(PCL_RASTER_END);
    
    // begin HPGL commands (vector part)
    buffer.append(HPGL_START);
    buffer.append(HPGL_VECTOR_INIT + HPGL_CMD_DELIMITER);

    buffer.append(createPayloadVectorBody(vector_vertexes, getOutputConfig()));

    return buffer;
}

//
// send ofPolyline to laser cutter
//
bool ofxEpilog::send(ofPolyline vector_vertexes)
{
    if(vector_vertexes.size() == 0)
        return false;
    
    ofBuffer buffer;
    buffer.append(createPayloadHeader(getMachineProfile(), getOutputConfig()));
    
    // end raster part regardless of it's empty
    buffer.append(PCL_RASTER_END);
    
    // begin HPGL commands (vector part)
    buffer.append(HPGL_START);
    buffer.append(HPGL_VECTOR_INIT + HPGL_CMD_DELIMITER);

    // create vector part and append to the buffer
    buffer.append(createPayloadVectorBody(vector_vertexes, getOutputConfig()));
    
    if(!keep_alive)
        buffer.append(createPayloadFooter()); // end the session
    
    if(pjl_file.get() != NULL)
        pjl_file->writeFromBuffer(buffer);
    
    return tcp_client.sendRaw(buffer);
}

bool ofxEpilog::send(ofPath path)
{
    //
    // TODO(ando@ycam.jp): test again and finish implementation
    //
    
    if (!tcp_client.isConnected())
    {
        return false;
    }
    
    ofBuffer buffer;
    
    vector<ofPolyline> line = path.getOutline();
    for (int i=0; i<line.size(); i++)
    {
        if(!send(line[i]))
            return false;
    }
    return true;
}

bool ofxEpilog::send(ofImage img, int w_mm, int h_mm)
{
    //
    // TODO(ando@ycam.jp): finish implementation
    //
    
    if(!img.isAllocated())
        return false;
    
    if(w_mm <= 0 || h_mm <= 0)
        return false;

    ofLog(OF_LOG_WARNING) << "bool ofxEpilog::send(ofImage img, int w_mm, int h_mm) is not implemented yet.";
    return false;
    
    //
    // dummy implementation
    //
    ofBuffer buffer;
    buffer.append(createPayloadHeader(getMachineProfile(), getOutputConfig()));
    
    // create raster part
    buffer.append(createPayloadRasterBody(img, getOutputConfig()));
    
    // end raster part regardless of it's empty
    buffer.append(PCL_RASTER_END);
    
    // begin HPGL commands (vector part)
    buffer.append(HPGL_START);
    buffer.append(HPGL_VECTOR_INIT + HPGL_CMD_DELIMITER);
    
    if(!keep_alive)
        buffer.append(createPayloadFooter()); // end the session
    
    if(pjl_file.get() != NULL)
        pjl_file->writeFromBuffer(buffer);
    
    return tcp_client.sendRaw(buffer);
}

void ofxEpilog::disconnect()
{
    if(tcp_client.isConnected())
    {
        if(keep_alive)
        {
            ofBuffer buffer;
            buffer.append(createPayloadFooter());
            
            // for debug
            ofLog(OF_LOG_VERBOSE) << buffer.getText();
            
            if(pjl_file.get() != NULL)
                pjl_file->writeFromBuffer(buffer);
            
            tcp_client.sendRaw(buffer);
        }
        tcp_client.close();
    }
}

bool ofxEpilog::isConnected()
{
    return tcp_client.isConnected();
}

void ofxEpilog::setIPAddress(string ip)
{
    // TODO(ando@ycam.jp): add validation
    ip_address = ip;
}

void ofxEpilog::setMachineProfile(MachineProfile m_profile)
{
    // TODO(ando@ycam.jp): add validation
    machine_profile = m_profile;
}

void ofxEpilog::setOutputConfig(OutputConfig out_conf)
{
    // TODO(ando@ycam.jp): add validation
    output_conf = out_conf;
}
