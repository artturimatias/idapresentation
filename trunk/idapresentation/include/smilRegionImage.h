
#ifndef SMILREGIONIMAGE_H
#define SMILREGIONIMAGE_H
#include "smilRegion.h"

class SmilRegionImage: public SmilRegion {
public:
    SmilRegionImage(int left, int top, int w, int h, int z, std::string id);
    
    void update               (const double time);
    void parse               (const TiXmlNode* xmlNode, const double time);  

    void setFit               (std::string& fit);
    void resize               (int s, int t);   
    float getTopOffset        (float x);
    void loadFile             (const std::string& filename );

private:

    std::vector<std::string>            mediaItems_;

    osg::ref_ptr<osg::Texture2D>        tex_;
    osg::ref_ptr<osg::TexMat>           texMat_;
    osg::ref_ptr<osg::Image>            image_;

    osg::Matrix          		texMatInit_;

};
#endif
