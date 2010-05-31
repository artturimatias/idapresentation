#include "baseRegion.h"

BaseRegion::BaseRegion(int left, int top, int w, int h, int z, std::string id)
    :color_(new osg::Vec4Array())
{
    alphaSampler_       = new osgAnimation::FloatLinearSampler;
    regionRotSampler_   = new osgAnimation::FloatLinearSampler;
    timingSampler_      = new osgAnimation::FloatLinearSampler;
    regionPosSampler_   = new osgAnimation::Vec2LinearSampler;
    
    // zero frame
    osgAnimation::FloatKeyframeContainer* timingKeys        = timingSampler_->getOrCreateKeyframeContainer();
    timingKeys->push_back(osgAnimation::FloatKeyframe(0, 0.0f));


    regPointList["top"] = Top;
    regPointList["center"] = Center;
    regPointList["bottom"] = Bottom;

    topRegPoint_ = Center;
    // depth
    _z = z;
    id_ = id;

    // remember initial position and size
    regionSize.set(w,h);
    regionPosition.set(left,top);

    fillMode_ = "meet";

    // add transformations
    pos_ = new osg::MatrixTransform();
    rot_ = new osg::MatrixTransform();
    pos_->addChild(rot_);

    pos_->setDataVariance(osg::Object::DYNAMIC);   
    defDuration_ = 5.0;

}


void BaseRegion::parse (const TiXmlNode* xmlNode, const double time) {


    enum StringValue { Undefined,Rotation,
                        Position}; 

    std::map <std::string, int> tagList;
    tagList["none"] =  Undefined;
    tagList["position"] =  Position;
    tagList["rotation"] =  Rotation;

    osgAnimation::FloatKeyframeContainer* regionRotKeys     = regionRotSampler_->getOrCreateKeyframeContainer();
    osgAnimation::Vec2KeyframeContainer* regionPosKeys      = regionPosSampler_->getOrCreateKeyframeContainer();

    const TiXmlNode* node;
    for ( node = xmlNode->FirstChild("animate"); node; node = node->NextSibling("animate")) {
        if(node) {
            const char* const attributeName = node->ToElement()->Attribute("attributeName"); 
            float animDur = convertToFloat(node->ToElement()->Attribute("dur")); 
            float animBegin = convertToFloat(node->ToElement()->Attribute("begin")); 

            switch (tagList[attributeName]) {

            case Position: {

                osg::Vec2f fromVec = convertToVec2f(node->ToElement()->Attribute("from"));
                osg::Vec2f toVec = convertToVec2f(node->ToElement()->Attribute("to"));
                // make sure initial position stays until animation begins
                if(regionPosKeys->size() == 0) {
                    regionPosKeys->push_back(osgAnimation::Vec2Keyframe(0, regionPosition));
                    regionPosKeys->push_back(osgAnimation::Vec2Keyframe(time, regionPosition));
                } else {
                    // copy previous keyframe if there is a gap between previous key time and current time 
                    if(time !=  regionPosKeys->back().getTime()) {
                        osg::Vec2 pp = regionPosKeys->back().getValue();
                        regionPosKeys->push_back(osgAnimation::Vec2Keyframe(time,pp ));
                    }
                }

/*
                if(animBegin != 0) {
                    regionPosKeys->push_back(osgAnimation::Vec2Keyframe(time + animBegin, osg::Vec2f(100,0)));
                }
*/
                regionPosKeys->push_back(osgAnimation::Vec2Keyframe(time + animBegin, fromVec));
                regionPosKeys->push_back(osgAnimation::Vec2Keyframe(time + animBegin + animDur, toVec));
            break;
            }
             case Rotation: {

                float from = convertToFloat(node->ToElement()->Attribute("from"));
                float to = convertToFloat(node->ToElement()->Attribute("to"));
                regionRotKeys->push_back(osgAnimation::FloatKeyframe(time + animBegin, from));
                regionRotKeys->push_back(osgAnimation::FloatKeyframe(time + animBegin + animDur, to));
            break;
            }
                           
            }
        }
    }


}


void BaseRegion::start() {


    playing_ = true;
    prevVal_ = 0;
    currentItem_ = 0;
}

void BaseRegion::stop() {

    playing_ = false;
}


float BaseRegion::getDuration () {

    osgAnimation::FloatKeyframeContainer* timingKeys        = timingSampler_->getOrCreateKeyframeContainer();
    if(timingKeys->size()) {
       return timingKeys->at(timingKeys->size()-1).getTime(); 
    }
    return 0.0;

}


void BaseRegion::update(const double time) {

    if(!playing_) return;

    osgAnimation::FloatKeyframeContainer* regionRotKeys = regionRotSampler_->getOrCreateKeyframeContainer();
    osgAnimation::Vec2KeyframeContainer* regionPosKeys = regionPosSampler_->getOrCreateKeyframeContainer();
    

    float angle;
    osg::Vec2 pos;

    if(regionRotKeys->size() != 0) {
        regionRotSampler_->getValueAt(time, angle);
        setRegionRotation(angle);
    }

    if(regionPosKeys->size() != 0) {
        regionPosSampler_->getValueAt(time, pos);
        setRegionPosition(pos);
    }

}


void BaseRegion::setFadeIn (float time, float dur, float alpha) {

    osgAnimation::FloatKeyframeContainer* alphaKeys = alphaSampler_->getOrCreateKeyframeContainer();

   // set fade in keyframes
   alphaKeys->push_back(osgAnimation::FloatKeyframe(time, 0.0f));
   alphaKeys->push_back(osgAnimation::FloatKeyframe(time + dur, alpha));
}


void BaseRegion::setFadeOut (float time, float dur, float alpha) {

    osgAnimation::FloatKeyframeContainer* alphaKeys = alphaSampler_->getOrCreateKeyframeContainer();
    // set fade out keyframes
   alphaKeys->push_back(osgAnimation::FloatKeyframe(time  - dur, alpha));
   alphaKeys->push_back(osgAnimation::FloatKeyframe(time , 0.0f));


}



void BaseRegion::setZ(float z) {

  _z = z;
}



osg::MatrixTransform* BaseRegion::getTransform(osg::Vec2 displayS) {

    // set dipslay size so that the top can be calculated
    displaySize = displayS;

    // initial pixel size   
    regionSizePixels.x() = displaySize.x()*(regionSize.x()/100);
    regionSizePixels.y() = displaySize.y()*(regionSize.y()/100);

    //set initial position
    setRegionPosition(regionPosition);
    setRegionSize(regionSize.x(), regionSize.y());

	return pos_;
}



void BaseRegion::setRegionRotation(float angle) {

    osg::Matrix rotation ;
    rotation.makeRotate(osg::Quat(osg::DegreesToRadians(angle), osg::Vec3(0,0,1)));
    rot_->setMatrix(rotation);
}

void BaseRegion::setRegionPosition (const osg::Vec2 pos) {

    // position is percentage of display or parent

    float left = pos.x();
    float top = pos.y();
    left = (displaySize.x()*left/100) + (displaySize.x()*regionSize.x()/100/2);
    top = (displaySize.y() - displaySize.y()*regionSize.y()/100/2) - (displaySize.y()*top/100);
    osg::Matrix trans ;
    trans.makeTranslate(osg::Vec3(left,top,_z));
    pos_->setMatrix(trans);
 
}

	

osg::Vec2d&  BaseRegion::getRegionPosition()  {
  return regionPosition;
}

osg::Vec2d&  BaseRegion::getRegionSize()  {
  return regionSize;
}

void BaseRegion::setRegionColor(float r, float g, float b){
    regionColor.set(r,g,b);
}







