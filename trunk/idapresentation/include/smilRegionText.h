
#ifndef SMILREGIONTEXT_H
#define SMILREGIONTEXT_H

#include <osgText/Text>
#include "baseRegion.h"

class SmilRegionText: public BaseRegion {
public:
    SmilRegionText(int left, int top, int w, int h, int z, std::string id);
 
    virtual void parse              (const TiXmlNode* xmlNode, const double time);
    virtual void update             (const double time);

    virtual void setRegionSize      (float x, float y);

    void setText                    (const std::string& text);
    void setText                    (const float text);
    void setFont                    (const std::string& font);

    void setTextSize                (unsigned int size);
    void setColor                   (float r, float g, float b, float a);
    void setAlpha                   (float alpha);

protected:
private:
    osg::ref_ptr<osg::Geode>        textGeode;
    osg::ref_ptr<osgText::Text>     text;
    std::vector<std::string>        mediaItems_;

};
#endif
