#include "smilRegionImage.h"

SmilRegionImage::SmilRegionImage(int left, int top, int w, int h, int z, std::string id)
  :SmilRegion(left, top, w, h, z, id)
{
    tex_ = new osg::Texture2D();
    texMat_ = new osg::TexMat;
    texMat_->setMatrix(osg::Matrix::identity());

    // make sure that alpha channel is working
    tex_->setInternalFormat(GL_RGBA);

    tex_->setWrap( osg::Texture2D::WRAP_S , osg::Texture2D::REPEAT );
    tex_->setWrap( osg::Texture2D::WRAP_T , osg::Texture2D::REPEAT );

    stateSet_->setTextureAttributeAndModes(0,tex_.get(),osg::StateAttribute::ON);
    stateSet_->setTextureAttributeAndModes(0, texMat_,osg::StateAttribute::ON);
    stateSet_->setAttribute(new osg::Material, true);

    stateSet_->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
    osg::Material* material = dynamic_cast<osg::Material*>(stateSet_->getAttribute(osg::StateAttribute::MATERIAL));
    if(material) {
        
        material->setAlpha(osg::Material::FRONT_AND_BACK, 0.0f);
    }

    HE_Geometry->setColorBinding (osg::Geometry::BIND_OFF);

    // load ffmpeg plugin
//    std::string libName = osgDB::Registry::instance()->createLibraryNameForExtension("ffmpeg"); 
  //  osgDB::Registry::instance()->loadLibrary(libName); 


}

void SmilRegionImage::setShader(const std::string& filename ) {

   osg::Program* brickProgramObject = new osg::Program;
   osg::Shader* brickVertexObject = 
      new osg::Shader( osg::Shader::VERTEX );
   osg::Shader* brickFragmentObject = 
      new osg::Shader( osg::Shader::FRAGMENT );
   brickProgramObject->addShader( brickFragmentObject );
   brickProgramObject->addShader( brickVertexObject );
   loadShaderSource( brickVertexObject, "shaders/bw.vert" );
   loadShaderSource( brickFragmentObject, "shaders/bw.frag" );

   stateSet_->setAttributeAndModes(brickProgramObject, osg::StateAttribute::ON);



}


bool SmilRegionImage::loadShaderSource (osg::Shader* obj, const std::string& fileName ) {

       std::string fqFileName = osgDB::findDataFile(fileName);
       if( fqFileName.length() == 0 )
       {
          std::cout << "File \"" << fileName << "\" not found." << std::endl;
          return false;
       }
       bool success = obj->loadShaderSourceFromFile( fqFileName.c_str());
       if ( !success  )
       {
          std::cout << "Couldn't load file: " << fileName << std::endl;
          return false;
       }
       else
       {
          return true;
       }


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
    
    if(panZoomKeys->size() != 0) {

        panZoomSampler_->getValueAt(time, vec);

        // update texture matrix
        osg::Matrix m;
        m.postMultScale(osg::Vec3d(vec[2],vec[3],0.0));
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

    size_t www;
    osg::Image* img;
    osg::ref_ptr<osg::ImageStream> mImageStream;

    osg::notify(osg::WARN) << "loading image:" << filename << std::endl;

    www = filename.find("http://");
    if (www != std::string::npos) {
    #ifdef QTWEBKIT
        osg::ref_ptr<osgQt::QWebViewImage> img = new osgQt::QWebViewImage;
        img->navigateTo(filename);
        osg::ref_ptr<osgViewer::InteractiveImageHandler> handler = new osgViewer::InteractiveImageHandler(img.get());
        HE_Geometry->setCullCallback(handler.get());
        tex_->setResizeNonPowerOfTwoHint(false);
        tex_->setImage(img);

    #endif
    } else  {

        osg::Image* img = osgDB::readImageFile(filename);
        mImageStream = dynamic_cast<osg::ImageStream*>(img);

        // test if image is stream image
        if (mImageStream.valid()) {
            std::cout << "Got movie" << std::endl;
            tex_->setInternalFormat(GL_RGB);
            tex_->setResizeNonPowerOfTwoHint(false);
            mImageStream->play();
        } else {
            std::cout << "No movie!" << std::endl;
        }

        // texture
        if(!img->valid()) {
            osg::notify(osg::WARN) << "in Image::loadImage(), failed to load image:" << filename << std::endl;
            return;
        } else {

                float s = img->s();
                float t = img->t();

                std::cout << "image size: " << s << " x " << t << std::endl; 
                tex_->setTextureSize(s,t);
                tex_->setImage(img);
                resize(s, t);
        }
    }

    //image_->ensureValidSizeForTexturing(1024);

}



void SmilRegionImage::setFit(const std::string& fit) {

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

      if( imageRatio > frameRatio) {

    float scale = 1/((imageRatio * regionSizePixels.y())/regionSizePixels.x());
        texMatInit_.makeScale(osg::Vec3d(scale,1,0.0));
        texMatInit_.postMultTranslate(osg::Vec3d(0.5 - scale/2, 0.0, 0.0));
        texMat_->setMatrix(texMatInit_);

      } else {

    
        float scale = 1/(regionSizePixels.x() * (1/imageRatio/regionSizePixels.y()) );
        texMatInit_.makeScale(osg::Vec3d(1.0, scale, 0.0));
        texMatInit_.postMultTranslate(osg::Vec3d(0.0, 0.5-scale/2, 0.0));
        texMat_->setMatrix(texMatInit_);

      }
    } else if (fillMode_ == "fill") { // do nothing, distorts image

    }
}


float SmilRegionImage::getTopOffset(float x) {

    if(topRegPoint_ == Top)
        return x;
    else if (topRegPoint_ == Center)
        return x/2;
    else return 0;
}




