
#include <iostream>
#include <fstream>
#include <pthread.h>
#include <queue>

#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <osg/Material>
#include <osg/PositionAttitudeTransform>
#include <osgDB/FileUtils>
#include "tinyxml.h"
#include <curl/curl.h>

#include "smilParser.h"
#include "smilUdpClient.h"
#include <boost/algorithm/string.hpp>

int main (void) {


    osg::ref_ptr<osg::Group> rootMain = new osg::Group;

    osgViewer::Viewer               viewer;
    osgViewer::Viewer::Windows      windows;
    osg::Camera*                    camera;
    osg::ref_ptr<osg::Camera>       camera_hud;
    std::string                     fileName, fileNameList;
    unsigned int                    width, height;

    osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
    if (!wsi){
            osg::notify(osg::NOTICE)<<"Error, no WindowSystemInterface available, cannot create windows."<<std::endl;
            return 0;
    }
                                            
    camera = viewer.getCamera();
    viewer.getWindows(windows);

    wsi->getScreenResolution(osg::GraphicsContext::ScreenIdentifier(0), width, height);
    std::cout << "display:" << width << " " << height << std::endl;

    camera->setProjectionMatrixAsPerspective(
       45.0,
       1.0,
       1.0,
       1000.0
    );
    camera->setClearColor(osg::Vec4(0.1568, 0.1804, 0.2431, 0.0));
    camera->setClearColor(osg::Vec4(0.0, 0.0, 0.0, 0.0));

    // First vector is where camera is, Second vector is where the
    // camera points, Third is the camera rotation vector (up vector)
    // Magnitude matters for the first two vectors but not for the 3rd.
    // For the 2nd vector, the x, y and z aim the gaze.
    // We're aiming the gaze at the middel object in the second row!
    // For the 3rd vector, the up vector, we use (0,1,0), which means
    // to always point the top of the camera up!
    camera->setViewMatrixAsLookAt(osg::Vec3d(-60.0, 0.0, 40.0),
                  osg::Vec3d(0.0, 0.0, 0.0), osg::Vec3d(0.0, 1.0, 0.0));

    viewer.setSceneData ( rootMain);
    camera_hud = PresentationParser::createCamera(width, height);
   // bool  result = osgDB::writeNodeFile( * ( rootMain.get() ) , "Callback.osg" ) ;

    osg::ref_ptr<SmilRegionImage>  reg = new SmilRegionImage(0,0,20,20,-4,"reg1");
    osg::ref_ptr<SmilRegion3D>  reg3D = new SmilRegion3D(10,0,20,20,-2,"reg3D");
    reg->loadFile("examples/images/tux.png");
    reg3D->loadFile("examples/models/color_bars.osg");
    reg->setAlpha(1.0f);
    reg->setRegionColor(1.0f,0.0f,0.0f,1.0f);
    reg3D->setAlpha(1.0f);
    camera_hud->addChild(reg->getTransform(osg::Vec2(width, height)));
    camera_hud->addChild(reg3D->getTransform(osg::Vec2(width, height)));
    rootMain->addChild(reg3D->getCamera());

    rootMain->addChild(camera_hud);

    while(!viewer.done()) {
    
        viewer.frame();

    }

}

