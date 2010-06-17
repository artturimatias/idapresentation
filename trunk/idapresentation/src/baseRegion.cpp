#include "baseRegion.h"

BaseRegion::BaseRegion(int left, int top, int w, int h, int z, std::string id)
    :color_(new osg::Vec4Array())
{
    alphaSampler_       = new osgAnimation::FloatLinearSampler;
    regionRotSampler_   = new osgAnimation::FloatLinearSampler;
    timingSampler_      = new osgAnimation::FloatLinearSampler;
    regionPosSampler_   = new osgAnimation::Vec3LinearSampler;
    
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
    regionPosition.set(left,top, z);

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

    float mainDur = convertToFloat(xmlNode->ToElement()->Attribute("dur"));
    float mediaBegin = convertToFloat(xmlNode->ToElement()->Attribute("begin"));

    osgAnimation::FloatKeyframeContainer* regionRotKeys     = regionRotSampler_->getOrCreateKeyframeContainer();
    osgAnimation::Vec3KeyframeContainer* regionPosKeys      = regionPosSampler_->getOrCreateKeyframeContainer();

    // set fades if no alpha keys are not set
    if(!parseAlpha(xmlNode, time)) {
        setFadeIn(time + mediaBegin, 1.0, 1.0);
        setFadeOut(time + mediaBegin + mainDur, 1.0, 1.0);
    }

    const TiXmlNode* node;
    for ( node = xmlNode->FirstChild("animate"); node; node = node->NextSibling("animate")) {
        if(node) {
            const char* const attributeName = node->ToElement()->Attribute("attributeName"); 

            switch (tagList[attributeName]) {

                case Position: {
                    osg::Vec3 fromVec, toVec;

                    // first key value is the initial region position, not zero
                    if(regionPosKeys->size() == 0) {
                        regionPosKeys->push_back(osgAnimation::Vec3Keyframe(0, getRegionPosition()));
                        regionPosKeys->push_back(osgAnimation::Vec3Keyframe(time, getRegionPosition()));
                    }

                    if(parseFromTo(node, fromVec, "from", "region_position")) {
                        insertFromToKey(node, fromVec, "from", regionPosKeys, time);
                    }

                    if(parseFromTo(node, toVec, "to", "region_position")) {
                        insertFromToKey(node, toVec, "to", regionPosKeys, time);
                    }
                    break;
                }
                 case Rotation: {

                    const char* fromStr = node->ToElement()->Attribute("from");
                    const char* toStr = node->ToElement()->Attribute("to");

                    if(fromStr)
                        insertFromToKey(node, convertToFloat(fromStr), "from", regionRotKeys, time);
                    if(toStr)
                        insertFromToKey(node, convertToFloat(toStr), "to", regionRotKeys, time);

                    break;
                }
            }
        }
    }
}


bool BaseRegion::parseAlpha(const TiXmlNode* xmlNode, const double time) {
        
    const TiXmlNode* node;
    bool foundAlpha = false;

    for ( node = xmlNode->FirstChild("animate"); node; node = node->NextSibling("animate")) {
        if(node) {
            const char* const fromStr = node->ToElement()->Attribute("from"); 
            float toVal = convertToFloat(node->ToElement()->Attribute("to")); 
            float dur = convertToFloat(node->ToElement()->Attribute("dur")); 
            float wait = convertToFloat(node->ToElement()->Attribute("wait")); 
            std::string attrStr = node->ToElement()->Attribute("attributeName"); 

            if(attrStr == "alpha") {

                foundAlpha = true;
                osgAnimation::FloatKeyframeContainer* keys = alphaSampler_->getOrCreateKeyframeContainer();

                // add zero key
                if(keys->size() == 0) {
                    keys->push_back(osgAnimation::FloatKeyframe(time, 0.0f));
                }


                float lastKeyTime = keys->at(keys->size()-1).getTime();
                // if we have "wait" state then we must copy the last key value before we insert "from" value
                if(wait != 0 ) {
                    keys->push_back(osgAnimation::FloatKeyframe(lastKeyTime + wait, keys->at(keys->size()-1).getValue()));
                }

                keys->push_back(osgAnimation::FloatKeyframe(lastKeyTime + wait + dur , toVal));
            }
        }
    }

    return foundAlpha;

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
    osgAnimation::Vec3KeyframeContainer* regionPosKeys = regionPosSampler_->getOrCreateKeyframeContainer();
    

    float angle;
    osg::Vec3 pos;

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

void BaseRegion::setRegionPosition (const osg::Vec3 pos) {

    // position is percentage of display or parent

    float left = pos.x();
    float top = pos.y();
    left = (displaySize.x()*left/100) + (displaySize.x()*regionSize.x()/100/2);
    top = (displaySize.y() - displaySize.y()*regionSize.y()/100/2) - (displaySize.y()*top/100);
    osg::Matrix trans ;
    trans.makeTranslate(osg::Vec3(left,top,_z));
    pos_->setMatrix(trans);
 
}

	

osg::Vec3d&  BaseRegion::getRegionPosition()  {
  return regionPosition;
}

osg::Vec2d&  BaseRegion::getRegionSize()  {
  return regionSize;
}

void BaseRegion::setRegionColor(float r, float g, float b){
    regionColor.set(r,g,b);
}


bool BaseRegion::parseFromTo(const TiXmlNode* xmlNode, osg::Vec3& vec, const char* attr, const char* channelType) {

    char delimiter = ',';
    const char* const str = xmlNode->ToElement()->Attribute(attr); 
    if(str) {
        std::vector<std::string> valueList = split2(str,delimiter);
        if(strcmp(channelType, "euler") == 0) {
            vec.set(
                    osg::DegreesToRadians(convertToFloat(valueList[0])),
                    osg::DegreesToRadians(convertToFloat(valueList[1])),
                    osg::DegreesToRadians(convertToFloat(valueList[2])));

        } else if(strcmp(channelType, "position") == 0){

            vec.set(    
                    convertToFloat(valueList[0]), 
                    convertToFloat(valueList[1]),
                    convertToFloat(valueList[2]));
        } else {    // region position

            vec.set(    
                    convertToFloat(valueList[0]), 
                    convertToFloat(valueList[1]),
                    0.0f);

        }

        return true;

    } else {
        return false;
    }

}




void BaseRegion::insertFromToKey(const TiXmlNode* node, float val, const char* attr, osgAnimation::FloatKeyframeContainer* keys, const double time) {

    float dur = convertToFloat(node->ToElement()->Attribute("dur"));
    float wait = convertToFloat(node->ToElement()->Attribute("wait"));


     // add zero key
    if(keys->size() == 0) {
        if(strcmp(attr, "from") == 0) {
            keys->push_back(osgAnimation::FloatKeyframe(0, val));
            keys->push_back(osgAnimation::FloatKeyframe(time, val));
        } else {
            keys->push_back(osgAnimation::FloatKeyframe(0, 0.0f));
            keys->push_back(osgAnimation::FloatKeyframe(time, 0.0f));
        }
    }

    float lastKeyTime = keys->at(keys->size()-1).getTime();

    std::cout << "Setting key \"" << attr << "\" at time: " << time << " lastKeyTime: " << lastKeyTime << std::endl;
    // if we have "wait" state then we must copy the last key value before we insert "from" value
    if(wait != 0 ) {
        keys->push_back(osgAnimation::FloatKeyframe(lastKeyTime + wait, keys->at(keys->size()-1).getValue()));
    }

    if(strcmp(attr, "from") == 0) {
        keys->push_back(osgAnimation::FloatKeyframe(lastKeyTime + wait , val));   
    } else {
        keys->push_back(osgAnimation::FloatKeyframe(lastKeyTime + wait + dur , val));   
    }
}



void BaseRegion::insertFromToKey(const TiXmlNode* node, osg::Vec3& vec, const char* attr, osgAnimation::Vec3KeyframeContainer* keys, const double time) {

    float dur = convertToFloat(node->ToElement()->Attribute("dur"));
    float wait = convertToFloat(node->ToElement()->Attribute("wait"));

    osg::Vec3 zeroValues = osg::Vec3(0.0,0.0,0.0);

     // add zero key
    if(keys->size() == 0) {
        if(strcmp(attr, "from") == 0) {
            keys->push_back(osgAnimation::Vec3Keyframe(0, vec));
            keys->push_back(osgAnimation::Vec3Keyframe(time, vec));
        } else {
            keys->push_back(osgAnimation::Vec3Keyframe(0, zeroValues));
            keys->push_back(osgAnimation::Vec3Keyframe(time, zeroValues));
        }
    }

    float lastKeyTime = keys->at(keys->size()-1).getTime();

    std::cout << "Setting key \"" << attr << "\" at time: " << time << " lastKeyTime: " << lastKeyTime << std::endl;
    // if we have "wait" state then we must copy the last key value before we insert "from" value
    if(wait != 0 ) {
        keys->push_back(osgAnimation::Vec3Keyframe(lastKeyTime + wait, keys->at(keys->size()-1).getValue()));
    }

    if(strcmp(attr, "from") == 0) {
        keys->push_back(osgAnimation::Vec3Keyframe(lastKeyTime + wait , vec));   
    } else {
        keys->push_back(osgAnimation::Vec3Keyframe(lastKeyTime + wait + dur , vec));   
    }
}








