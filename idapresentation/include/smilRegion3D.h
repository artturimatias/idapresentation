//###########################################################################
//  File:       hud_element.h
//  Purpose:    This class represents an individual HUD Element which represents
//              a 2D rectangle painted on the front of the screen, on top of
//              all other SceneData. The HUD Element may also contain Text Box
//              elements. Any Text Box element attached to this HUD Element
//              will automatically be cleaned up upon class destruction.
//############################################################################

#ifndef SMILREGION3D_H
#define SMILREGION3D_H

#include <osg/PositionAttitudeTransform>
#include <osg/MatrixTransform>
#include <osgAnimation/BasicAnimationManager>
#include <osgAnimation/UpdateCallback> 
#include "smilRegion.h"
#include "cameraCallback.h"
#include <boost/bind.hpp>

enum StringValue {  SetTransform, 
                    Animate3D,
                    Animate3DCamera};

typedef std::map<std::string, osg::ref_ptr<osgAnimation::Vec3LinearChannel> > MapVec3;
typedef std::map<std::string, osg::ref_ptr<osgAnimation::Vec4LinearChannel> > MapVec4;

class SmilRegion3D:  public SmilRegion {
public:
    SmilRegion3D(int left, int top, int w, int h, int z, std::string id);

    // virtual methods
    void update           (const double time);              
    void parse            (const TiXmlNode* xmlNode, const double time);

    void loadFile         (const std::string& filename );
    void setupCamera      ();
    void parse3D          (const TiXmlNode* xmlNode, const double time);
    void set3dKeys  (const TiXmlNode* xmlNode, osgAnimation::Vec3LinearChannel* chPosition, const double time);

    osg::Camera* getCamera();
    osg::Node* findNamedNode(const std::string& searchName, osg::Node* currNode);
    osg::MatrixTransform* findAndAddTransform(const char* const nodeName);

private:

    osg::ref_ptr<osg::Texture2D>            tex_;
    osg::ref_ptr<osg::Camera>               camera;
    osg::ref_ptr<osg::MatrixTransform>      modelTransform_;
        
    std::vector<const TiXmlNode*>           xmlNodes_;
    std::vector<std::string>                mediaItems_;
    std::map <std::string, int>              tagList;

    osg::ref_ptr<osgAnimation::BasicAnimationManager> manager;
};

#endif