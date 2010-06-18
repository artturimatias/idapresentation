#include "smilRegionText.h"

SmilRegionText::SmilRegionText(int left, int top, int w, int h, int z, std::string id)
  :BaseRegion(left, top, w, h , z, id),
  textGeode(new osg::Geode()),
  text(new osgText::Text())
{
    textGeode->addDrawable(text);
    
    //Set the screen alignment - always face the screen
    text->setAxisAlignment(osgText::Text::SCREEN);
    
    //Set the text to our default text string
    text->setText("Default Text");
    text->setFont("DejaVuSans.ttf");
    text->setDataVariance(osg::Object::DYNAMIC);   
    text->setAlignment(osgText::Text::LEFT_TOP); 
    text->setLineSpacing(0.25); 
    setTextSize(18);
    setColor(1.0,1.0,1.0, 1.0f);
    rot_->addChild(textGeode);

}


void SmilRegionText::parse (const TiXmlNode* xmlNode, const double time) {

    BaseRegion::parse(xmlNode, time);
    float dur = convertToFloat(xmlNode->ToElement()->Attribute("dur"));
    float mediaBegin = convertToFloat(xmlNode->ToElement()->Attribute("begin"));
    std::string textVal = convertToString(xmlNode->ToElement()->GetText());
    osgAnimation::FloatKeyframeContainer* timingKeys        = timingSampler_->getOrCreateKeyframeContainer();
    
    osg::notify(osg::WARN) <<"adding text:" << textVal << " at keyframe time: " << time + mediaBegin << " id: " << mediaItems_.size() << std::endl;

    mediaItems_.push_back(textVal);
    timingKeys->push_back(osgAnimation::FloatKeyframe(time + mediaBegin + dur, (float)mediaItems_.size()));



}



void SmilRegionText::update(const double time) {

    if(!playing_) return;

    float timingVal, alphaVal, angle;

    // position and rotation
    BaseRegion::update(time);

    osgAnimation::FloatKeyframeContainer* regionRotKeys = regionRotSampler_->getOrCreateKeyframeContainer();
    if(regionRotKeys->size() != 0) {
        regionRotSampler_->getValueAt(time, angle);
   //     text->setRotation(osg::Quat(osg::DegreesToRadians(angle), osg::Vec3(0,0,1)));
    }


    // timing
    timingSampler_->getValueAt(time, timingVal);

    if(mediaItems_.size() == 0) {
        return;
    }

    if( timingVal > currentItem_  || timingVal < prevVal_ ) {

        if(currentItem_ >= mediaItems_.size())
              currentItem_ = 0;

        setText(mediaItems_.at(currentItem_));

        prevVal_ = timingVal;
        currentItem_++;
    }

    // alpha
    osgAnimation::FloatKeyframeContainer* alphaKeys        = alphaSampler_->getOrCreateKeyframeContainer();
    if(alphaKeys->size() == 0)
        return;


    alphaSampler_->getValueAt(time, alphaVal);
    text->setColor(osg::Vec4(regionColor, alphaVal));


}

void SmilRegionText::setRegionSize(float x, float y){

    float textTop = displaySize.y()*regionSize.y()/100/2;
    // fix text position
    text->setPosition(osg::Vec3(displaySize.x()*regionSize.x()/100/2*(-1), textTop ,0));

    text->setMaximumWidth(displaySize.x()*regionSize.x()/100);

}

void SmilRegionText::setText(const std::string& t){
    text->setText(t,osgText::String::ENCODING_UTF8);
}

void SmilRegionText::setText(const float t){
    text->setText(convertToString(t));
}


void SmilRegionText::setAlpha(float alpha) {

    text->setColor(osg::Vec4(regionColor, alpha));
}

void SmilRegionText::setTextSize(unsigned int size){

    if(size != 0)
        text->setCharacterSize(size);

    // TODO: make this relative to screen size
    text->setFontResolution(128,128); 
}


void SmilRegionText::setFont(const std::string& font){
    text->setFont(font);
}


void SmilRegionText::setColor(float r, float g, float b, float a){

    regionColor.set(r,g,b);
    text->setColor(osg::Vec4(regionColor, a));
}




