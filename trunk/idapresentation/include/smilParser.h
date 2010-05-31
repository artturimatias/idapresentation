//###########################################################################
//  File:       smilParser.h
//  Purpose:    This class reads SMIL and creates regions and image and text 
//  containers
//############################################################################

#ifndef SMILPARSER_H
#define SMILPARSER_H
#include <osgDB/ReadFile>

#include <string>
#include <iostream>
#include <vector>
#include "tinyxml.h"
#include "func.h"
#include "smilNet.h"
#include "smilRegionText.h"
#include "smilRegionImage.h"
#include "smilRegion3D.h"

//typedef std::pair<std::string, osg::ref_ptr<SmilFlow> > ff;


class PresentationParser: public osg::Referenced {
public:
    PresentationParser(osg::Group* root, unsigned int width, unsigned int height);

    void load               (const std::string& filename );
    void load               (int fileIndex);
    void readFileList       (std::string list );
    void insert2FileList    (std::string list );
    void preParse           (TiXmlNode* node);
    void parseLayout        (TiXmlNode* node);
    float parseTimeline     (TiXmlNode* node, float time);
    void start              ();
    void stop               ();
    void next               ();
    void update             ();
    void clear              ();
    bool isRunning          () { return running_; }
    int currentShow         () { return currentFile_; }
    int fileCount           () { return fileList_.size(); }
    float getShowDuration   () { return showDuration_; }
    void getRightTime       (TiXmlNode* node, TiXmlNode* parent,  float& timeLineTime, float& duration);
    static osg::ref_ptr<osg::Camera>    createCamera(unsigned int width, unsigned int height);
    osg::ref_ptr<osg::Camera>           getCamera() {return camera_; }

private:
    bool                                        running_;
    float                                       showDuration_;
    float                                       currentFile_;
    std::string                                 showDurationStr_;
    std::string                                 fileName_;
    osg::Vec2d                                  displaySize;   
    osg::Timer_t                                startTime_;
    osg::Timer_t                                currentTime_;
    std::map<std::string, int>                  tagList;
    std::map<int, std::string>                  fileList_;
    TiXmlDocument*                              doc_;
    osg::Group*                                 mainNode_;
    SmilRegionText*                             debugText_;
    osg::ref_ptr<osg::Camera>                   camera_;
    std::map<std::string, osg::ref_ptr<BaseRegion> >    regions_;
};
#endif
