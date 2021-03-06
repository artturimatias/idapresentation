#include "smilRegion3D.h"


class SmilCallback: public osg::NodeCallback {
   public:
    SmilCallback() {

        posSampler_ = new osgAnimation::Vec3LinearSampler;
        camTargetPosSampler_ = new osgAnimation::Vec3LinearSampler;

        startTime_ = 0.0;
        playing_ = true;
}


    osgAnimation::Vec3KeyframeContainer* getPosKeys ()          { return posSampler_->getOrCreateKeyframeContainer(); }
    osgAnimation::Vec3KeyframeContainer* getRotKeys ()          { return camTargetPosSampler_->getOrCreateKeyframeContainer(); }

    void setCamPos (osg::Vec3 pos)          {    camPos_ = pos; }
    void setCamTargetPos (osg::Vec3 pos)    {    camTargetPos_ = pos; }

    void clear() {

        osgAnimation::Vec3KeyframeContainer* camPosKeys = posSampler_->getOrCreateKeyframeContainer();
        osgAnimation::Vec3KeyframeContainer* camTargetPosKeys = camTargetPosSampler_->getOrCreateKeyframeContainer();
        camPosKeys->clear();
        camTargetPosKeys->clear();
    }

   virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
   {

        if(playing_ && nv->getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR) {
        currentTime_ = nv->getFrameStamp()->getSimulationTime();
        if(startTime_ == 0.0)
            startTime_ = currentTime_;

        osgAnimation::Vec3KeyframeContainer* posKeys = posSampler_->getOrCreateKeyframeContainer();
        //osgAnimation::Vec3KeyframeContainer* camTargetPosKeys = camTargetPosSampler_->getOrCreateKeyframeContainer();
        // return if we do not have keyframes
        if(posKeys->size() > 1 ) {

            osg::Vec3 val;
            osg::Vec3 valTarget;

            // get position
            float t = currentTime_ - startTime_;
            posSampler_->getValueAt(t, val);

            osg::ref_ptr<osg::PositionAttitudeTransform> trans = dynamic_cast<osg::PositionAttitudeTransform*> (node );

            if(trans) {
                trans->setPosition(val);
            } 
        }

      traverse(node, nv);
        }
   }

    private:
        bool _first, playing_;
        double startTime_;
        double currentTime_;
        osg::Vec3 camTargetPos_;
        osg::Vec3 camPos_;

        osg::ref_ptr<osgAnimation::Vec3LinearSampler> posSampler_;
        osg::ref_ptr<osgAnimation::Vec3LinearSampler> camTargetPosSampler_;
};






SmilRegion3D::SmilRegion3D(int left, int top, int w, int h, int z, std::string id)
  :SmilRegion(left, top, w, h , z, id)
{
    modelTransform_ = new osg::MatrixTransform;


    tagList["setTransform"] =  SetTransform;
    tagList["animate3D"] =  Animate3D;
    tagList["animate3DCamera"] =  Animate3DCamera;

    setupCamera();
}

void SmilRegion3D::setupCamera() {

    camera = new osg::Camera;
   // osg::ref_ptr<osg::Texture2D> texture;
    unsigned int tex_width = 1024;
    unsigned int tex_height = 1024;
    unsigned int samples = 0;
    unsigned int colorSamples = 0;
    
    // color texture
    tex_ = new osg::Texture2D;
    tex_->setTextureSize(tex_width, tex_height);
    tex_->setInternalFormat(GL_RGBA);
    tex_->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
    tex_->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
    
    stateSet_->setTextureAttributeAndModes(0, tex_, osg::StateAttribute::ON);
    stateSet_->setAttribute(new osg::Material, true);


    HE_Geometry->setColorBinding (osg::Geometry::BIND_OFF);

    // set up the background color and clear mask.
    camera->setClearColor(osg::Vec4(0.0f,0.0f,0.0f,0.0f));
    camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set viewport
    camera->setViewport(0,0,tex_width,tex_height);

    // set the camera to render before the main camera.
    camera->setRenderOrder(osg::Camera::PRE_RENDER);
    
    osg::StateSet* states = camera->getOrCreateStateSet();
    states->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

    osg::Camera::RenderTargetImplementation renderImplementation = osg::Camera::FRAME_BUFFER_OBJECT;
    // tell the camera to use OpenGL frame buffer object where supported.
    camera->setRenderTargetImplementation(renderImplementation);

   bool useImage;
   useImage = false;

    if (useImage)
    {
        osg::Image* image = new osg::Image;
        //image->allocateImage(tex_width, tex_height, 32, GL_RGBA, GL_UNSIGNED_BYTE);
        image->allocateImage(tex_width, tex_height, 1, GL_RGBA, GL_FLOAT);

        // attach the image so its copied on each frame.
        camera->attach(osg::Camera::COLOR_BUFFER, image,
                       samples, colorSamples);
        
        //camera->setPostDrawCallback(new MycameraPostDrawCallback(image));
        
        // Rather than attach the texture directly to illustrate the texture's ability to
        // detect an image update and to subload the image onto the texture.  You needn't
        // do this when using an Image for copying to, as a separate camera->attach(..)
        // would suffice as well, but we'll do it the long way round here just for demonstration
        // purposes (long way round meaning we'll need to copy image to main memory, then
        // copy it back to the graphics card to the texture in one frame).
        // The long way round allows us to manually modify the copied image via the callback
        // and then let this modified image by reloaded back.
        tex_->setImage(0, image);
        
    }
    else
    {
        // attach the texture and use it as the color buffer.
        //camera->attach(osg::Camera::COLOR_BUFFER, tex_, 
          //             0, 0, false,
            //           samples, colorSamples);
        camera->attach(osg::Camera::COLOR_BUFFER, tex_);
        //camera->attach(osg::Camera::DEPTH_BUFFER, textureD);

    }


    camera->addChild(modelTransform_);
    camera->setUpdateCallback(new CameraUpdateCallback());
}

osg::Camera* SmilRegion3D::getCamera () {

    return camera;

}

void SmilRegion3D::parse(const TiXmlNode* xmlNode, const double time) {

    BaseRegion::parse(xmlNode, time);
    osgAnimation::FloatKeyframeContainer* timingKeys        = timingSampler_->getOrCreateKeyframeContainer();

    const char* fileName = xmlNode->ToElement()->Attribute("src"); 

    if(fileName) {

        float mediaDur = convertToFloat(xmlNode->ToElement()->Attribute("dur"));
        float mediaBegin = convertToFloat(xmlNode->ToElement()->Attribute("begin"));

        osg::notify(osg::WARN) <<"adding model:" << fileName << " at keyframe time: " << time + mediaBegin << " id: " << mediaItems_.size() << std::endl;

        mediaItems_.push_back(fileName);
        xmlNodes_.push_back(xmlNode);

        // set timing keyframe
        if(mediaItems_.size() == 1 && time > 0)
            timingKeys->push_back(osgAnimation::FloatKeyframe(time + mediaBegin, 0));

        timingKeys->push_back(osgAnimation::FloatKeyframe(time + mediaBegin + mediaDur, (float)mediaItems_.size()));
    }
}


void SmilRegion3D::parse3D(const TiXmlNode* xmlNode) {

    const TiXmlNode* node;

    parse3DCamera(xmlNode);

    for ( node = xmlNode->FirstChild("animate3D"); node; node = node->NextSibling("animate3D")) {
        if(node) {
            const char* const sel = node->ToElement()->Attribute("select"); 
            std::string attr = convertToString(node->ToElement()->Attribute("attributeName")); 

            if(sel) {
                osg::ref_ptr<osg::Node> osgNode = findNamedNode(sel ,modelTransform_); 
                if(osgNode) {

                        SmilCallback* up;
                        up = dynamic_cast<SmilCallback*>(osgNode->getParent(0)->getUpdateCallback());
                        // insert transformation node
                        if(!up) {
                            osg::ref_ptr<osg::PositionAttitudeTransform> pos = new osg::PositionAttitudeTransform;
                            osg::Group* parent = osgNode->getParent(0);
                            parent->removeChild(osgNode);
                            pos->addChild(osgNode);
                            parent->addChild(pos);

                            osg::ref_ptr<SmilCallback> c = new SmilCallback();
                            pos->setUpdateCallback(c); 
                            pos->setDataVariance(osg::Object::DYNAMIC); 
                            up = c;
                        }

                        osgAnimation::Vec3KeyframeContainer* keys = up->getPosKeys();
                        set3dKeys(node, keys);

                }
            }
        }
    }
}



void SmilRegion3D::parse3DCamera(const TiXmlNode* xmlNode) {
        
    const TiXmlNode* node;
    osg::Vec3 fromVec, toVec;

    // clear camera animation
    osg::ref_ptr<CameraUpdateCallback> camUpdateCallback = dynamic_cast <CameraUpdateCallback*> (camera->getUpdateCallback());
    camUpdateCallback->clear();
    camUpdateCallback->setStartTime();


    // set camera
    const osg::BoundingSphere& bs = modelTransform_->getBound();
    camera->setViewMatrixAsLookAt(bs.center()-osg::Vec3(0.0f,3.0f,0.0f)*bs.radius(),bs.center(),osg::Vec3(0.0f,0.0f,1.0f));


    for ( node = xmlNode->FirstChild("animate3DCamera"); node; node = node->NextSibling("animate3DCamera")) {
        if(node) {
            std::string attrStr = node->ToElement()->Attribute("attributeName"); 
            osgAnimation::Vec3KeyframeContainer* keys = camUpdateCallback->getCamPosKeys();

            if(parseFromTo(node, fromVec, "from", "position")) {
                insertFromToKey(node, fromVec, "from", keys, 0); // time starts from 0
            }

            if(parseFromTo(node, toVec, "to", "position")) {
                insertFromToKey(node, toVec, "to", keys, 0);
            }
        }
    }
}



void  SmilRegion3D::set3dKeys  (const TiXmlNode* node, osgAnimation::Vec3KeyframeContainer* keys) {

    osg::Vec3 fromVec, toVec;
    std::string apu;
    apu = "position";


    if(parseFromTo(node, fromVec, "from", apu.c_str())) {
        insertFromToKey(node, fromVec, "from", keys, 0);
    }

    if(parseFromTo(node, toVec, "to", apu.c_str())) {
        insertFromToKey(node, toVec, "to", keys, 0);
    }
}


osg::MatrixTransform* SmilRegion3D::findAndAddTransform(const char* const nodeName) {

     osg::Node* node = findNamedNode(nodeName ,modelTransform_); 
     if(node) {
            osg::Group* parent = node->getParent(0);
            parent->removeChild(node);
            osg::ref_ptr<osg::MatrixTransform> pos = new osg::MatrixTransform();
            pos->addChild(node);
            parent->addChild(pos);
            return pos;
     }
        return 0;
}




void SmilRegion3D::update (const double time) {

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

        //std::cout << "timingval " <<  time << std::endl;
        //std::cout << "mediaitems " <<  mediaItems_.size() << std::endl;
        //std::cout << "prevVal_ " <<  prevVal_ << std::endl;
        //std::cout << "currentItem_ " <<  currentItem_ << std::endl;
    if( timingVal > currentItem_  || timingVal < prevVal_ ) {

        if(currentItem_ >= mediaItems_.size()   )
              currentItem_ = 0;
        loadFile(mediaItems_.at(currentItem_)); 
        parse3D(xmlNodes_.at(currentItem_)); // parse 3D animations

        prevVal_ = timingVal;
        currentItem_++;
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

void SmilRegion3D::loadFile( const std::string& filename ) {

    osg::notify(osg::WARN) << "loading Model:" << filename << std::endl;

    int i = modelTransform_->getNumChildren();
    if(i)
        modelTransform_->removeChildren(0,i);


    osg::ref_ptr<osg::Group> model = new osg::Group;
    model->addChild ( osgDB::readNodeFile(filename));


    const osg::BoundingSphere& bs = model->getBound();
    if (!bs.valid())
    {
        osg::notify(osg::WARN) << "loading failed!" << filename << std::endl;
    }

    // reset matrix
    modelTransform_->setMatrix(osg::Matrix::identity());

    // create a transform to spin the model.
    modelTransform_->addChild(model);
    modelTransform_->dirtyBound();    

//    float znear = 1.0f*bs.radius();
//    float zfar  = 3.0f*bs.radius();

   // float proj_top   = 0.5f*znear;
   // float proj_right = 0.5f*znear;

    //znear *= 0.9f;
    //zfar *= 1.1f;

    // set up projection.
   //camera->setProjectionMatrixAsFrustum(-proj_right,proj_right,-proj_top,proj_top,znear,zfar);

    // set view
    camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);

    camera->setProjectionMatrixAsPerspective(
       45.0,
       1.0,
       1.0,
       1000.0
    );

    // this is for regiontest (is re-set in parse3D)
    camera->setViewMatrixAsLookAt(bs.center()-osg::Vec3(0.0f,2.0f,0.0f)*bs.radius(),bs.center(),osg::Vec3(0.0f,0.0f,1.0f));

    osg::ref_ptr<CameraUpdateCallback> camUpdateCallback = dynamic_cast <CameraUpdateCallback*> (camera->getUpdateCallback());
    
    // write lookat to update callback
    camUpdateCallback->setCamPos(bs.center()-osg::Vec3(0.0f,2.0f,0.0f)*bs.radius());
    camUpdateCallback->setCamTargetPos(bs.center());

}

osg::Node* SmilRegion3D::findNamedNode(const std::string& searchName, osg::Node* currNode) {
       osg::Group* currGroup;
       osg::Node* foundNode;

       if ( !currNode)
       {
          return NULL;
       }

       // We have a valid node, check to see if this is the node we 
       // are looking for. If so, return the current node.
       if (currNode->getName() == searchName)
       {
          return currNode;
       }


       currGroup = currNode->asGroup(); // returns NULL if not a group.
       if ( currGroup ) 
       {
          for (unsigned int i = 0 ; i < currGroup->getNumChildren(); i ++)
          { 
             foundNode = findNamedNode(searchName, currGroup->getChild(i));
             if (foundNode)
                return foundNode; // found a match!
          }
          return NULL; // We have checked each child node - no match found.
       }
       else 
       {
          return NULL; // leaf node, no match 
       }


}





