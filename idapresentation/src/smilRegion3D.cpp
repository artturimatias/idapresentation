#include "smilRegion3D.h"


SmilRegion3D::SmilRegion3D(int left, int top, int w, int h, int z, std::string id)
  :SmilRegion(left, top, w, h , z, id)
{
    modelTransform_ = new osg::MatrixTransform;
    manager = new osgAnimation::BasicAnimationManager();
    modelTransform_->setUpdateCallback(manager);


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

    //states->setMode(GL_BLEND, osg::StateAttribute::ON);
    //states->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );


 //   camera->setClearColor(osg::Vec4(0.0, 0.1804, 0.1, 0.0));

    osg::Camera::RenderTargetImplementation renderImplementation = osg::Camera::FRAME_BUFFER;
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
            camera->attach(osg::Camera::COLOR_BUFFER, tex_, 
                           0, 0, false,
                           samples, colorSamples);
        }


        camera->addChild(modelTransform_);
        camera->setUpdateCallback(new CameraUpdateCallback());
    }

osg::Camera* SmilRegion3D::getCamera () {

    return camera;

}

/*
// set flow's texture to the geode
void SmilRegion3D::setRegion(osg::ref_ptr<SmilRegion> region) {

    region->getSurface()->setStateSet(stateSet_.get());
    regionName_ = region->getId(); 
    surfaces_.push_back(region);
}
*/

void SmilRegion3D::parse(const TiXmlNode* xmlNode, const double time) {

    osgAnimation::FloatKeyframeContainer* timingKeys        = timingSampler_->getOrCreateKeyframeContainer();

    const char* fileName = xmlNode->ToElement()->Attribute("src"); 

    if(fileName) {

        float dur = convertToFloat(xmlNode->ToElement()->Attribute("dur"));
        float mediaBegin = convertToFloat(xmlNode->ToElement()->Attribute("begin"));

        osg::notify(osg::WARN) <<"adding model:" << fileName << " at keyframe time: " << time + mediaBegin << " id: " << mediaItems_.size() << std::endl;

        mediaItems_.push_back(fileName);
        xmlNodes_.push_back(xmlNode);

        // set timing keyframe
        if(mediaItems_.size() == 1 && time > 0)
            timingKeys->push_back(osgAnimation::FloatKeyframe(time + mediaBegin, 0));

        timingKeys->push_back(osgAnimation::FloatKeyframe(time + mediaBegin + dur, (float)mediaItems_.size()));

        // set fadein/out
        setFadeIn(time + mediaBegin, 1.0, 1.0);
        setFadeOut(time + mediaBegin + dur, 1.0, 1.0);



    }

}


void SmilRegion3D::parse3D(const TiXmlNode* xmlNode, const double time) {

    char delimiter = ',';
    const TiXmlNode* node;

    MapVec3 positionChannels;
    MapVec3::iterator ic;
    MapVec3 rotationChannels;
    MapVec3::iterator ir;

    // clear camera animation
    osg::ref_ptr<CameraUpdateCallback> camUpdateCallback = dynamic_cast <CameraUpdateCallback*> (camera->getUpdateCallback());
    camUpdateCallback->clear();
    camUpdateCallback->setStartTime();


    for ( node = xmlNode->FirstChild(); node; node = node->NextSibling("animate3D")) {
        if(node) {
            const char* const sel = node->ToElement()->Attribute("select"); 
            std::string attr = convertToString(node->ToElement()->Attribute("attributeName")); 

            if(sel) {
                osg::ref_ptr<osg::Node> osgNode = findNamedNode(sel ,modelTransform_); 
                if(osgNode) {
                    // insert transformation node
                    ic = positionChannels.find(sel);
                    ir = rotationChannels.find(sel);
                    if(attr == "position" && ic != positionChannels.end()) {

                        set3dKeys(node, ic->second, time);

                    } else if(attr == "rotation" && ir != rotationChannels.end()) {

                        set3dKeys(node, ir->second, time);
                   
                    } else {
                        osg::ref_ptr<osg::MatrixTransform> pos = new osg::MatrixTransform;
                        osg::Group* parent = osgNode->getParent(0);
                        parent->removeChild(osgNode);
                        pos->addChild(osgNode);
                        parent->addChild(pos);
                        osgAnimation::UpdateTransform* up = dynamic_cast<osgAnimation::UpdateTransform*> (osgNode->getParent(0)->getUpdateCallback());
                        if(!up) {
                            pos->setUpdateCallback(new osgAnimation::UpdateTransform(sel)); 
                        }


                        osg::ref_ptr<osgAnimation::Vec3LinearChannel> channel = new osgAnimation::Vec3LinearChannel;
                        channel->setTargetName(sel);
                        if(attr == "position") {
                            channel->setName("position");
                            positionChannels.insert(std::pair<const char*, osgAnimation::Vec3LinearChannel*>(sel,channel));
                        } else {
                            channel->setName("euler");
                            rotationChannels.insert(std::pair<const char*, osgAnimation::Vec3LinearChannel*>(sel,channel));
                        }
                        set3dKeys(node, channel, time);

                    }


                }
            }
        }
    }

        osgAnimation::Animation* anim1 = new osgAnimation::Animation;
        anim1->setPlaymode(osgAnimation::Animation::ONCE); 
        manager->registerAnimation(anim1);
    if(positionChannels.size() > 0) {
        // add 3D animation channels to animation
        std::cout << positionChannels.size() << std::endl;
        for(ic = positionChannels.begin(); ic != positionChannels.end(); ic++) {
            anim1->addChannel(ic->second);
        }

    }

    if(rotationChannels.size() > 0) {
        // add 3D animation channels to animation
        std::cout << rotationChannels.size() << std::endl;
        for(ir = rotationChannels.begin(); ir != rotationChannels.end(); ir++) {
            anim1->addChannel(ir->second);
        }

    }

        manager->playAnimation(anim1);
        
    // set camera
    const osg::BoundingSphere& bs = modelTransform_->getBound();
    camera->setViewMatrixAsLookAt(bs.center()-osg::Vec3(0.0f,2.0f,0.0f)*bs.radius(),bs.center(),osg::Vec3(0.0f,0.0f,1.0f));

}

/*


void SmilRegion3D::parse3D(const TiXmlNode* xmlNode, const double time) {

    char delimiter = ',';
    const TiXmlNode* node;

    std::map <std::string, osg::Vec3> positionValues;

    // clear camera animation
    osg::ref_ptr<CameraUpdateCallback> camUpdateCallback = dynamic_cast <CameraUpdateCallback*> (camera->getUpdateCallback());
    camUpdateCallback->clear();
    camUpdateCallback->setStartTime();


    for ( node = xmlNode->FirstChild(); node; node = node->NextSibling()) {
        if(node) {
            switch (tagList[node->Value()]) {

            case SetTransform:
            {
                osg::ref_ptr<osg::MatrixTransform> pos = new osg::MatrixTransform;
                const char* const sel = node->ToElement()->Attribute("select"); 
                const std::string attrName = convertToString(node->ToElement()->Attribute("attributeName")); 
                osg::Vec3 posVec; 
                osg::Vec4 rotVec; 
                osg::Matrix m ;

                if(attrName == "position") {
                    const char* posStr = node->ToElement()->Attribute("position"); 
                    if(posStr) {
                        std::vector<std::string> vec = split2(posStr,delimiter);
                        posVec.set(convertToFloat(vec[0]), convertToFloat(vec[1]),convertToFloat(vec[2]));
                        m.makeTranslate(posVec);
                    }
                } else if(attrName == "rotation") {
                    const char* rotStr = node->ToElement()->Attribute("rotation"); 
                    if(rotStr) {
                        std::vector<std::string> vec = split2(rotStr,delimiter);
                        rotVec.set(osg::DegreesToRadians(convertToFloat(vec[0])), convertToFloat(vec[1]),convertToFloat(vec[2]), convertToFloat(vec[3]));
                        m.makeRotate(rotVec);
                    }

                }

                if(sel) {
                    osg::ref_ptr<osg::Node> osgNode = findNamedNode(sel ,modelTransform_); 
                    if(osgNode) {
                        osg::Group* parent = osgNode->getParent(0);
                        parent->removeChild(osgNode);
                        pos->addChild(osgNode);
                        parent->addChild(pos);
                        pos->setMatrix(m);
                    }
                } else {
                        modelTransform_->setMatrix(m);
                }
            }
            break;

            case Animate3D: 
            {
                const char* const sel = node->ToElement()->Attribute("select"); 
                osg::ref_ptr<osg::MatrixTransform> pos = new osg::MatrixTransform;

                if(sel) {
                    osg::ref_ptr<osg::Node> osgNode = findNamedNode(sel ,modelTransform_); 
                    if(osgNode) {

                        // get animation list
                        osgAnimation::AnimationList a = manager->getAnimationList();
                        osgAnimation::AnimationList::iterator it;
                        osgAnimation::ChannelList::iterator itc;

                        // search for animation for selection
                        it = std::find_if( a.begin(), a.end(), boost::bind( &osgAnimation::Animation::getName, _1 ) == sel );
                        if(it!=a.end()) {
                            std::cout << "Found animation for " << (*it)->getName() << std::endl;
                            osgAnimation::ChannelList channels = (*it)->getChannels();

                            // search position channel
                            itc = std::find_if( channels.begin(), channels.end(), boost::bind( &osgAnimation::Channel::getName, _1 ) == "position" );
                            if(itc != channels.end()) {
                                std::cout << "Found channel " << (*itc)->getName() << std::endl;
                                osgAnimation::Vec3LinearChannel* chPosition;
                                chPosition = dynamic_cast<osgAnimation::Vec3LinearChannel*>( (&*itc)->get() );

                              //  chPosition->getOrCreateSampler()->getOrCreateKeyframeContainer()->push_back(osgAnimation::Vec3Keyframe(12.0, osg::Vec3(20,20,0)));
                                setPositionKeys(node, chPosition, time);
                            } 

                        // animation not found, initialise.
                        } else {

                            std::cout << "Animation not found for "  <<  sel  << std::endl;
                            osgAnimation::UpdateTransform* up = dynamic_cast<osgAnimation::UpdateTransform*> (osgNode->getParent(0)->getUpdateCallback());
                            if(!up) {
                                pos->setUpdateCallback(new osgAnimation::UpdateTransform(sel)); 
                            }

                            osgAnimation::Animation* anim1 = new osgAnimation::Animation;

                            osg::ref_ptr<osgAnimation::Vec3LinearChannel> chPosition = new osgAnimation::Vec3LinearChannel;
                            setPositionKeys(node, chPosition, time);

                            chPosition->setTargetName(sel);
                            chPosition->setName("position");
                            anim1->addChannel(chPosition);
                            anim1->setName(sel);
                            anim1->setPlaymode(osgAnimation::Animation::ONCE); 
                            manager->registerAnimation(anim1);
                          //  manager->playAnimation(anim1);

                            // insert transformation node
                            osg::Group* parent = osgNode->getParent(0);
                            parent->removeChild(osgNode);
                            pos->addChild(osgNode);
                            parent->addChild(pos);



                        }


                    }
                } else {

                }


            }
             break;

            case Animate3DCamera: {

                        const char* const fromStr = node->ToElement()->Attribute("from"); 
                        const char* const toStr = node->ToElement()->Attribute("to"); 
                        const char* const durStr = node->ToElement()->Attribute("dur"); 
                        const char* const beginStr = node->ToElement()->Attribute("begin"); 
                        std::string attrStr = node->ToElement()->Attribute("attributeName"); 
                        float dur = 0.0f;
                        float begin = 0.0f;

                        if(durStr)
                            dur = convertToFloat(durStr);

                        if(beginStr) {
                            begin = convertToFloat(beginStr);
                            dur = begin + dur;
                        }

                        if(fromStr && toStr) {
                            
                            osg::Vec3 fromVec; 
                            osg::Vec3 toVec; 

                            std::vector<std::string> fromList = split2(fromStr,delimiter);
                            std::vector<std::string> toList = split2(toStr,delimiter);

                            fromVec.set(convertToFloat(fromList[0]), convertToFloat(fromList[1]),convertToFloat(fromList[2]));
                            toVec.set(convertToFloat(toList[0]), convertToFloat(toList[1]),convertToFloat(toList[2]));
                        


                            if(camUpdateCallback) {
                                camUpdateCallback->insert(begin, fromVec, attrStr);
                                camUpdateCallback->insert(dur, toVec, attrStr);
                            }
                        }
            }
            break;
            default:
                break;
            }
        }
    }

    // set camera
    const osg::BoundingSphere& bs = modelTransform_->getBound();
    camera->setViewMatrixAsLookAt(bs.center()-osg::Vec3(0.0f,2.0f,0.0f)*bs.radius(),bs.center(),osg::Vec3(0.0f,0.0f,1.0f));
                        osgAnimation::AnimationList a = manager->getAnimationList();
                        osgAnimation::AnimationList::iterator il;
                        il = a.begin();
                            manager->playAnimation(*il);

}
*/


void  SmilRegion3D::set3dKeys  (const TiXmlNode* node, osgAnimation::Vec3LinearChannel* channel, const double time) {

    char delimiter = ',';
    const char* const fromStr = node->ToElement()->Attribute("from"); 
    const char* const toStr = node->ToElement()->Attribute("to"); 
    float dur = convertToFloat(node->ToElement()->Attribute("dur"));
    float begin = convertToFloat(node->ToElement()->Attribute("begin"));

    if(fromStr && toStr) {
        
        osg::Vec3 fromVec; 
        osg::Vec3 toVec; 

        std::vector<std::string> fromList = split2(fromStr,delimiter);
        std::vector<std::string> toList = split2(toStr,delimiter);

        if(channel->getName() == "euler") {
            fromVec.set(
                    osg::DegreesToRadians(convertToFloat(fromList[0])),
                    osg::DegreesToRadians(convertToFloat(fromList[1])),
                    osg::DegreesToRadians(convertToFloat(fromList[2])));

            toVec.set(
                    osg::DegreesToRadians(convertToFloat(toList[0])),
                    osg::DegreesToRadians(convertToFloat(toList[1])),
                    osg::DegreesToRadians(convertToFloat(toList[2])));

        } else {

            fromVec.set(    
                        convertToFloat(fromList[0]), 
                        convertToFloat(fromList[1]),
                        convertToFloat(fromList[2]));
            toVec.set(
                        convertToFloat(toList[0]), 
                        convertToFloat(toList[1]),
                        convertToFloat(toList[2]));
        }

        osgAnimation::Vec3KeyframeContainer* posKeys      = channel->getOrCreateSampler()->getOrCreateKeyframeContainer();
        // add zero key
        if(posKeys->size() == 0) 
            posKeys->push_back(osgAnimation::Vec3Keyframe(0, fromVec));

        float lastKeyTime = posKeys->at(posKeys->size()-1).getTime();

        std::cout << "Setting keys at time: " << time << " lastKeyTime: " << lastKeyTime << std::endl;

        if(lastKeyTime == 0) {
            posKeys->push_back(osgAnimation::Vec3Keyframe(time + begin, fromVec));
            posKeys->push_back(osgAnimation::Vec3Keyframe(time + begin + dur, toVec));
        } else {

            posKeys->push_back(osgAnimation::Vec3Keyframe(lastKeyTime + begin , fromVec));
            posKeys->push_back(osgAnimation::Vec3Keyframe(lastKeyTime + begin + dur, toVec));
        }
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
        parse3D(xmlNodes_.at(currentItem_), time); // parse 3D animations

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

