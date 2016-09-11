#include "ofApp.h"



/*
 //
 // this example need following arduino code.
 //
 int a0_raw = 0;
 int a1_raw = 0;
 
 void setup()
 {
    Serial.begin(9600);
 }
 
 void loop()
 {
    a0_raw = analogRead(A0);
    a1_raw = analogRead(A1);
    byte a0 = map(a0_raw, 0, 1023, 0, 255);
    byte a1 = map(a1_raw, 0, 1023, 0, 255);
    Serial.write(a0);
    Serial.write(a1);
    delay(50);
 }
*/


//--------------------------------------------------------------
void ofApp::setup(){
    ofSetFrameRate(25);
    
    // vector power 0~100%
    out_conf.vec_power = 50;
    // vector speed 1~100%
    out_conf.vec_speed = 100;
    // frequency of vector process ~100%
    out_conf.vec_freq = 100;
    
    // raster power 0~100%
    out_conf.ras_power = 10;
    // raster speed 1~100%
    out_conf.ras_speed = 50;
    // frequency of raster process ~100%
    out_conf.ras_freq = 50;
    
    // auto focus switch
    out_conf.use_auto_fucus = false;
    
    // material thickness in mm (only fusion)
    out_conf.z_thickness = 0;
    
    // setting table height in mm (only fusion)
    out_conf.z_offset = 0;
    
    // output dpi (75, 150, 300 600, 1200), don't use other values
    out_conf.dpi = 300;//75;
    
    // emit or not laser
    out_conf.enable_laser_emit = true;
    
    // init epilog laser object with PJL file output (fusion)
    //epilogLaser.setup(ofxEpilog::FUSION_32, out_conf, true);
    
    // init epilog laser object with PJL file output (mini)
    epilogLaser.setup(ofxEpilog::MINI_18, out_conf, true);
    
    // easy to see the location of cutting path and it's shape
    ofSetWindowShape(ofxEpilog::MINI_18.width, ofxEpilog::MINI_18.height);
    
    serial.listDevices();
    serial.setup(0, 9600);
    serial.flush();
}

//--------------------------------------------------------------
void ofApp::exit(){
    if(epilogLaser.isConnected())
        epilogLaser.disconnect();
    
    serial.close();
}


//--------------------------------------------------------------
void ofApp::update(){
    if(epilogLaser.isConnected())
        ofSetWindowTitle("connection established");
    else
        ofSetWindowTitle("disconnected");
    
    if(serial.available())
    {
        int vector_power = serial.readByte();
        this->out_conf.vec_power = ofMap(vector_power, 0, 255, 0, 100);
    }
    if(serial.available())
    {
        int vector_speed = serial.readByte();
        this->out_conf.vec_speed = ofMap(vector_speed, 0, 255, 0, 100);
    }
    epilogLaser.setOutputConfig(out_conf);
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofSetBackgroundColor(255);
    
    ofRectangle working_area(0, 0, 150, 150);
    
    float noise_x = ofNoise(float(ofGetFrameNum())/100.f, float(ofGetFrameNum())/300.0f);
    float noise_y = ofNoise(float(ofGetFrameNum())/100.f, float(ofGetFrameNum())/400.0f);

    ofBeginShape();
    ofSetColor(0);
    ofVertices(trail);
    ofNoFill();
    ofEndShape(false);
    
    ofPushMatrix();
    float _x = ofMap( noise_x, 0, 1, working_area.x, working_area.x + working_area.width, true);
    float _y = ofMap( noise_y, 0, 1, working_area.y, working_area.y + working_area.height, true);
    
    ofTranslate(_x, _y, 0);
    ofNoFill();
    ofSetColor(245, 58, 135);
    ofDrawCircle(0, 0, 5);
    ofPopMatrix();

    if( trail.size() == 0 || ( trail.back() - ofPoint(_x, _y) ).length() > dist_threshold )
    {
        trail.push_back(ofPoint(_x,_y));
        
        if(trail.size() > 10)
        {
            trail.erase(trail.begin(), trail.begin()+1);
        }
        
        if(trail.size() > 2)
        {
            ofPolyline line;
            line.addVertex(trail[trail.size()-2]);
            line.addVertex(trail[trail.size()-1]);
            if(epilogLaser.isConnected() && send_vertexes)
                epilogLaser.send(line);
        }
    }
    
    ofSetColor(0);
    ofDrawBitmapString("vector speed: " + ofToString(out_conf.vec_speed) + " vector power: " + ofToString(out_conf.vec_power), 10, ofGetWindowHeight() -10 );
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if(key == 'c')
    {
        // connect to epilog laser (finish the job)
        //epilogLaser.connect("192.168.3.4", false);
        
        // connect to epilog laser (keep the process)
        epilogLaser.connect("192.168.3.4", true);
        
    }
    else if (key == 's')
    {
        if(epilogLaser.isConnected())
        {
            send_vertexes = !send_vertexes;
        }
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
