#ifndef SMILREGION3D_H
#define SMILREGION3D_H

#include <osg/PositionAttitudeTransform>
#include <osg/MatrixTransform>
#include "smilRegion.h"
#include "cameraCallback.h"
#include <boost/bind.hpp>

enum StringValue {  SetTransform, 
                    Animate3D,
                    Animate3DCamera};


class SmilRegion3D:  public SmilRegion {
public:
    SmilRegion3D(int left, int top, int w, int h, int z, std::string id);

    // virtual methods
    void update             (const double time);              
    void parse              (const TiXmlNode* xmlNode, const double time);

    void loadFile           (const std::string& filename );
    void setupCamera        ();
    void parse3D            (const TiXmlNode* xmlNode);
    void parse3DCamera      (const TiXmlNode* xmlNode);
    void set3dKeys          (const TiXmlNode* xmlNode, osgAnimation::Vec3KeyframeContainer* keys);

    osg::Camera* getCamera  ();
    osg::Node* findNamedNode(const std::string& searchName, osg::Node* currNode);
    osg::MatrixTransform* findAndAddTransform(const char* const nodeName);

private:

    osg::ref_ptr<osg::Texture2D>            tex_;
    osg::ref_ptr<osg::Camera>               camera;
    osg::ref_ptr<osg::MatrixTransform>      modelTransform_;
        
    std::vector<const TiXmlNode*>           xmlNodes_;
    std::vector<std::string>                mediaItems_;
    std::map <std::string, int>              tagList;

};

#endif
