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

#include "ofxEpilog.h"


//
// Default profiles from tech spechs
// https://www.epiloglaser.com/products/fusion-laser-series.htm#techspecs
// https://www.epiloglaser.com/products/legend-laser-series.htm#techspecs
// http://support.epiloglaser.com/article/8205/42831/focus-lens-101
//
// TODO : load from configuration file
//
// 2 inch lens
const struct MachineProfile ofxEpilog::FUSION_32 =
{
    "Fusion_32", 813, 508, -12.7, 76.2, 0, 177.8, 75, 1200, 0, 100, true, 50.8
};
// 1.5 inch lens
const struct MachineProfile ofxEpilog::MINI_18 =
{
    "Mini_18", 457, 305, 0, 0, 0, 177.8, 75, 1200, 0, 5000, false, 38.1
};

ofxEpilog::ofxEpilog()
{
    ip_address = "000.000.000.000"; // dummy
    hostname = "UNKOWN HOSTNAME";
    
    char localhost[128] = "";
    gethostname(localhost, sizeof(localhost));
    hostname = string(localhost);
    
    is_live_mode = false;
    
    cout << "ofxEpilog::ofxEpilog() hostname=" << hostname << endl;
}

ofxEpilog::ofxEpilog(const ofxEpilog &obj)
{
    ofLog(OF_LOG_VERBOSE, "ofxEpilog::ofxEpilog(const ofxEpilog &obj)");
}

ofxEpilog::~ofxEpilog()
{
    if(tcp_client.isConnected())
        disconnect();
    
    //model.clear();
    
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

//void ofxEpilog::threadedFunction()
//{
//    thread.setName("ofxEpilog_" + thread.name());
//}


bool ofxEpilog::connect(string ip, bool live)
{
    setIPAddress(ip);
    
    ofLog(OF_LOG_VERBOSE, "ofxEpilog::connect(): start cnnecting to the laser cutter.");
    
    if(tcp_client.isConnected())
    {
        ofLog(OF_LOG_WARNING, "ofxEpilog: the connection is created already.You should disconnect first." );
        disconnect();
    }
    
    is_live_mode = live;
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
        stream << "\003" << "125899906843000" << " dfA" << hostname << "\n"; // <- buffer size is correct?
        tcp_client.sendRaw(stream.str());
        
        tcp_client.receiveRawBytes(&response, 1);
    }
    else
        return false;
    
    ofLog() << "ofxEpilog::connect(): LPR handshake complete.";
    
    return true;
}

ofBuffer ofxEpilog::getPCLHeader()
{
    ofBuffer buffer;
    
    std::ostringstream stream;
    stream.str("");
    stream.clear();
    char tmp_str[100] = {0};
    
    
    if(machine_profile.name == ofxEpilog::FUSION_32.name)
    {
        stream << PCL_COLOR_COMPONENT_ONE;
        stream << PCL_UNKNOWN_CMD1;
        
        sprintf(tmp_str, PCL_TIME_STAMP.c_str(), ofGetTimestampString("%Y%m%d%H%M%S").c_str());
        stream << tmp_str;
        
        stream << PCL_UNKNOWN_CMD2;
        stream << PCL_UNKNOWN_CMD3;
        stream << PCL_UNKNOWN_CMD4;
    }
    stream << PCL_OFFSET_X;
    stream << PCL_OFFSET_Y;
    
    memset(tmp_str, 0, strlen(tmp_str));
    sprintf(tmp_str, PCL_PRINT_RESOLUTION.c_str(), output_conf.dpi);
    stream << tmp_str;
    
    stream << PCL_RASTER_ORIENTATION;
    stream << PCL_UNKNOWN_CMD5;
    
    buffer.append(stream.str());
    return buffer;
}

ofBuffer ofxEpilog::getPCLDefaultOutputConfig()
{
    ofBuffer buffer;
    
    std::ostringstream stream;
    stream.str("");
    stream.clear();
    char tmp_str[100] = {0};
    
    stream << PCL_UNKNOWN_CMD6;
    
    memset(tmp_str, 0, strlen(tmp_str));
    sprintf(tmp_str, PCL_RASTER_POWER.c_str(), output_conf.ras_power);
    stream << tmp_str;
    
    memset(tmp_str, 0, strlen(tmp_str));
    sprintf(tmp_str, PCL_RASTER_SPEED.c_str(), output_conf.ras_speed);
    stream << tmp_str;

    memset(tmp_str, 0, strlen(tmp_str));
    sprintf(tmp_str, PCL_PAGE_WIDTH.c_str(), int(machine_profile.width * (double)((double)output_conf.dpi/(double)MM_PER_INCH)));
    stream << tmp_str;

    memset(tmp_str, 0, strlen(tmp_str));
    sprintf(tmp_str, PCL_PAGE_HEIGHT.c_str(), int(machine_profile.height * (double)((double)output_conf.dpi/(double)MM_PER_INCH)));
    stream << tmp_str;

    stream << PCL_RASTER_COMPRESSION2.c_str();

    memset(tmp_str, 0, strlen(tmp_str));
    sprintf(tmp_str, PCL_RASTER_DIRECTION.c_str(), 0);
    stream << tmp_str;
    
    if(machine_profile.name == ofxEpilog::FUSION_32.name)
    {
        memset(tmp_str, 0, strlen(tmp_str));
        // raster freq
        sprintf(tmp_str, PCL_RASTER_FREQ.c_str(), output_conf.ras_freq);
        //ofLog() << tmp_str;
        stream << tmp_str;
        
        if(output_conf.use_auto_fucus)
        {
            memset(tmp_str, 0, strlen(tmp_str));
            // offset
            // convert unit 0.001 inch
            sprintf(tmp_str, PCL_RASTER_DEFAULT_OFFSET_Z.c_str(),
                    (int)round((1/(MM_PER_INCH/1000)) * output_conf.z_offset));
            stream << tmp_str;
            
            memset(tmp_str, 0, strlen(tmp_str));
            // thickness
            // convert unit 0.001 inch
            sprintf(tmp_str, PCL_RASTER_DEFAULT_THICKNESS_Z.c_str(),
                    (int)round((1/(MM_PER_INCH/1000)) * (output_conf.z_thickness)));
            stream << tmp_str;
        }
    }
    
    memset(tmp_str, 0, strlen(tmp_str));
    sprintf(tmp_str, PCL_RASTER_AIR_ASSIST.c_str(), 2);
    stream << tmp_str;
    
    buffer.append(stream.str());
    return buffer;
}

ofBuffer ofxEpilog::getPCLColorMappingOutputConfig(OutputConfig config)
{
    ofBuffer buffer;
    
    std::ostringstream stream;
    stream.str("");
    stream.clear();
    char tmp_str[100] = {0};
    
    stream << PCL_UNKNOWN_CMD6;
    
    memset(tmp_str, 0, strlen(tmp_str));
    sprintf(tmp_str, PCL_RASTER_POWER.c_str(), config.ras_power);
    stream << tmp_str;
    
    memset(tmp_str, 0, strlen(tmp_str));
    sprintf(tmp_str, PCL_RASTER_SPEED.c_str(), config.ras_speed);
    stream << tmp_str;
    
    
    if(machine_profile.name == ofxEpilog::FUSION_32.name)
    {
        memset(tmp_str, 0, strlen(tmp_str));
        // raster freq
        sprintf(tmp_str, PCL_RASTER_FREQ.c_str(), config.ras_freq);
        //ofLog() << tmp_str;
        stream << tmp_str;
        
        if(output_conf.use_auto_fucus)
        {
            memset(tmp_str, 0, strlen(tmp_str));
            // offset
            // convert unit 0.001 inch
            sprintf(tmp_str, PCL_RASTER_COLORMAP_OFFSET_Z.c_str(),
                    (int)round((1/(MM_PER_INCH/1000)) * config.z_offset));
            stream << tmp_str;
            
            memset(tmp_str, 0, strlen(tmp_str));
            // thickness
            // convert unit 0.001 inch
            sprintf(tmp_str, PCL_RASTER_COLORMAP_THICKNESS_Z.c_str(),
                    (int)round((1/(MM_PER_INCH/1000)) * (config.z_thickness)));
            stream << tmp_str;
        }
    }
    
    memset(tmp_str, 0, strlen(tmp_str));
    sprintf(tmp_str, PCL_RASTER_AIR_ASSIST.c_str(), 1);
    stream << tmp_str;
    
    
    buffer.append(stream.str());
    return buffer;
}

ofBuffer ofxEpilog::getPCLRasterBody(ofImage img, int w_mm, int h_mm, bool add_start_header, bool add_end_footer)
{
    
    ofBuffer buffer;
    
    std::ostringstream stream;
    stream.str("");
    stream.clear();
    char tmp_str[100] = {0};
    
    if (add_start_header) {
        stream << PCL_RASTER_START;
    }
    
    {
        //
        // TODO: finish implementation, rasterize ofImage
        //
        ofLog(OF_LOG_WARNING) << "getPCLRasterBody():: not implemented!";
    }
    
    if (add_end_footer) {
        stream << PCL_RASTER_END;
    }
    
    buffer.append(stream.str());
    return buffer;
}

ofBuffer ofxEpilog::getHPGLHeader()
{
    ofBuffer buffer;
    
    std::ostringstream stream;
    stream.str("");
    stream.clear();
    char tmp_str[100] = {0};
    
    stream << PCL_RASTER_END;
    stream << HPGL_START;
    stream << HPGL_VECTOR_INIT + HPGL_CMD_DELIMITER;
    
    buffer.append(stream.str());
    return buffer;
}

ofBuffer ofxEpilog::getHPGLDefaultOutputConfig()
{
    ofBuffer buffer;
    
    std::ostringstream stream;
    stream.str("");
    stream.clear();
    char tmp_str[100] = {0};
    
    if(machine_profile.name == ofxEpilog::FUSION_32.name)
    {
        stream << HPGL_UNKNOWN + HPGL_CMD_DELIMITER;

        if(output_conf.use_auto_fucus)
        {
            memset(tmp_str, 0, strlen(tmp_str));
            // offset
            // convert unit 0.001 inch
            sprintf(tmp_str, HPGL_VECTOR_DEFAULT_OFFSET_Z.c_str(),
                    (int)round((1/(MM_PER_INCH/1000)) * output_conf.z_offset));
            stream << tmp_str + HPGL_CMD_DELIMITER;
            
            memset(tmp_str, 0, strlen(tmp_str));
            // thickness
            // convert unit 0.001 inch
            sprintf(tmp_str, HPGL_VECTOR_DEFAULT_THICKNESS_Z.c_str(),
                    (int)round((1/(MM_PER_INCH/1000)) * (output_conf.z_thickness)));
            stream << tmp_str + HPGL_CMD_DELIMITER;
        }
        
        memset(tmp_str, 0, strlen(tmp_str));
        // freq
        sprintf(tmp_str, HPGL_VECTOR_FREQ_FUSION.c_str(), output_conf.vec_freq);
        stream << tmp_str + HPGL_CMD_DELIMITER;
        
        // enable speed, power comp.
        memset(tmp_str, 0, strlen(tmp_str));
        sprintf(tmp_str, HPGL_VECTOR_SPEED_CMP.c_str(), 1);
        stream << tmp_str + HPGL_CMD_DELIMITER;
        
        memset(tmp_str, 0, strlen(tmp_str));
        sprintf(tmp_str, HPGL_VECTOR_POWER_CMP.c_str(), 1);
        stream << tmp_str + HPGL_CMD_DELIMITER;
    }
    else if(machine_profile.name == ofxEpilog::MINI_18.name)
    {
        memset(tmp_str, 0, strlen(tmp_str));
        // freq
        sprintf(tmp_str, HPGL_VECTOR_FREQ_MINI.c_str(), output_conf.vec_freq);
        stream << tmp_str + HPGL_CMD_DELIMITER;

    }
    memset(tmp_str, 0, strlen(tmp_str));
    // power
    sprintf(tmp_str, HPGL_VECTOR_POWER.c_str(), output_conf.vec_power);
    stream << tmp_str + HPGL_CMD_DELIMITER;
    
    memset(tmp_str, 0, strlen(tmp_str));
    // speed
    sprintf(tmp_str, HPGL_VECTOR_SPEED.c_str(), output_conf.vec_speed);
    stream << tmp_str + HPGL_CMD_DELIMITER;

    memset(tmp_str, 0, strlen(tmp_str));
    sprintf(tmp_str, HPGL_VECTOR_AIR_ASSIST.c_str(), 2);
    stream << tmp_str + HPGL_CMD_DELIMITER;

    buffer.append(stream.str());
    return buffer;
}

ofBuffer ofxEpilog::getHPGLColorMappingOutputConfig(OutputConfig config)
{
    ofBuffer buffer;
    
    std::ostringstream stream;
    stream.str("");
    stream.clear();
    char tmp_str[100] = {0};
    
    if(machine_profile.name == ofxEpilog::FUSION_32.name)
    {
        stream << HPGL_UNKNOWN + HPGL_CMD_DELIMITER;
        
        if(output_conf.use_auto_fucus)
        {
            memset(tmp_str, 0, strlen(tmp_str));
            // offset
            // convert unit 0.001 inch
            sprintf(tmp_str, HPGL_VECTOR_COLORMAP_OFFSET_Z.c_str(),
                    (int)round((1/(MM_PER_INCH/1000)) * config.z_offset));
            stream << tmp_str + HPGL_CMD_DELIMITER;
            
            memset(tmp_str, 0, strlen(tmp_str));
            // thickness
            // convert unit 0.001 inch
            sprintf(tmp_str, HPGL_VECTOR_COLORMAP_THICKNESS_Z.c_str(),
                    (int)round((1/(MM_PER_INCH/1000)) * (config.z_thickness)));
            stream << tmp_str + HPGL_CMD_DELIMITER;
        }
        
        memset(tmp_str, 0, strlen(tmp_str));
        // freq
        sprintf(tmp_str, HPGL_VECTOR_FREQ_FUSION.c_str(), config.vec_freq);
        stream << tmp_str + HPGL_CMD_DELIMITER;
    }
    else if(machine_profile.name == ofxEpilog::MINI_18.name)
    {
        memset(tmp_str, 0, strlen(tmp_str));
        // freq
        sprintf(tmp_str, HPGL_VECTOR_FREQ_MINI.c_str(), config.vec_freq);
        stream << tmp_str + HPGL_CMD_DELIMITER;
        
    }
    memset(tmp_str, 0, strlen(tmp_str));
    // power
    sprintf(tmp_str, HPGL_VECTOR_POWER.c_str(), config.vec_power);
    stream << tmp_str + HPGL_CMD_DELIMITER;
    
    memset(tmp_str, 0, strlen(tmp_str));
    // speed
    sprintf(tmp_str, HPGL_VECTOR_SPEED.c_str(), config.vec_speed);
    stream << tmp_str + HPGL_CMD_DELIMITER;
    
    memset(tmp_str, 0, strlen(tmp_str));
    sprintf(tmp_str, HPGL_VECTOR_AIR_ASSIST.c_str(), 1);
    stream << tmp_str + HPGL_CMD_DELIMITER;
    
    buffer.append(stream.str());
    return buffer;
    
}

ofBuffer ofxEpilog::getHPGLVectorBody(ofPolyline line, OutputConfig config, int resample_count)
{
    ofBuffer buffer;
    std::ostringstream stream;
    string tmp = "";
    double x,y,z = 0;
    
    //vector<ofPoint> vertices = line.getVertices();
    
    if(config.use_bezier_cmd)
    {
        ofLog(OF_LOG_WARNING) << "getHPGLVectorBody(ofPolyline line):: bezer output is not implemented!";
        //
        // TODO: finish implementation
        // PDBZ x1,y1,x2,y2,x3,y3 ...
        // HPGL_VECTOR_PEN_DOWN_BEZIER
        //
    }
    else
    {
        vector<ofPoint> verts;
        
        if(resample_count < 2)
        {
            verts = line.getVertices();
        }
        else
        {
            //
            // FIXME: check curvature
            //
            verts = line.getResampledByCount(resample_count).getVertices();
        }
        //vector<ofPoint> resampled = line.getResampledByCount(resample_count).getVertices();
        
        if(verts.size()==0)
        {
            return buffer;
        }
        
        x = ((double)config.dpi/(double)MM_PER_INCH) * verts[0].x;
        y = ((double)config.dpi/(double)MM_PER_INCH) * verts[0].y;
        z = verts[0].z;//((double)config.dpi/(double)MM_PER_INCH) * verts[0].z;
        y *= -1; // for illustrator dxf
        config.z_offset = z;
        stream << getHPGLColorMappingOutputConfig(config);
        
        stream << HPGL_VECTOR_PEN_UP << floor(x) << ',' << floor(y) << HPGL_CMD_DELIMITER; // PUx,y;
//        stream << HPGL_VECTOR_PEN_UP << x << ',' << y << HPGL_CMD_DELIMITER; // PUx,y;
        tmp = stream.str();
        
        buffer.append(tmp.c_str());
//        buffer.append(HPGL_VECTOR_PEN_DOWN);
        
        for(int i=0; i<verts.size()-1; i++)
        {
            stream.str("");
            stream.clear();
            tmp.erase();
            
            
            //
            // FIXME: calculation is wrong?
            //
            double x1 = ((double)config.dpi/(double)MM_PER_INCH) * verts[i].x;
            double y1 = ((double)config.dpi/(double)MM_PER_INCH) * verts[i].y;
            double z1 = verts[i].z;//((double)config.dpi/(double)MM_PER_INCH) * verts[i].z;
            y1 *= -1;// for illustrator dxf
            double x2 = ((double)config.dpi/(double)MM_PER_INCH) * verts[i+1].x;
            double y2 = ((double)config.dpi/(double)MM_PER_INCH) * verts[i+1].y;
            //double z2 = verts[i+1].z;//((double)config.dpi/(double)MM_PER_INCH) * verts[i].z;
            y2 *= -1;// for illustrator dxf

            config.z_offset = z1;
            stream << getHPGLColorMappingOutputConfig(config);
            //stream << HPGL_VECTOR_PEN_UP << floor(x) << ',' << floor(y) << HPGL_CMD_DELIMITER; // PUx,y;
            stream << HPGL_VECTOR_PEN_DOWN;
            
            ofLog(OF_LOG_VERBOSE) << "x = " << (config.dpi / MM_PER_INCH) << " * " << verts[i].x << " , " << floor(x);
            ofLog(OF_LOG_VERBOSE) << "y = " << (config.dpi / MM_PER_INCH) << " * " << verts[i].y << " , " << floor(y);
            
            //stream << HPGL_VECTOR_PEN_DOWN << floor(x) << ',' << floor(y); // PDx,y{,x,y...};
            //if(i>0)
            //    stream << ',';
            
            stream << floor(x1) << ',' << floor(y2) << ','; // x,y{,x,y...};
            stream << floor(x2) << ',' << floor(y2) << HPGL_CMD_DELIMITER; // x,y{,x,y...};
//            stream << x << ',' << y; // x,y{,x,y...};
            tmp = stream.str();
            buffer.append(tmp.c_str(), tmp.length());
        }
        //buffer.append(HPGL_CMD_DELIMITER);
        
        stream.str("");
        stream.clear();
        tmp.erase();
        
//        stream << HPGL_VECTOR_PEN_UP << floor(x) << ',' << floor(y) << HPGL_CMD_DELIMITER; // PUx,y;
        tmp = stream.str();
        buffer.append(tmp.c_str(), tmp.length());
    }
    
    return buffer;
}

//ofBuffer ofxEpilog::getHPGLVectorBody(ofPath path, OutputConfig config, bool use_bezier_output, int resample_count)
//{
//    ofBuffer buffer;
//    
//    vector<ofPolyline> line = path.getOutline();
//    for (int i=0; i<line.size(); i++) {
//        line[i];
//        buffer.append(getHPGLVectorBody(line[i], config, use_bezier_output, resample_count));
//    }
//    return buffer;
//}


ofBuffer ofxEpilog::getHPGLVectorBody(ofPath path, OutputConfig config, int resample_count)
{
    ofBuffer buffer;
    
    vector<ofPolyline> line = path.getOutline();
    for (int i=0; i<line.size(); i++)
    {
        //line[i];
        buffer.append(getHPGLVectorBody(line[i], config, resample_count));
    }
    return buffer;
}


ofBuffer ofxEpilog::getHPGLFooter()
{
    return ofBuffer(HPGL_END + HPGL_START + HPGL_VECTOR_PEN_UP + PCL_RESET);
}

ofBuffer ofxEpilog::getPCLFooter()
{
    ofBuffer buffer;
    buffer.append(PJL_FOOTER);
    
    // close session
    for(int i = 0; i < 4090; i++)
    {
        buffer.append(" ");
        //buffer.append(0x0);
    }
    buffer.append(machine_profile.name+"\n");
    
    return buffer;
}

bool ofxEpilog::send(ofPolyline line, OutputConfig config)
{
    //
    // TODO: finish implementation
    //
    ofLog(OF_LOG_WARNING) << "bool ofxEpilog::send(ofPolyline line):: not implemented!";
    
    //if (!tcp_client.isConnected())
    {
        //return false;
    }
    
    ofBuffer buffer;
    
    //
    // TODO: check header sent already
    //
    std::ostringstream stream;
    stream.str("");
    stream.clear();
    char tmp_str[100] = {0};
    
    sprintf(tmp_str, PJL_HEADER.c_str(), ofToString("ofx_" + ofToString(ofGetFrameNum())).c_str());
    
    //sprintf(tmp_str, PJL_HEADER.c_str(), ofGetTimestampString("%H%M%S").c_str());

    buffer.append(tmp_str);

    //ofLog() << getPCLHeader();
    buffer.append(getPCLHeader());
    
    ofLog() << getPCLDefaultOutputConfig();
    buffer.append(getPCLDefaultOutputConfig());
    
    //ofLog() << getPCLColorMappingOutputConfig(output_conf);
    //ofLog() << getPCLRasterBody(ofImage(), 100, 200);
    //ofLog() << getHPGLHeader();
    buffer.append(getHPGLHeader());
    
    //ofLog() << getHPGLDefaultOutputConfig();
    buffer.append(getHPGLDefaultOutputConfig());
    
    //ofLog() << getHPGLColorMappingOutputConfig(output_conf);
    buffer.append(getHPGLColorMappingOutputConfig(output_conf));
    
    //ofLog() << getHPGLVectorBody(line, config);
    buffer.append(getHPGLVectorBody(line, config));
    
    // why not working?
    //if(!is_live_mode)
    {
        //ofLog() << getHPGLFooter();
        buffer.append(getHPGLFooter());
        
        //ofLog() << getPCLFooter();
        buffer.append(getPCLFooter());
        
        ofLog() << buffer.getText();
    }
    
    if(pjl_file.get() != NULL)
        pjl_file->writeFromBuffer(buffer);

    char response;
    tcp_client.sendRaw(buffer);
    tcp_client.receiveRawBytes(&response, 1);
    if(response != 0)
        return false;
    
    return true;
}

bool ofxEpilog::send(ofPath path, OutputConfig config)
{
    //
    // TODO: finish implementation
    //
    ofLog(OF_LOG_WARNING) << "ofxEpilog::send(ofPath path):: is WIP";
    
    //if (!tcp_client.isConnected())
    {
        //return false;
    }
    
    ofBuffer buffer;
    
    //
    // TODO: check header sent already
    //
    std::ostringstream stream;
    stream.str("");
    stream.clear();
    char tmp_str[100] = {0};
    
    if(pjl_header_sent == false)
    {
        sprintf(tmp_str, PJL_HEADER.c_str(), ofToString("ofxEpilog" + ofToString(ofGetFrameNum())).c_str());
        buffer.append(tmp_str);
        
        buffer.append(getPCLHeader());
    }
    
    ofLog() << getPCLDefaultOutputConfig();
    buffer.append(getPCLDefaultOutputConfig());
    
    
    //ofLog() << getPCLColorMappingOutputConfig(output_conf);
    //ofLog() << getPCLRasterBody(ofImage(), 100, 200);
    //ofLog() << getHPGLHeader();
    buffer.append(getHPGLHeader());
    
    buffer.append(getHPGLDefaultOutputConfig());
    
    // TODO: give OutputConfig[] array
    //buffer.append(getHPGLColorMappingOutputConfig(output_conf));
    
    buffer.append(getHPGLVectorBody(path, config));
    
    if(is_live_mode)
    {
        // if live mode is enabled we dont send PJL footer
        char response;
        tcp_client.sendRaw(buffer);
        tcp_client.receiveRawBytes(&response, 1);
        if(response != 0)
        {
            pjl_header_sent = false;
            return false;
        }
        else
        {
            pjl_header_sent = true;
            return true;
        }
    }
    
    buffer.append(getHPGLFooter());
    
    buffer.append(getPCLFooter());
    
    ofLog(OF_LOG_VERBOSE) << buffer.getText();
    
    if(pjl_file.get() != NULL)
        pjl_file->writeFromBuffer(buffer);
    
    char response;
    tcp_client.sendRaw(buffer);
    tcp_client.receiveRawBytes(&response, 1);
    if(response != 0)
        return false;
    
    pjl_header_sent = false;
    return true;
}

bool ofxEpilog::send(ofImage img, int w_mm, int h_mm, OutputConfig config, bool add_start_header, bool add_end_footer)
{
    //
    // TODO: finish implementation
    //
    ofLog(OF_LOG_WARNING) << "bool ofxEpilog::send(ofImage img):: not implemented!";
    return false;
}

//
// TODO: finish implementation
//
bool ofxEpilog::send(ofXml gml_file, int w_mm, int h_mm, OutputConfig config)
{
    ofLog(OF_LOG_WARNING) << "bool ofxEpilog::send(ofXml gml_file, int w_mm, int h_mm, OutputConfig config):: not implemented!";

    return false;
    
    ofBuffer buffer;
    ofXml gml;
    
    /*
     GML minimum format
     <gml spec='1.0 (minimum)'>
        <tag>
            <drawing>
                <stroke>
                <pt>
                    <x>0.0</x>
                    <y>0.0</y>
                    <!-- <t></t> -->
                    <!-- <time></time> -->
                </pt>
            </stroke>
            </drawing>
        </tag>
     </gml>
     */
    
    /*
    std::ostringstream stream;
    string tmp;
    
    if(gml.exists("//tag/drawing/stroke"))
    {
        gml.setTo("//tag/drawing[0]");
        int strokes = gml.getNumChildren();
        if(strokes == 0)
        {
            ofLog(OF_LOG_WARNING) << "No stroke in this GML.";
            return false;
        }
        
        gml.setTo("stroke[0]");
        for(int i=0; i<strokes; i++)
        {
            int points = gml.getNumChildren();
            gml.setTo("pt[0]");
            for(int j=0; j<points; j++)
            {
                float x = gml.getValue<float>("x");
                float y = gml.getValue<float>("y");
                
                //
                // TODO: Convert coordinates to the MM.
                //
                stream.str("");
                stream.clear();
                tmp.erase();
                
                if(j==0)
                {
                    stream << "PU" << floor(x) << ',' << floor(y) << ';'; // PUx,y;
                    tmp = stream.str();
                    buffer.append(tmp.c_str(), tmp.length());
                }
                
                stream << "PD" << floor(x) << ',' << floor(y);  // PDx,y{,x,y...};
                tmp = stream.str();
                buffer.append(tmp.c_str(), tmp.length());
                
                ofLog(OF_LOG_VERBOSE) << "GML::stroke["+ofToString(i)+"]::pt["+ofToString(j)+"] x="+ofToString(x)+" y="+ofToString(y);
                
                gml.setToSibling();
            }
            gml.setToParent();
            gml.setToSibling();
        }
    }
    else
    {
        ofLog(OF_LOG_ERROR) << "This file is not GML.";
        return false;
    }
    
    return true;
    */
}

void ofxEpilog::debug()
{

}

void ofxEpilog::disconnect()
{
    if(tcp_client.isConnected())
    {
        if(is_live_mode)
        {
            //
            // Finish live job
            //
            //std::ostringstream stream;
            //stream << "PU0,0;";
            //tcp_client.sendRaw(stream.str());
            //sendPCLVectorFooter();
            //sendPJLFooter();
            
            ofBuffer buffer;
            
            buffer.append(HPGL_CMD_DELIMITER + HPGL_VECTOR_PEN_UP + "0,0" + HPGL_CMD_DELIMITER);
            buffer.append(getHPGLFooter());
            buffer.append(getPCLFooter());
            
            ofLog(OF_LOG_VERBOSE) << buffer.getText();
            
            if(pjl_file.get() != NULL)
                pjl_file->writeFromBuffer(buffer);
            
            char response;
            tcp_client.sendRaw(buffer);
            tcp_client.receiveRawBytes(&response, 1);
        }
        tcp_client.close();
        pjl_header_sent = false;
    }
}

bool ofxEpilog::isConnected()
{
    return tcp_client.isConnected();
}

void ofxEpilog::setIPAddress(string ip)
{
    // TODO: validation
    ip_address = ip;
}

void ofxEpilog::setMachineProfile(MachineProfile m_profile)
{
    // TODO: validation
    machine_profile = m_profile;
}

void ofxEpilog::setOutputConfig(OutputConfig out_conf)
{
    // TODO: validation, range check
    output_conf = out_conf;
}

//bool ofxEpilog::loadModel(string f_name)
//{
//    model.clear();
//    return model.loadModel(f_name);
//}

/*
 // Not supporting thread feature yet
 void ofxEpilog::threadedFunction()
 {
 while(isThreadRunning())
 {
 if(tcp_client.isConnected())
 {
 
 //lock();
 //
 // manipulate shared resources
 //
 //unlock();
 
 }
 else
 {
 
 }
 }
 }
 */



// no longer used
//ofPtr<HPGLBuffer> HPGLBuffer::create(ofPixels pixelsRef, ofPoint offset, OutputConfig config)
//{
//    ofPtr<HPGLBuffer> buffer = ofPtr<HPGLBuffer>(new HPGLBuffer());
//
//    ofLog(OF_LOG_VERBOSE, "HPGLBuffer::create(ofPixels, OutputConfig) img.width=%d, img.height=%d", pixelsRef.getWidth(), pixelsRef.getHeight());
//
//    int w = pixelsRef.getWidth();
//    int h = pixelsRef.getHeight();
//    if(w == 0 && h == 0)
//        return buffer;
//
//    const int POINTS_PER_INCH = 72;
//    int basex = offset.x * config.dpi / POINTS_PER_INCH;
//    int basey = offset.y * config.dpi / POINTS_PER_INCH;
//
//    std::ostringstream stream;
//    string tmp = "";
//
//    unsigned char *pixels = pixelsRef.getPixels();
//
//
//    //
//    // TODO: finish implementation
//    //
//    switch (pixelsRef.getNumChannels()) {
//        case 1: // Grayscale
//            for(int y=0; y<h; y++)
//            {
//
//                for(int x=0; x<w; x++)
//                {
//                    int pixel = pixels[y*w+x];
//
//                    stream.str("");
//                    stream.clear();
//                    tmp.erase();
//                    stream << "\e*p" << basey + y << 'Y'; // *p#Y
//                    tmp = stream.str();
//                    buffer->append(tmp.c_str(), tmp.length());
//
//                    stream.str("");
//                    stream.clear();
//                    tmp.erase();
//                    stream << "\e*p" << basey << 'X'; // *p#X
//                    tmp = stream.str();
//                    buffer->append(tmp.c_str(), tmp.length());
//
//                    /*
//                     *b#A    # = Number of unpacked bytes (1 byte = 8 pixels)
//                     If this is negative, pixels are unpacked right-to-left
//                     */
//                    /*
//                     stream.str("");
//                     stream.clear();
//                     tmp.erase();
//                     stream << "\e*b" << "????" << 'A'; // *b#A
//                     tmp = stream.str();
//                     buffer->append(tmp.c_str(), tmp.length());
//                     */
//
//                    /*
//                     *b#W    # = Number of encoded bytes
//                     */
//                    /*
//                     stream.str("");
//                     stream.clear();
//                     tmp.erase();
//                     stream << "\e*b" << "????" << 'W'; // *b#W
//                     tmp = stream.str();
//                     buffer->append(tmp.c_str(), tmp.length());
//                     */
//                }
//            }
//            break;
//
//        case 3: // RGB
//            for(int y=0; y<h; y++)
//            {
//                for(int x=0; x<w; x++)
//                {
//                    int r = pixels[(y*w+x)*3];
//                    int g = pixels[(y*w+x)*3 +1];
//                    int b = pixels[(y*w+x)*3 +2];
//
//                    stream.str("");
//                    stream.clear();
//                    tmp.erase();
//                    stream << "\e*p" << basey + y << 'Y'; // *p#Y
//                    tmp = stream.str();
//                    buffer->append(tmp.c_str(), tmp.length());
//
//                    stream.str("");
//                    stream.clear();
//                    tmp.erase();
//                    stream << "\e*p" << basey << 'X'; // *p#X
//                    tmp = stream.str();
//                    buffer->append(tmp.c_str(), tmp.length());
//
//                    /*
//                     *b#A    # = Number of unpacked bytes (1 byte = 8 pixels)
//                     If this is negative, pixels are unpacked right-to-left
//                     */
//                    /*
//                     stream.str("");
//                     stream.clear();
//                     tmp.erase();
//                     stream << "\e*b" << "????" << 'A'; // *b#A
//                     tmp = stream.str();
//                     buffer->append(tmp.c_str(), tmp.length());
//                     */
//
//                    /*
//                     *b#W    # = Number of encoded bytes
//                     */
//                    /*
//                     stream.str("");
//                     stream.clear();
//                     tmp.erase();
//                     stream << "\e*b" << "????" << 'W'; // *b#W
//                     tmp = stream.str();
//                     buffer->append(tmp.c_str(), tmp.length());
//                     */
//                }
//            }
//            break;
//
//        case 4: // RGBA
//            for(int y=0; y<h; y++)
//            {
//                for(int x=0; x<w; x++)
//                {
//                    int r = pixels[(y*w+x)*4];
//                    int g = pixels[(y*w+x)*4 +1];
//                    int b = pixels[(y*w+x)*4 +2];
//                    int a = pixels[(y*w+x)*4 +3];
//
//                    stream.str("");
//                    stream.clear();
//                    tmp.erase();
//                    stream << "\e*p" << basey + y << 'Y'; // *p#Y
//                    tmp = stream.str();
//                    buffer->append(tmp.c_str(), tmp.length());
//
//                    stream.str("");
//                    stream.clear();
//                    tmp.erase();
//                    stream << "\e*p" << basey << 'X'; // *p#X
//                    tmp = stream.str();
//                    buffer->append(tmp.c_str(), tmp.length());
//
//                    /*
//                     *b#A    # = Number of unpacked bytes (1 byte = 8 pixels)
//                     If this is negative, pixels are unpacked right-to-left
//                     */
//                    /*
//                     stream.str("");
//                     stream.clear();
//                     tmp.erase();
//                     stream << "\e*b" << "????" << 'A'; // *b#A
//                     tmp = stream.str();
//                     buffer->append(tmp.c_str(), tmp.length());
//                     */
//
//                    /*
//                     *b#W    # = Number of encoded bytes
//                     */
//                    /*
//                     stream.str("");
//                     stream.clear();
//                     tmp.erase();
//                     stream << "\e*b" << "????" << 'W'; // *b#W
//                     tmp = stream.str();
//                     buffer->append(tmp.c_str(), tmp.length());
//                     */
//                }
//            }
//            break;
//
//        default:
//            break;
//    }
//
//    return buffer;
//}

/*
 // no longer used
 ofPtr<HPGLBuffer> HPGLBuffer::create(ofImage img, ofPoint offset, OutputConfig config)
 {
 ofLog(OF_LOG_VERBOSE, "HPGLBuffer::create(ofImage, OutputConfig) img.width=%d, img.height=%d", img.width, img.height);
 return create(img.getPixelsRef(), offset, config);
 }
 */


// no longer used
//ofPtr<GMLBuffer> GMLBuffer::create(string gmlFilePath, OutputConfig config)
//{
//    ofPtr<GMLBuffer> buffer = ofPtr<GMLBuffer>(new GMLBuffer());
//
//    if(gmlFilePath == "")
//    {
//        ofLog(OF_LOG_ERROR, "GML file path is empty.");
//        return buffer;
//    }
//
//    ofXml gml;
//    if(gml.load(gmlFilePath))
//    {
//        /*
//         GML minimum format
//         <gml spec='1.0 (minimum)'>
//         <tag>
//         <drawing>
//         <stroke>
//         <pt>
//         <x>0.0</x>
//         <y>0.0</y>
//         </pt>
//         </stroke>
//         </drawing>
//         </tag>
//         </gml>
//         */
//
//        std::ostringstream stream;
//        string tmp;
//
//        if(gml.exists("//tag/drawing/stroke"))
//        {
//            gml.setTo("//tag/drawing[0]");
//            int strokes = gml.getNumChildren();
//            if(strokes == 0)
//            {
//                ofLog(OF_LOG_WARNING, "No stroke in this GML.");
//                return buffer;
//            }
//
//            gml.setTo("stroke[0]");
//            for(int i=0; i<strokes; i++)
//            {
//                int points = gml.getNumChildren();
//                gml.setTo("pt[0]");
//                for(int j=0; j<points; j++)
//                {
//                    double x = gml.getValue<double>("x");
//                    double y = gml.getValue<double>("y");
//
//                    //
//                    // TODO: Convert coordinates to the MM.
//                    //
//                    //x *= 1000;
//                    //y *= 1000;
//
//                    stream.str("");
//                    stream.clear();
//                    tmp.erase();
//
//                    if(j==0)
//                    {
//                        stream << "PU" << floor(x) << ',' << floor(y) << ';'; // PUx,y;
//                        tmp = stream.str();
//                        buffer->append(tmp.c_str(), tmp.length());
//                    }
//
//                    stream << "PD" << floor(x) << ',' << floor(y);  // PDx,y{,x,y...};
//                    tmp = stream.str();
//                    buffer->append(tmp.c_str(), tmp.length());
//
//                    ofLog(OF_LOG_VERBOSE, "GML::stroke["+ofToString(i)+"]::pt["+ofToString(j)+"] x="+ofToString(x)+" y="+ofToString(y));
//
//                    gml.setToSibling();
//                }
//                gml.setToParent();
//                gml.setToSibling();
//            }
//        }
//        else
//        {
//            ofLog(OF_LOG_ERROR, "This file is not GML.");
//            return buffer;
//        }
//    }
//    else
//    {
//        ofLog(OF_LOG_ERROR, "Unable to load GML file.");
//        return buffer;
//    }
//
//    return buffer;
//}

