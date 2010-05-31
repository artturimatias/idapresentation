
#ifndef SMILREGION_H
#define SMILREGION_H

#include "baseRegion.h"

class SmilRegion: public BaseRegion {
public:
    SmilRegion(int left, int top, int w, int h, int z, std::string id);
 
    virtual void parse                (const TiXmlNode* xmlNode, const double time);
    virtual void update               (const double time);

    void setRegionColor               (float r, float g, float b, float a);
    void setAlpha                     (float alpha);
    void setRegionSize                (float x, float y);

protected:

    osg::ref_ptr<osg::Geode>            HE_Geode;
    osg::ref_ptr<osg::Geometry>         HE_Geometry;
    osg::ref_ptr<osg::Vec3Array>        HE_Vertices;
    osg::ref_ptr<osg::Vec2Array>        texCoords;
    osg::ref_ptr<osg::DrawElementsUInt> HE_Indices;
    osg::ref_ptr<osg::Vec3Array>        HE_Normals;
    osg::ref_ptr<osg::StateSet>         stateSet_;



};
#endif
