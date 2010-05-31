
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

int gPage = 0;
std::vector<int> pageQueue;

class PageHandler : public osgGA::GUIEventHandler
{
public: 
    PageHandler (PresentationParser* pres);
   virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&);
   virtual void accept(osgGA::GUIEventHandlerVisitor& v)   { v.visit(*this); };

private:
    int page;
    PresentationParser*     presentation_;
};

PageHandler::PageHandler(PresentationParser* pres) {
    presentation_ = pres;
}

bool PageHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa)
 {

    (void)aa; /* Here we use the parameter to please the compiler */

    if(gPage != presentation_->currentShow()){
        if( gPage < presentation_->fileCount()) {
        std::cout << "************* CHANGING  presentation!************** " << gPage <<std::endl;
        presentation_->load(gPage);
        } else {
            std::cout << "No such presentation!  " << gPage <<std::endl;
            gPage = presentation_->currentShow();
        }
    }

   switch(ea.getEventType())
   {
   case(osgGA::GUIEventAdapter::KEYDOWN):
      {
         switch(ea.getKey())
         {
         case 'w':
            std::cout << " w key pressed" << std::endl;
            presentation_->next();
            return false;
            break;
         default:
            return false;
         } 
      }
   default:
      return false;
   }
}

void setPage (int page) {
   
    int counter = 0;
    pageQueue.push_back(page);

    if(pageQueue.size() > 5)
        pageQueue.erase(pageQueue.begin());

    for( unsigned int i =0; i < pageQueue.size(); i++ ) {
        if(pageQueue.at(i) != gPage)
            counter++;
    }

    if(counter > 4) {
        std::cout << "*************" << gPage << " CHANGING PAGE!************** " << pageQueue.at(0) <<std::endl;
        gPage = pageQueue.at(0);
    }

}

void error(const char* msg) {
    std::cout << msg << std::endl;
}

void netCommandParser(char* com) {
    
    char delimiter = ':';
    std::string g;
    g = com;
    boost::trim(g);
    std::vector<std::string> parts = split2(g,delimiter);

    std::cout << g << std::endl;
    if(parts[0] == "page") {
        try {
            gPage = convertToInt(parts[1]);
        } catch( int i) {
            switch (i) {
            case 1: std::cout << "invalid page: " << parts[1] << std::endl;
            break;
            case 2: std::cout << "empty page number: " << std::endl;
            }
        }
    }
}

void * server_tcp(void * Arg) {

    (void)Arg; /* Here we use the parameter to please the compiler */
    int sockfd, newsockfd, clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    char port[] = "20400";

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
    error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(atoi(port));
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
          sizeof(serv_addr)) < 0) 
          error("ERROR on binding");
    listen(sockfd,5);

    while(1) {
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, 
                 (struct sockaddr *) &cli_addr, 
                 (socklen_t*)&clilen);
        if (newsockfd < 0) 
          error("ERROR on accept");
        bzero(buffer,256);
        n = read(newsockfd,buffer,255);
        if (n < 0) error("ERROR reading from socket");
        printf("Here is the message: %s\n",buffer);

        netCommandParser(buffer);

        n = write(newsockfd,"I got your message",18);
        if (n < 0) error("ERROR writing to socket");
    }
    return 0; 
}

std::string getPathName(const std::string& s) {

    char sep = '/';
    size_t i = s.rfind(sep, s.length());
    if (i != std::string::npos) {
        return(s.substr(0,i));
    }
    return "";
}

int main (int argc, char **argv ) {

//    osgDB::getDataFilePathList().push_back("/home/arihayri/IDA3/data/images");

    osg::ref_ptr<osg::Group> rootMain = new osg::Group;

    osgViewer::Viewer               viewer;
    osgViewer::Viewer::Windows      windows;
    osg::Camera*                    camera;

    std::string                     fileName, fileNameList;
    unsigned int                    width, height;
    int                             x,y,w_width,w_height;

    osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
    if (!wsi){
            osg::notify(osg::NOTICE)<<"Error, no WindowSystemInterface available, cannot create windows."<<std::endl;
            return 0;
    }
                                            
    camera = viewer.getCamera();
    viewer.getWindows(windows);
    for(osgViewer::Viewer::Windows::iterator itr = windows.begin(); itr != windows.end();++itr)
    {
        (*itr)->useCursor(false);
        (*itr)->requestWarpPointer(0,0);
        (*itr)->getWindowRectangle(x, y, w_width, w_height);
    //    (*itr)->setCursor(osgViewer::GraphicsWindow::NoCursor);

    } 
    wsi->getScreenResolution(osg::GraphicsContext::ScreenIdentifier(0), width, height);
    std::cout << "display:" << width << " " << height << std::endl;
    std::cout << "window:" << x << " " << y << std::endl;

    //windows[0]->getWindowRectangle(x, y, w_width, w_height);
   // x = (int) camera->getViewport()->x(); 
  //  std::cout << "window:" << x << " " << x << std::endl;

    //std::cout << (int)&camera->getViewport()->x << " " << (int)&camera->getViewport()->y << std::endl;


    camera->setProjectionMatrixAsPerspective(
       45.0,
       1.0,
       1.0,
       1000.0
    );
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


    PresentationParser* pp = new PresentationParser(rootMain, width, height);

    osg::ArgumentParser arguments(&argc,argv);
    if(arguments.read("--file", fileName)) {
        std::string path = getPathName(fileName);
        osgDB::getDataFilePathList().push_back(path);
        pp->insert2FileList(fileName);
        pp->load(0);
    } else if( arguments.read("--filelist", fileName)) {
        pp->readFileList(fileName);
        pp->load(0);
    } else {

        std::cout << "usage: --file or --filelist" << std::endl;
        return 0;
    }

    if(pp->getShowDuration() == 0.0) {
        std::cout << "File missing, malformed or zero duration!" << std::endl;
        return 0;
    }

  //  pp->start(); // not needed


    viewer.setSceneData ( rootMain);
   // bool  result = osgDB::writeNodeFile( * ( rootMain.get() ) , "Callback.osg" ) ;

    PageHandler* pageHandler = new PageHandler(pp);
    viewer.addEventHandler(pageHandler); 



    // säikoe
//    pthread_t thread1;
 //   int iret1 = pthread_create( &thread1, NULL, server_tcp, NULL);

while(!viewer.done()) {
    
        pp->update();
        viewer.frame();

        // loop code
        if(!pp->isRunning() && pp->getShowDuration() > 0.0) {
            pp->next();
        }

    }

}

