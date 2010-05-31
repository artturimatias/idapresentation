
#ifndef BASEREGION_H
#define BASEREGION_H
#include <osg/Geode>
#include <osg/Geometry>
#include <osgDB/ReadFile>
#include <osg/Material>
#include <osg/TexMat>
#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include <osgUtil/RenderBin>
#include <osg/Timer>
#include <osgAnimation/Sampler>

#include <string>
#include <iostream>

#include "tinyxml.h"
#include "func.h"

enum TopRegPoint {  Top,
                    Center,
                    Bottom 
                };


class BaseRegion: public osg::Referenced {
public:
    BaseRegion(int left, int top, int w, int h, int z, std::string id);
    
    void setRegionPosition          (const osg::Vec2 pos);
    void setRegionRotation          (float angle);
    void setRegionColor             (float r, float g, float b);
    void setFadeIn                  (float time, float dur, float alpha);
    void setFadeOut                 (float time, float dur, float alpha);
    void setZ                       (float z);
    void start                      () ;
    void stop                       () ;
    float getZ                      () { return _z; }
    void setDefDuration             (float dur) {defDuration_ = dur;}
    float getDefDuration            () {return defDuration_;}


    float getDuration               ();
    osg::Vec2d& getRegionPosition   ();
    osg::Vec2d& getRegionSize       ();
   
    osg::MatrixTransform*  getTransform(osg::Vec2 displayS);
    std::string getId               ()         { return id_ ;}

    virtual void parse              (const TiXmlNode* xmlNode, const double time);
    virtual void update             (const double time);

    virtual void setRegionSize      (float x, float y) = 0;
protected:

    osg::Vec2d       regionSize;   
    osg::Vec2d       regionSizePixels;   
    osg::Vec2d       regionPosition;
    osg::Vec3d       regionColor;
    osg::Vec2d       displaySize;   
    float            _z;
    float            _zalpha;
    std::string      fillMode_;
    std::string      id_;
    int              topRegPoint_;
    float            prevVal_;
    unsigned int     currentItem_;
    bool             playing_;
    float            defDuration_;

    std::map<std::string, int>          regPointList;
    osg::ref_ptr<osg::MatrixTransform>  pos_;
    osg::ref_ptr<osg::MatrixTransform>  rot_;

    osg::ref_ptr<osg::Vec4Array>                    color_;
    osg::ref_ptr<osgAnimation::FloatLinearSampler>  regionRotSampler_;
    osg::ref_ptr<osgAnimation::FloatLinearSampler>  timingSampler_;
    osg::ref_ptr<osgAnimation::FloatLinearSampler>  alphaSampler_;
    osg::ref_ptr<osgAnimation::Vec2LinearSampler>   regionPosSampler_;


};
#endif
