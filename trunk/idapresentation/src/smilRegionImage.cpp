#include "smilRegionImage.h"

SmilRegionImage::SmilRegionImage(int left, int top, int w, int h, int z, std::string id)
  :SmilRegion(left, top, w, h, z, id)
{
    tex_ = new osg::Texture2D();
    texMat_ = new osg::TexMat;

    // make sure that alpha channel is working
    tex_->setInternalFormat(GL_RGBA);
    //tex_->setResizeNonPowerOfTwoHint(false);

    tex_->setWrap( osg::Texture2D::WRAP_S , osg::Texture2D::REPEAT );
    tex_->setWrap( osg::Texture2D::WRAP_T , osg::Texture2D::REPEAT );

    stateSet_->setTextureAttributeAndModes(0,tex_.get(),osg::StateAttribute::ON);
    stateSet_->setTextureAttributeAndModes(0, texMat_,osg::StateAttribute::ON);
    stateSet_->setAttribute(new osg::Material, true);

    texMat_->setMatrix(osg::Matrixd::scale(osg::Vec3d(1,1,0.0)));
    stateSet_->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
    osg::Material* material = dynamic_cast<osg::Material*>(stateSet_->getAttribute(osg::StateAttribute::MATERIAL));
    if(material) {
        
        material->setAlpha(osg::Material::FRONT_AND_BACK, 0.0f);
    }

    HE_Geometry->setColorBinding (osg::Geometry::BIND_OFF);


}

void SmilRegionImage::parse (const TiXmlNode* xmlNode, const double time) {

    SmilRegion::parse(xmlNode, time);

    osgAnimation::FloatKeyframeContainer* timingKeys        = timingSampler_->getOrCreateKeyframeContainer();

    float dur = convertToFloat(xmlNode->ToElement()->Attribute("dur"));
    float mediaBegin = convertToFloat(xmlNode->ToElement()->Attribute("begin"));
    const char* fileName = xmlNode->ToElement()->Attribute("src"); 

    // get file name and load
    if(fileName) {

        osg::notify(osg::WARN) <<"adding image:" << fileName << " at keyframe time: " << time + mediaBegin << " id: " << mediaItems_.size() << std::endl;
        mediaItems_.push_back(fileName);
        
        // set timing keyframe
        timingKeys->push_back(osgAnimation::FloatKeyframe(time + mediaBegin + dur, (float)mediaItems_.size()));
    }

/*
    // set panzoom animation
    panZoomKeys->push_back(osgAnimation::Vec4Keyframe(time, osg::Vec4(0,0,1,1)));

    const TiXmlNode* node;
    for ( node = xmlNode->FirstChild("animate"); node; node = node->NextSibling("animate")) {
        if(node) {
            const char* const attributeName = node->ToElement()->Attribute("attributeName"); 
            float animDur = convertToFloat(node->ToElement()->Attribute("dur")); 
            float animBegin = convertToFloat(node->ToElement()->Attribute("begin")); 

            switch (tagList[attributeName]) {

            case PanZoom: {

                    const char* const fromStr = node->ToElement()->Attribute("from"); 
                    const char* const toStr = node->ToElement()->Attribute("to"); 

                    animBegin += mediaBegin; // animation starts after media element's begin value


                    if(fromStr && toStr) {
                        
                        osg::Vec4 fromVec; 
                        osg::Vec4 toVec; 

                        std::vector<std::string> fromList = split2(fromStr,delimiter);
                        std::vector<std::string> toList = split2(toStr,delimiter);

                        fromVec.set(convertToFloat(fromList[0])/100, 
                                    convertToFloat(fromList[1])/100,
                                    convertToFloat(fromList[2])/100,
                                    convertToFloat(fromList[3])/100);

                        toVec.set(  convertToFloat(toList[0])/100, 
                                    convertToFloat(toList[1])/100,
                                    convertToFloat(toList[2])/100,
                                    convertToFloat(toList[3])/100);

                        float animEnd;
                        if((animBegin+animDur) > (mediaBegin+dur))
                            animEnd = mediaBegin+dur;
                        else
                            animEnd = animBegin+animDur;

                        std::cout << "setting panzoom keys time:" << time+animBegin << " fromx:" <<  fromVec[0] << " toX:" << toVec[0] << std::endl;
                        std::cout << "seetting panzoom keys time:" << animEnd << " fromY:" <<  fromVec[1] << " toY:" << toVec[1] << std::endl;

                       panZoomKeys->push_back(osgAnimation::Vec4Keyframe(time + animBegin, fromVec));
                       panZoomKeys->push_back(osgAnimation::Vec4Keyframe(time + animEnd, toVec));
                    }

            break;
            }
            }            
        }
    }
    */

}

void SmilRegionImage::update(const double time) {

    if(!playing_) return;

    float timingVal, alphaVal;

    osgAnimation::FloatKeyframeContainer* alphaKeys        = alphaSampler_->getOrCreateKeyframeContainer();

    // position and rotation
    SmilRegion::update(time);

    // timing
    timingSampler_->getValueAt(time, timingVal);

    if(mediaItems_.size() == 0) {
        return;
    }

    //std::cout << "timingval " << timingVal << "time " << time << "currenitem " << currentItem_ << std::endl;

    if( timingVal > currentItem_  || timingVal < prevVal_ ) {
        if(currentItem_ >= mediaItems_.size()   )
              currentItem_ = 0;

        // reset panzoom
        texMat_->setMatrix(osg::Matrix::identity());
        loadFile(mediaItems_.at(currentItem_));

        prevVal_ = timingVal;
        currentItem_++;
    }
    
    // panzoom
    osg::Vec4 vec;
    osgAnimation::Vec4KeyframeContainer* panZoomKeys = panZoomSampler_->getOrCreateKeyframeContainer();
    
    // return if we do not have keyframes
    if(panZoomKeys->size() != 0) {

        panZoomSampler_->getValueAt(time, vec);

        // update texture matrix
        osg::Matrix m;
        m.makeScale(osg::Vec3d(vec[2],vec[3],0.0));
        m.postMultTranslate(osg::Vec3d(vec[0], vec[1] ,0.0));
        texMat_->setMatrix(m);
    }

    // alpha
    if(alphaKeys->size() == 0)
        return;

    alphaSampler_->getValueAt(time, alphaVal);
    osg::Material* material = dynamic_cast<osg::Material*>(stateSet_->getAttribute(osg::StateAttribute::MATERIAL));
    if(material) {
        
        material->setAlpha(osg::Material::FRONT_AND_BACK, alphaVal);
    }


}


void SmilRegionImage::loadFile( const std::string& filename ) {

    osg::notify(osg::WARN) << "loading image:" << filename << std::endl;
    image_ = osgDB::readImageFile(filename);
    // texture
    if(!image_.valid()) {
        osg::notify(osg::WARN) << "in Image::loadImage(), failed to load image:" << filename << std::endl;
        return;
    } else {

            float s = image_->s();
            float t = image_->t();

            std::cout << "image size: " << s << " x " << t << std::endl; 
            tex_->setTextureSize(s,t);
            tex_->setImage(image_);
            resize(s, t);
    }
    //image_->ensureValidSizeForTexturing(1024);

}



void SmilRegionImage::setFit(std::string& fit) {

    fillMode_ = fit;
}



void SmilRegionImage::resize(int s, int t) {

    // calculate new size
   float w = regionSize.x(); 
   float h = regionSize.y(); 

  float frameRatio = regionSizePixels.x() / regionSizePixels.y();
  float imageRatio = (float)s/(float)t;


//   osg::notify(osg::ALWAYS) << "ruutusuhde:" << regionSizePixels.x() << "/" << regionSizePixels.y() << "=" << frameRatio <<std::endl;
 //  osg::notify(osg::ALWAYS) << "kuvasuhde:" << s << "/" << t << "=" << imageRatio <<std::endl;


  if(fillMode_ == "meet") { // change region size to fit

      if( imageRatio > frameRatio) {

        // image is wider than region > decrease the height of the region to fit
        float widthAsPixels = regionSizePixels.x()/imageRatio;
        setRegionSize(w, widthAsPixels/displaySize.y()*100);

      } else {

        // image is hiegher than region > decrease the width of the region to fit
        float heightAsPixels = regionSizePixels.y()*imageRatio;
        setRegionSize(heightAsPixels/displaySize.x()*100, h);  // NOTE: percentage values

      }
    } else if (fillMode_ == "slice") { // scale texture coordinates to fit
//       osg::notify(osg::ALWAYS) << "ruutusuhde:" << w << "/" << h << "=" << frameRatio <<std::endl;
//       osg::notify(osg::ALWAYS) << "kuvasuhde:" << s << "/" << t << "=" << imageRatio <<std::endl;

      if( imageRatio > frameRatio) {

/*
       osg::notify(osg::ALWAYS) << "kuva on leveempi:" << s << "/" << t << "=" << imageRatio <<std::endl;
        osg::Matrix m;
        m.makeScale(osg::Vec3d(imageRatio,1,0.0));
        texMat_->setMatrix(m);
*/

      } else {
/*
       osg::notify(osg::ALWAYS) << "kuva on korkeempi:" << s << "/" << t << "=" << imageRatio <<std::endl;
        osg::Matrix m;
        m.makeScale(osg::Vec3d(1,imageRatio,0.0));
        texMat_->setMatrix(m);
*/

      }
    } else if (fillMode_ == "fill") { // fill is the default, distorts image

    }
}


float SmilRegionImage::getTopOffset(float x) {

    if(topRegPoint_ == Top)
        return x;
    else if (topRegPoint_ == Center)
        return x/2;
    else return 0;
}




