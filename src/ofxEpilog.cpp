/*
 
 ofxEpilog - OpenFrameworks library for Epilog laser cutter.
 
 
 Copyright (C) 2002-2008 Andrews & Arnold Ltd <info@aaisp.net.uk>
 Copyright (C) 2008 AS220 Labs <brandon@as220.org>
 Copyright (C) 2011 Trammell Hudson <hudson@osresearch.net>
 Copyright (C) 2013 Mitsuhito Ando <mitsuhito.ando@gmail.com>
 
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

ofPtr<HPGLBuffer> HPGLBuffer::create(ofPolyline line, OutputConfig config)
{
    ofPtr<HPGLBuffer> buffer = ofPtr<HPGLBuffer>(new HPGLBuffer());
    
    ofLog(OF_LOG_VERBOSE, "HPGLBuffer::create(ofPolyline, OutputConfig) line.size=%d", line.size());
    
    if(line.size() == 0)
        return buffer;
    
    std::ostringstream stream;
    string tmp = "";
    double x,y = 0;
    
    vector<ofPoint> vertices = line.getVertices();
    x = ((double)config.dpi/(double)MM_PER_INCH) * vertices[0].x;
    y = ((double)config.dpi/(double)MM_PER_INCH) * vertices[0].y;
    stream << "PU" << floor(x) << ',' << floor(y) << ';'; // PUx,y;
    tmp = stream.str();
    buffer->append(tmp.c_str(), tmp.length());
    
    for(int i=0; i<vertices.size(); i++)
    {
        stream.str("");
        stream.clear();
        tmp.erase();
        x = ((double)config.dpi/(double)MM_PER_INCH) * vertices[i].x;
        y = ((double)config.dpi/(double)MM_PER_INCH) * vertices[i].y;
        //cout << "x=" << x << " ,y=" << y << "a" << endl;
        stream << "PD" << floor(x) << ',' << floor(y); // PDx,y{,x,y...};
        tmp = stream.str();
        buffer->append(tmp.c_str(), tmp.length());
    }
    buffer->append(";", 1);
    
    stream.str("");
    stream.clear();
    tmp.erase();
    
    x = ((double)config.dpi/(double)MM_PER_INCH) * vertices[vertices.size()-1].x;
    y = ((double)config.dpi/(double)MM_PER_INCH) * vertices[vertices.size()-1].y;
    stream << "PU" << floor(x) << ',' << floor(y) << ';'; // PUx,y;
    tmp = stream.str();
    buffer->append(tmp.c_str(), tmp.length());    
    
    return buffer;
}

ofPtr<HPGLBuffer> HPGLBuffer::create(ofPath path, OutputConfig config)
{
    //
    // not implemented yet
    //
}

ofPtr<HPGLBuffer> HPGLBuffer::create(ofImage img, OutputConfig config)
{
    ofPtr<HPGLBuffer> buffer = ofPtr<HPGLBuffer>(new HPGLBuffer());
    
    ofLog(OF_LOG_VERBOSE, "HPGLBuffer::create(ofImage, OutputConfig) img.width=%d, img.height=%d", img.width, img.height);
    
    int w = img.width;
    int h = img.height;
    if(w == 0 && h == 0)
        return buffer;
    //
    // TODO: finish implementation
    //
    /*
     int basex = 0;
     int basey = 0;
     
     std::ostringstream stream;
     string tmp = "";
     
     unsigned char *pixels = img.getPixels();
     for(int i=0; i<w; i+=8)
     {
     for(int j=0; j<h; j+=8)
     {
     int r = pixels[j*3*w + i*3];
     int g = pixels[j*3*w + i*3+1];
     int b = pixels[j*3*w + i*3+2];
     }
     }
    */
    
    return buffer;
}

ofPtr<GMLBuffer> GMLBuffer::create(string gmlFilePath, OutputConfig config)
{
    ofPtr<GMLBuffer> buffer = ofPtr<GMLBuffer>(new GMLBuffer());
    
    if(gmlFilePath == "")
    {
        ofLog(OF_LOG_ERROR, "GML file path is empty.");
        return buffer;
    }
    
    ofXml gml;
    if(gml.load(gmlFilePath))
    {
        /*
         GML minimum format
         <gml spec='1.0 (minimum)'>
         <tag>
         <drawing>
         <stroke>
         <pt>
         <x>0.0</x>
         <y>0.0</y>
         </pt>
         </stroke>
         </drawing>
         </tag>
         </gml>
        */
        
        std::ostringstream stream;
        string tmp;
        
        if(gml.exists("//tag/drawing/stroke"))
        {
            gml.setTo("//tag/drawing[0]");
            int strokes = gml.getNumChildren();
            if(strokes == 0)
            {
                ofLog(OF_LOG_WARNING, "No stroke in this GML.");
                return buffer;
            }
            
            gml.setTo("stroke[0]");
            for(int i=0; i<strokes; i++)
            {
                int points = gml.getNumChildren();
                gml.setTo("pt[0]");
                for(int j=0; j<points; j++)
                {
                    double x = gml.getValue<double>("x");
                    double y = gml.getValue<double>("y");
                    
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
                        buffer->append(tmp.c_str(), tmp.length());
                    }
                    
                    stream << "PD" << floor(x) << ',' << floor(y);  // PDx,y{,x,y...};
                    tmp = stream.str();
                    buffer->append(tmp.c_str(), tmp.length());
                    
                    //ofLog(OF_LOG_VERBOSE, "GML::stroke["+ofToString(i)+"]::pt["+ofToString(j)+"] x="+ofToString(x)+" y="+ofToString(y));
                    
                    gml.setToSibling();
                }
                gml.setToParent();
                gml.setToSibling();
            }
        }
        else
        {
            ofLog(OF_LOG_ERROR, "This file is not GML.");
            return buffer;
        }
    }
    else
    {
        ofLog(OF_LOG_ERROR, "Unable to load GML file.");
        return buffer;
    }
    
    return buffer;
}

ofxEpilog::ofxEpilog()
{
    ipAddr = "000.000.000.000"; // dummy
    hostname = "UNKOWN HOSTNAME";
    
    char localhost[128] = "";
    gethostname(localhost, sizeof(localhost));
    hostname = string(localhost);
    
    //modelType = UNKNOWN;
    //isLiveMode = false;
    
    cout << "ofxEpilog::ofxEpilog() hostname=" << hostname << endl;
}

ofxEpilog::ofxEpilog(const ofxEpilog &obj)
{
    ofLog(OF_LOG_VERBOSE, "ofxEpilog::ofxEpilog(const ofxEpilog &obj)");
}

ofxEpilog::~ofxEpilog()
{
    if(tcpClient.isConnected())
        disconnect();
}

void ofxEpilog::setWorkareaSize(WorkareaSize size)
{
    workareaSize = size;
    
    ofLog(OF_LOG_VERBOSE, "ofxEpilog::setWorkareaSize(size) width=%d, height=%d z=%d", size.width, size.height, size.z);
}

void ofxEpilog::setOutputConfig(OutputConfig config)
{
    ofLog(OF_LOG_WARNING, "ofxEpilog.modelType is UNKNOWN. Specify what model you using. (e.g. FUSION, MINI, HELIX ....");
    
    if(config.rpower > 100)
        config.rpower = 100;
    
    if(config.rspeed > 100)
        config.rspeed = 100;
    
    if(config.vpower > 100)
        config.vpower = 100;
    
    if(config.vspeed > 100)
        config.vspeed = 100;
    
    //
    // TODO: Is this percentage or hz? ask technical support.
    //
    if(config.freq > 100)
        config.freq = 100;
    if(config.freq < 10)
        config.freq = 10;
    
    if(config.dpi > 1200)
        config.dpi = 1200;
    if(config.dpi < 75)
        config.dpi = 75;
    
    if(tcpClient.isConnected())
    {
        // update vector outputconfig
        //if(hpglmode)
        //{
        //  updateVectorOutputConfig();
        //}
    }
    else
    {
        outputConfig = config;
    }
    
    ofLog(OF_LOG_VERBOSE, "ofxEpilog::setOutputConfig(config) vspeed=%d, vpower=%d, rspeed=%d, rpower=%d, dpi=%d, freq=%d, rasterMode=%d", config.vspeed, config.vpower, config.rspeed, config.rpower, config.dpi, config.freq, config.rasterMode);
}

OutputConfig ofxEpilog::getOutputConfig()
{
    return outputConfig;
}

bool ofxEpilog::connect(string ip, bool liveMode)
{
    if(ip == "")
        return false;
    
    ofLog(OF_LOG_VERBOSE, "ofxEpilog::connect(): start cnnecting to the laser cutter.");
    //isLiveMode = liveMode;
    
    if(tcpClient.isConnected())
    {
        ofLog(OF_LOG_ERROR,"ofxEpilog: the connection is created already.You should disconnect first." );
        return false;
    }
    
    ipAddr = ip;
    bool isConnected = tcpClient.setup(ipAddr, PRINTER_SERVICE_PORT, true);
    
    if(!isConnected)
        return false;
    
    ofLog(OF_LOG_ERROR,"ofxEpilog:connect(): start sending LPR handshake packets." );
    
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
    tcpClient.sendRaw("\002\n");
    
    //string response = tcpClient.receiveRaw();
    
    char response;
    tcpClient.receiveRawBytes(&response, 1);
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
        tcpClient.sendRaw(stream.str());
        
        tcpClient.receiveRawBytes(&response, 1);
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
        tcpClient.sendRaw(stream.str());
        
        tcpClient.receiveRawBytes(&response, 1);
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
        tcpClient.sendRaw(stream.str());
    }
    else
        return false;
    
    ofLog(OF_LOG_VERBOSE, "ofxEpilog::connect(): LPR handshake complete.");
    
    return sendPJLHeader();
}

bool ofxEpilog::send(const ofPtr<HPGLBuffer> &buffer, JOB_TYPE type)
{    
    ofLog(OF_LOG_VERBOSE, "ofxEpilog::send() const ofPtr<HPGLBuffer> &buffer.useCount=" + ofToString(buffer.use_count()) );
    
    if(!tcpClient.isConnected())
    {
        ofLog(OF_LOG_VERBOSE, "ofxEpilog is not connected.");
        return false;
    }
    
    if(buffer->size() == 0)
    {
        ofLog(OF_LOG_VERBOSE, "HPGLBuffer is empty.");
        return false;
    }
    
    bool isSent = true;
    if(type == VECTOR)
    {
        if(1/*vector job is ready*/)
        {
            isSent &= sendPCLVectorHeader();
            if(!isSent) false;
            isSent &= updateVectorOutputConfig();
            if(!isSent) false;
            isSent &= sendPCLVectorJob(buffer);
            if(!isSent) false;
            isSent &= sendPCLVectorFooter();
            if(!isSent) false;
        }
    }
    else if(type == RASTER)
    {
        if(1/*raster job is ready*/)
        {
            isSent &= sendPCLRasterHeader();
            if(!isSent) false;
            isSent &= sendPCLRasterJob(buffer);
            if(!isSent) false;
            isSent &= sendPCLRasterFooter();
            if(!isSent) false;
        }
    }
    return true;
}

bool ofxEpilog::isConnected()
{
    return tcpClient.isConnected();
}

void ofxEpilog::disconnect()
{
    if(tcpClient.isConnected())
    {
        sendPJLFooter();
        tcpClient.close();
    }
}

bool ofxEpilog::sendPJLHeader()
{
    ofLog(OF_LOG_VERBOSE, "ofxEpilog::sendPJLHeader(): start sending PJL header.");
    
    std::ostringstream stream;
    stream << "\e\%-12345X@PJL JOB NAME=ofxEpilog\r\n";
    
    // Start sending JPL header
    return tcpClient.sendRaw(stream.str());
}

bool ofxEpilog::sendPJLFooter()
{
    ofLog(OF_LOG_VERBOSE, "ofxEpilog::sendPJLFooter(): start sending PJL footer.");
    
    std::ostringstream stream;
    stream << "\eE"; // Reset
    stream << "\e\%-12345X@PJL EOJ \r\n"; // Job complete
    
    // Pad out the remainder of the file with 0 characters
    for(int i = 0; i < 4096; i++) {
        stream << 0x0;
    }
    
    // Start sending JPL footer
    return tcpClient.sendRaw(stream.str());
}

bool ofxEpilog::sendPCLRasterHeader()
{
    ofLog(OF_LOG_VERBOSE, "ofxEpilog::sendPCLRasterHeader(): start sending PCL raster header.");
    
    //
    // Start sending PCL raster header
    //
    std::ostringstream stream;
    bool isSent = true;
    
    stream << "\eE@PJL ENTER LANGUAGE=PCL\r\n"; // PCL ENTER command
    isSent &= tcpClient.sendRaw(stream.str());
    
    stream.str("");
    stream.clear();
    stream << "\e&y" << outputConfig.autoFocusEnabled <<"A"; // Auto focus
    isSent &= tcpClient.sendRaw(stream.str());
    
    stream.str("");
    stream.clear();
    stream << "\e&l0U"; // Left (long-edge) offset registration
    isSent &= tcpClient.sendRaw(stream.str());
    
    stream.str("");
    stream.clear();
    stream << "\e&l0Z"; // Top (short-edge) offset registration
    isSent &= tcpClient.sendRaw(stream.str());
    
    stream.str("");
    stream.clear();
    stream << "\e&u" << outputConfig.dpi << "D"; // Resolution
    isSent &= tcpClient.sendRaw(stream.str());
    
    stream.str("");
    stream.clear();
    stream << "\e*p0X"; // X position = 0
    isSent &= tcpClient.sendRaw(stream.str());
    
    stream.str("");
    stream.clear();
    stream << "\e*p0Y"; // Y position = 0
    isSent &= tcpClient.sendRaw(stream.str());
    
    stream.str("");
    stream.clear();
    stream << "\e*t" << outputConfig.dpi << "R"; // Resolution
    isSent &= tcpClient.sendRaw(stream.str());
    
    stream.str("");
    stream.clear();
    stream << "\e&y0C"; // Global air assist (0 = global, 1 = local)
    isSent &= tcpClient.sendRaw(stream.str());
    
    stream.str("");
    stream.clear();
    stream << "\e*r0F"; // Flush pages
    isSent &= tcpClient.sendRaw(stream.str());
    
    stream.str("");
    stream.clear();
    stream << "\e&y" << outputConfig.rpower << "P"; // Raster power
    isSent &= tcpClient.sendRaw(stream.str());
    
    stream.str("");
    stream.clear();
    stream << "\e&z" << outputConfig.rspeed << "S"; // Raster speed
    isSent &= tcpClient.sendRaw(stream.str());
    
    stream.str("");
    stream.clear();
    stream << "\e*r" <<  floor((double)(outputConfig.dpi/(double)MM_PER_INCH) * workareaSize.height) << "T"; // 
    isSent &= tcpClient.sendRaw(stream.str());
    
    stream.str("");
    stream.clear();
    stream << "\e*r" << floor((double)(outputConfig.dpi/(double)MM_PER_INCH) * workareaSize.width) << "S"; // 
    isSent &= tcpClient.sendRaw(stream.str());
    
    stream.str("");
    stream.clear();
    stream << "\e*b" << outputConfig.rasterMode << "M"; // Raster compression
    isSent &= tcpClient.sendRaw(stream.str());
    
    stream.str("");
    stream.clear();
    stream << "\e&y0O"; // Raster direction (0 = down, 1 = up)
    isSent &= tcpClient.sendRaw(stream.str());
    
    stream.str("");
    stream.clear();
    stream << "\e&z" << 2 << "A"; // Air assist (0 = raster air assist off, 1 = raster air assist on, 2 = global air assist on)
    isSent &= tcpClient.sendRaw(stream.str());
    
    return isSent;
}

bool ofxEpilog::sendPCLRasterJob(const ofPtr<HPGLBuffer> &buffer)
{
    ofLog(OF_LOG_VERBOSE, "ofxEpilog::sendPCLRasterJob(): start sending PCL raster job, buffer.size=&d.", buffer->size());
    
    // Start sending PCL raster job
    return tcpClient.sendRawBytes(buffer->getBinaryBuffer(), buffer->size());
}

bool ofxEpilog::sendPCLRasterFooter()
{
    ofLog(OF_LOG_VERBOSE, "ofxEpilog::sendPCLRasterFooter(): start sending PCL raster footer.");
    
    std::ostringstream stream;
    stream << "\e*rC"; // End graphics with reset
    stream << 0x1a << 0x4; // End of file markers
    
    // Start sending JPL footer
    return tcpClient.sendRaw(stream.str());
}

bool ofxEpilog::sendPCLVectorHeader()
{
    ofLog(OF_LOG_VERBOSE, "ofxEpilog::sendPCLVectorHeader(): start sending PCL vector header.");
    
    //
    // Start sending PCL vector header
    //
    std::ostringstream stream;
    bool isSent = true;
    
    stream << "\eE@PJL ENTER LANGUAGE=PCL\r\n";
    isSent &= tcpClient.sendRaw(stream.str());
    
    stream.str("");
    stream.clear();
    stream << "\e*r0F";
    isSent &= tcpClient.sendRaw(stream.str());
    
    stream.str("");
    stream.clear();
    stream << "\e&y" << outputConfig.autoFocusEnabled <<"A"; // Auto focus
    isSent &= tcpClient.sendRaw(stream.str());
    
    stream.str("");
    stream.clear();
    stream << "\e&l0U"; // Left (long-edge) offset registration
    isSent &= tcpClient.sendRaw(stream.str());
    
    stream.str("");
    stream.clear();
    stream << "\e&l0Z"; // Top (short-edge) offset registration
    isSent &= tcpClient.sendRaw(stream.str());
    
    stream.str("");
    stream.clear();
    stream << "\e&u" << outputConfig.dpi << "D"; // Resolution
    isSent &= tcpClient.sendRaw(stream.str());
    
    stream.str("");
    stream.clear();
    stream << "\e*p0X"; // X position = 0
    isSent &= tcpClient.sendRaw(stream.str());
    
    stream.str("");
    stream.clear();
    stream << "\e*p0Y"; // Y position = 0
    isSent &= tcpClient.sendRaw(stream.str());
    
    stream.str("");
    stream.clear();
    stream << "\e*t" << outputConfig.dpi << "R"; // Resolution
    isSent &= tcpClient.sendRaw(stream.str());
    
    stream.str("");
    stream.clear();
    stream << "\e*r" << floor((double)(outputConfig.dpi/(double)MM_PER_INCH) * workareaSize.height) << "T"; //
    isSent &= tcpClient.sendRaw(stream.str());
    
    stream.str("");
    stream.clear();
    stream << "\e*r" << floor((double)(outputConfig.dpi/(double)MM_PER_INCH) * workareaSize.width) << "S"; // 
    isSent &= tcpClient.sendRaw(stream.str());
    
    stream.str("");
    stream.clear();
    stream << "\e*r1A"; // Start at current position
    isSent &= tcpClient.sendRaw(stream.str());
    
    stream.str("");
    stream.clear();
    stream << "\e*rC"; // End graphics with reset
    isSent &= tcpClient.sendRaw(stream.str());
    
    stream.str("");
    stream.clear();
    stream << "\e\%1B"; // 
    isSent &= tcpClient.sendRaw(stream.str());
    
    stream.str("");
    stream.clear();
    stream << "IN;"; // We are now in HPGL mode
    isSent &= tcpClient.sendRaw(stream.str());
    
    return isSent;
}

bool ofxEpilog::sendPCLVectorJob(const ofPtr<HPGLBuffer> &buffer)
{
    ofLog(OF_LOG_VERBOSE, "ofxEpilog::sendPCLVectorJob(): start sending PCL vector job, buffer.size=%d.", buffer->size());
    ofLog(OF_LOG_VERBOSE, "\tbuffer=" + buffer->getText());
    
    // Start sending PCL vector job
    return tcpClient.sendRawBytes(buffer->getBinaryBuffer(), buffer->size());
}

bool ofxEpilog::sendPCLVectorFooter()
{
    ofLog(OF_LOG_VERBOSE, "ofxEpilog::sendPCLVectorFooter(): start sending PCL vector footer.");
    
    //
    // Start sending PCL vector footer
    //
    std::ostringstream stream;
    bool isSent = true;
    
    stream << "PU;"; // Stop the laser
    isSent &= tcpClient.sendRaw(stream.str());
    
    stream.str("");
    stream.clear();
    stream << "\e\%0B"; // End HLGL
    isSent &= tcpClient.sendRaw(stream.str());
    
    stream.str("");
    stream.clear();
    stream << "\e\%1BPU"; // Start HLGL, and pen up, end
    isSent &= tcpClient.sendRaw(stream.str());
    
    return isSent;
}

bool ofxEpilog::updateVectorOutputConfig()
{
    if(!tcpClient.isConnected())
        return false;
    
    ofLog(OF_LOG_VERBOSE, "ofxEpilog::updateVectorOutputConfig(): start sending vector output configuration.");
    
    //
    // Start updating vector output config
    //
    std::ostringstream stream;
    bool isSent = true;
    
    stream << "XR" << setfill('0') << setw(4) << right << outputConfig.freq << ';'; // Vector freq
    stream << "YP" << setfill('0') << setw(3) << right << outputConfig.vpower << ';'; // Vector power
    stream << "ZS" << setfill('0') << setw(3) << right << outputConfig.vspeed << ';'; // Vector speed
    
    ofLog(OF_LOG_VERBOSE, stream.str());
    
    isSent &= tcpClient.sendRaw(stream.str());
    
    return isSent;
}

/*
 // Not supporting thread feature yet
 void ofxEpilog::threadedFunction()
 {
 while(isThreadRunning())
 {
 if(tcpClient.isConnected())
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
