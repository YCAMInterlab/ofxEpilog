#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
    // if you want to use laser cutter as interactive, don't send commands too fast each time.
    ofSetFrameRate(30);
    
    ofSetLogLevel(OF_LOG_VERBOSE);
    ofEnableAlphaBlending();

    font.load("NotoSans-Regular.ttf", 8, true, true);
    
    OutputConfig out_conf;
    out_conf.vec_power = 10;
    out_conf.vec_speed = 100;
    out_conf.vec_freq = 50;
    out_conf.ras_power = 10;
    out_conf.ras_speed = 50;
    out_conf.ras_freq = 50;
    //out_conf.use_auto_fucus = true;
    out_conf.use_auto_fucus = false;
    out_conf.z_thickness = 0;
    out_conf.z_offset = 0;
    out_conf.dpi = 75; // 75, 150, 300 600, 1200
    
    
    // init epilog laser object
    //epilogLaser.setup(ofxEpilog::FUSION_32, out_conf, true);

    // init epilog laser object
    epilogLaser.setup(ofxEpilog::MINI_18, out_conf, true);
    
    // create datgui object
    gui = new ofxDatGui( ofxDatGuiAnchor::TOP_RIGHT );
    
    ofxDatGuiScrollView* myScrollView = new ofxDatGuiScrollView("hoge", 2);
    
    // fps display
    //gui->addFRM();
    
    // fullscreen toggle
    //gui->addToggle("toggle fullscreen", true);
    
    // vsync toggle
    //gui->addToggle("vsync", vsync);
    
    gui->addBreak();
    
    // machine profile
    ofxDatGuiFolder* machine_pforile_folder = gui->addFolder("machine profile", ofColor::white);
    machine_pforile_folder->addTextInput("ip address", "192.168.3.4");
    machine_pforile_folder->addTextInput("table width (mm)", ofToString(epilogLaser.getMachineProfile().width));
    machine_pforile_folder->addTextInput("table height (mm)", ofToString(epilogLaser.getMachineProfile().height));
    machine_pforile_folder->addTextInput("z top (mm)", ofToString(epilogLaser.getMachineProfile().z_top));
    machine_pforile_folder->addTextInput("z buttom (mm)", ofToString(epilogLaser.getMachineProfile().z_buttom));
    machine_pforile_folder->addTextInput("max dpi", ofToString(epilogLaser.getMachineProfile().max_dpi));
    machine_pforile_folder->addTextInput("min dpi", ofToString(epilogLaser.getMachineProfile().min_dpi));
    machine_pforile_folder->addTextInput("max freq", ofToString(epilogLaser.getMachineProfile().max_freq));
    machine_pforile_folder->addTextInput("min freq", ofToString(epilogLaser.getMachineProfile().min_freq));
    machine_pforile_folder->addTextInput("lens focus length (mm)", ofToString(epilogLaser.getMachineProfile().focus_dist));
    machine_pforile_folder->addTextInput("max thickness (mm)", ofToString(epilogLaser.getMachineProfile().max_thickness));
    machine_pforile_folder->addTextInput("min thickness (mm)", ofToString(epilogLaser.getMachineProfile().min_thickness));
    machine_pforile_folder->expand();
    
    gui->addBreak();
    gui->addToggle("connect", false)->setFocused(true);
//    gui->addTextInput("connection status", "not connected");
    gui->addBreak();
    
    // output config
    ofxDatGuiFolder* output_config_folder = gui->addFolder("output config", ofColor::red);
    output_config_folder->addSlider("vector pwer (%)", 0, 100, 50)->bind(epilogLaser.getOutputConfig().vec_power);
    output_config_folder->addSlider("vector speed (%)", 0, 100, 50)->bind(epilogLaser.getOutputConfig().vec_speed);
    output_config_folder->addSlider("raster pwer (%)", 0, 100, 50)->bind(epilogLaser.getOutputConfig().ras_power);
    output_config_folder->addSlider("raster speed (%)", 0, 100, 50)->bind(epilogLaser.getOutputConfig().ras_speed);

    output_config_folder->addSlider("dpi", 0, 1200, 0)->bind(epilogLaser.getOutputConfig().dpi);
    output_config_folder->addSlider("frequency (%)", 0, 100, 50)->bind(epilogLaser.getOutputConfig().vec_freq);

    output_config_folder->addTextInput("z offset (mm)", ofToString(epilogLaser.getOutputConfig().z_offset) );
    output_config_folder->addTextInput("material thickness (mm)", ofToString(epilogLaser.getOutputConfig().z_thickness));
    output_config_folder->addToggle("use autofacus", epilogLaser.getOutputConfig().use_auto_fucus); // boolean bind is not working?
    output_config_folder->addToggle("enable laser emit", epilogLaser.getOutputConfig().enable_laser_emit); // boolean bind is not working?
    
    output_config_folder->expand();
    
    gui->addBreak();
    
    ofxDatGui2dPad* pad = gui->add2dPad("xy");
    pad->setBounds(ofRectangle(0,0,epilogLaser.getMachineProfile().width, epilogLaser.getMachineProfile().height));
    
    ofLog() << gui->get2dPad("xy")->getPoint();

    ofxDatGuiValuePlotter *z_plotter = gui->addValuePlotter("z axis", 0, 100);
    z_plotter->setRange(-1*epilogLaser.getMachineProfile().z_buttom, fabs(epilogLaser.getMachineProfile().z_top));
    z_plotter->setValue(0);
    z_plotter->setSpeed(1);
    
    gui->addButton("load dxf file");
    gui->addButton("load 3d model");
    
    gui->addBreak();
    gui->addToggle("simulate tool path");
    gui->addToggle("show tool path", is_visible_dxf);
    gui->addToggle("show 3d model", is_visible_3d_model);
    gui->addSlider("step", 0, 0, 0);
    gui->addToggle("send dxf tool path");
    
    // adding the optional header allows you to drag the gui around //
    gui->addHeader("machine profiles");
    
    // adding the optional footer allows you to collapse/expand the gui //
    gui->addFooter();
    
    // gui event handlers
    gui->onButtonEvent(this, &ofApp::gui_button_handler);
    gui->on2dPadEvent(this, &ofApp::gui_2dpad_handler);
    gui->onSliderEvent(this, &ofApp::gui_slider_hander);
    
    gui->setTheme(new ofxDatGuiThemeSmoke());
    gui->setWidth(850);
    gui->setPosition(20, 20);
    gui->update();
    
    ofSetWindowShape(3200, gui->getHeight() + 40);
    
    epilog_table.set(epilogLaser.getMachineProfile().width, epilogLaser.getMachineProfile().height, 5);
    epilog_table.setScale(1);
    
    easy_cam.setDistance(600);
    easy_cam.setPosition(ofVec3f(-80, 50, 600));
    easy_cam.setNearClip(0);
    
    live_mode = true;
}

//--------------------------------------------------------------
void ofApp::update()
{
    ofSetWindowTitle("ofxEpilog::"+ofToString(ofGetFrameRate()));

    if(is_simulating)
    {
        toolpath_render.update();
        gui->getValuePlotter("z axis")->setValue(-1*toolpath_render.get_current_pt().z);
        
        gui->getSlider("step")->setValue(toolpath_render.get_progress());
    }

    gui->get2dPad("xy")->update();
}

//--------------------------------------------------------------
void ofApp::draw()
{
    ofBackground(90);

    model_origin = ofPoint(-1*(epilogLaser.getMachineProfile().width/2),
                     epilogLaser.getMachineProfile().height/2,
                     0);

    
    ofRectangle rect = ofRectangle(gui->getPosition().x, gui->getPosition().y, gui->getWidth(), gui->getHeight());
    if(rect.inside(ofPoint(ofGetMouseX(), ofGetMouseY())))
    {
        easy_cam.disableMouseInput();
    }
    else
    {
        easy_cam.enableMouseInput();
    }
        
    
    easy_cam.begin();
    ofRotateX(-40);
    ofPushMatrix();
    
    
    // draw bot buttom surface
    ofPushMatrix();
    ofPushStyle();
    ofSetColor(255,255,255,10);
    //ofNoFill();
    ofTranslate(-1*(epilogLaser.getMachineProfile().width/2),
                -1*(epilogLaser.getMachineProfile().height/2),
                -1*(fabs(epilogLaser.getMachineProfile().z_buttom) + fabs(epilogLaser.getMachineProfile().max_thickness)));
    ofDrawRectangle(0, 0, epilogLaser.getMachineProfile().width, epilogLaser.getMachineProfile().height);
    
    ofSetColor(ofColor::white);
    ofNoFill();
    ofDrawRectangle(0, 0, epilogLaser.getMachineProfile().width, epilogLaser.getMachineProfile().height);
    ofPopStyle();
    ofPopMatrix();
    
    
    // draw ground surface
    
    ofPushStyle();
    ofSetColor(52, 59, 65, 205);
    ofTranslate(-1*(epilogLaser.getMachineProfile().width/2),
                (epilogLaser.getMachineProfile().height/2),
                -1*toolpath_render.get_current_pt().z);
    ofDrawRectangle(0, 0, epilogLaser.getMachineProfile().width, -1*epilogLaser.getMachineProfile().height);
    ofPopStyle();
    
    // draw traced line
    ofPushStyle();
    ofSetLineWidth(3);
    ofSetColor(ofColor(255, 30, 10));
    
    ofDrawCircle(0, 0, 0.5);
    
    if(is_simulating)
    {
        ofSetColor(255);
        font.drawString("Z : " + ofToString(-1.0*toolpath_render.get_current_pt().z) + " mm", -50, 10);
    }
    
    ofPushMatrix();
    ofSetColor(ofColor::red);
    ofRotate(180, 1, 0, 0);
    polyline_mouse_cursor.draw();
    
    if(polyline_mouse_cursor.getVertices().size() > 0)
    {
        ofSetLineWidth(5);
        ofSetColor(ofColor(255, 0, 0));
        ofVec3f head = polyline_mouse_cursor.getVertices()[polyline_mouse_cursor.getVertices().size()-1];
        ofDrawLine(head.x, head.y, head.z,
                   head.x, head.y, 1*(head.z - 400));
    }
    ofPopMatrix();
    ofPopStyle();
    
    
    if(model.hasMeshes() && is_visible_3d_model)
    {
        // draw table model_origin axis
        ofPushMatrix();
        ofPushStyle();

        ofVec3f center = model.getSceneCenter();
        //center /= model.getNormalizedScale();
        //        ofTranslate( center.x/2, center.y/2, 0 );
        //ofTranslate( model_origin.x + 80, model_origin.y - 15, 0 );
        
        model.drawWireframe();
        //  model.drawVertices();
        
        ofPopStyle();
        ofPopMatrix();
        
        //model.setPosition(-1*(model.getMesh(0).getCentroid().x/2),
        //                  -1*(model.getMesh(0).getCentroid().y/2) , model_origin.z);
    }
    
    if(is_visible_dxf)
    {
        ofPushStyle();
        ofSetLineWidth(5);
        ofSetColor(ofColor(0, 0, 255));
        
        toolpath_render.draw();
        ofSetLineWidth(5);
        ofSetColor(ofColor(255, 0, 0));
        ofVec3f head = toolpath_render.get_current_pt();
        
        if(is_simulating)
        ofDrawLine(head.x, head.y, head.z,
                   head.x, head.y, (head.z + 400));
        ofPopStyle();
    }
    
    
    
    ofPopMatrix();
    
    // draw bot top surface
    ofPushMatrix();
    ofPushStyle();
    ofSetColor(ofColor::white);
    ofNoFill();
    ofTranslate(-1*(epilogLaser.getMachineProfile().width/2),
                -1*(epilogLaser.getMachineProfile().height/2),
                fabs(epilogLaser.getMachineProfile().z_top));
    ofDrawRectangle(0, 0, epilogLaser.getMachineProfile().width, epilogLaser.getMachineProfile().height);
    ofPopStyle();
    ofPopMatrix();
    
    // draw table model_origin axis
    ofPushMatrix();
    ofPushStyle();
    
    ofTranslate(model_origin);
    ofSetColor(255, 0, 0);
    ofDrawLine(0, 0, 0, 0, -50, 0);
    ofSetColor(0, 0, 255);
    ofDrawLine(0, 0, 0, 0, 0, 50);
    ofSetColor(0, 255, 0);
    ofDrawLine(0, 0, 0, 50, 0, 0);
    ofPopStyle();
    ofPopMatrix();
    
    
    easy_cam.end();
}

void ofApp::exit()
{
    ofLog(OF_LOG_VERBOSE) << "app exiting..";
    delete gui;
}

void ofApp::gui_button_handler(ofxDatGuiButtonEvent event)
{
    ofLog(OF_LOG_VERBOSE) << event.target->getName();
    
    if(event.target->is("toggle fullscreen"))
    {
        ofToggleFullscreen();
    }
    
    if(event.target->is("vsync"))
    {
        vsync = !vsync;
        ofSetVerticalSync(vsync);
    }
    
    if(event.target->is("load 3d model"))
    {
        ofFileDialogResult result = ofSystemLoadDialog("Select a 3d model file");
        
        if (result.bSuccess)
        {
            ofLog(OF_LOG_VERBOSE) << result.getPath();
            
            model.clear();
            
            model.loadModel(result.getPath());
            model.setScale(1/model.getNormalizedScale(), 1/model.getNormalizedScale(), 1/model.getNormalizedScale());
            ofVec3f w = easy_cam.screenToWorld(ofVec3f(model_origin.x, model_origin.y, model_origin.z));
            model.setRotation(0, -180, 0, 0, 1);
            model.getSceneCenter();
            model.calculateDimensions();
            //ofLog() << model.getNormalizedScale();
            //ofLog() << model.getSceneCenter();
        }
        else
        {
            ofLog(OF_LOG_VERBOSE) << "file load canceld";
        }

    }
    
    
    if(event.target->is("load dxf file"))
    {
        ofFileDialogResult result = ofSystemLoadDialog("Select dxf file");
        
        if (result.bSuccess)
        {
            toolpath.close();
            toolpath.clear();
            
            toolpath = dxf_reader.getOfPath(ofToDataPath(result.getPath()));
            
            toolpath.setStrokeColor(ofColor::blue);
            toolpath.setStrokeWidth(5);
            toolpath_render.set_path(toolpath);
            
            gui->getSlider("step")->setMax(1.0);
            gui->getSlider("step")->setMin(0.0);
        }
        else
        {
            ofLog(OF_LOG_VERBOSE) << "file load canceld";
        }
    }
    
    if(event.target->is("simulate tool path"))
    {
        is_simulating = !is_simulating;
    }
    
    if(event.target->is("show tool path"))
    {
        is_visible_dxf = !is_visible_dxf;
    }
    
    if(event.target->is("show 3d model"))
    {
        is_visible_3d_model = !is_visible_3d_model;
    }
    
    if(event.target->is("connect"))
    {
        if(epilogLaser.isConnected())
        {
            epilogLaser.disconnect();
        }
        else
        {
            epilogLaser.connect( gui->getTextInput("ip address")->getText(), live_mode);
        }
    }
    
    if(event.target->is("use autofacus"))
    {
        epilogLaser.getOutputConfig().use_auto_fucus = !epilogLaser.getOutputConfig().use_auto_fucus;
    }
    
    if(event.target->is("enable laser emit"))
    {
        epilogLaser.getOutputConfig().enable_laser_emit = !epilogLaser.getOutputConfig().enable_laser_emit;
    }
    
    if(event.target->is("send dxf tool path"))
    {
        if(epilogLaser.isConnected())
            epilogLaser.send(toolpath);
    }

}


void ofApp::gui_2dpad_handler(ofxDatGui2dPadEvent event)
{
    ofLog() << "getFocused " << event.target->getFocused();
    if(event.target->is("xy"))
    {
        ofLog() << event.target->getPoint();
        
        ofPoint p = event.target->getPoint();
        polyline_mouse_cursor.addVertex(p.x, p.y);
        
        if(polyline_mouse_cursor.getVertices().size() > 200)
        {
            vector<ofPoint>::iterator it = polyline_mouse_cursor.getVertices().begin();
            polyline_mouse_cursor.getVertices().erase(it);
        }
    }
}

void ofApp::gui_slider_hander(ofxDatGuiSliderEvent event)
{
    if(event.target->is("step"))
    {
        ofLog() << event.target->getValue();
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
    if(key == 'h')
    {
        gui->setVisible(!gui->getVisible());
    }
    else if(key == 'f')
    {
        ofToggleFullscreen();
    }
    else if(key == 'l')
    {
        gui->setEnabled(true);
    }
    else if(key == 'o')
    {
        if(easy_cam.getOrtho())
        {
            easy_cam.disableOrtho();
        }
        else
            easy_cam.enableOrtho();
    }
    else if(key == 'm')
    {
        is_mouse_hiding = !is_mouse_hiding;
        
        if(is_mouse_hiding)
            ofShowCursor();
        else
            ofHideCursor();
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key)
{

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y )
{

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button)
{

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button)
{

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{
    if(epilogLaser.isConnected())
    {
        epilogLaser.send(polyline_mouse_cursor);
    }
    
    polyline_mouse_cursor.clear();
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y)
{

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y)
{

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h)
{

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg)
{

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo)
{

}
