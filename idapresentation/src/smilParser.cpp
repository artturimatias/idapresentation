#include "smilParser.h"

PresentationParser::PresentationParser(osg::Group* root, unsigned int width, unsigned int height) 
{
    tagList[""] = NotDefined;
    tagList["body"] =  Body;
    tagList["seq"] =  Seq;
    tagList["par"] =  Par;
    tagList["img"] =  Img;
    tagList["model"] =  Model;
    tagList["text"] = Text;
    tagList["query"] = Query;

    displaySize.set(width, height);

    startTime_ = 0;
    showDuration_ = 0;

    currentFile_ = 0;

    mainNode_ = root;
    camera_ = createCamera(width, height);       // camera for 2D objects
    mainNode_->addChild(camera_);

    debugText_ = new SmilRegionText(0,0,200,100,0,"debug");
    camera_->addChild(debugText_->getTransform(displaySize));
}

void PresentationParser::start() {

    startTime_ = osg::Timer::instance()->tick();
    std::map<std::string, osg::ref_ptr<BaseRegion> >::iterator i = regions_.begin();

    if(showDuration_ > 0 ) {
        for( ; i != regions_.end(); ++i )
        {
            std::cout << "Starting region flow: " << i->first << std::endl;
            i->second->start();
        }

        running_ = true;
    }
}

void PresentationParser::stop() {

    std::map<std::string, osg::ref_ptr<BaseRegion> >::iterator i = regions_.begin();

    for( ; i != regions_.end(); ++i )
    {
        std::cout << "Stopping region flow: " << i->first << std::endl;
        i->second->stop();
    }


    running_ = false;
}


void PresentationParser::next() {

    if(fileList_.size() > 0) {
        currentFile_++;
        if(currentFile_ > fileList_.size()-1){
            currentFile_ = 0;
        }
        stop();
        clear();
        load(fileList_[currentFile_]);
        start();
    }
}

void PresentationParser::load(int fileIndex) {

    if(fileList_.size() > 0) {
        if(fileIndex > -1 && fileIndex < (signed)fileList_.size()) {
            currentFile_ = fileIndex;
            stop();
            clear();
            load(fileList_[currentFile_]);
            start();
        }
    }
}



void PresentationParser::update() {

    if(!running_ || showDuration_ == 0.0) return;
     
    std::map<std::string, osg::ref_ptr<BaseRegion> >::iterator i = regions_.begin();

    currentTime_ = osg::Timer::instance()->tick();
    float t = osg::Timer::instance()->delta_s(startTime_, currentTime_);

    if(t < showDuration_) {

        for( ; i != regions_.end(); ++i )
        {
            i->second->update(t);
        }
    } else {

        std::cout << "Show ended! "  << std::endl;
        running_ = false;

    }
}


void PresentationParser::clear() {

    int i = mainNode_->getNumChildren();
    if(i)
        mainNode_->removeChildren(0,i);


    int j = camera_->getNumChildren();
    if(j)
        camera_->removeChildren(0,j);

 //   camera_ = createCamera();
    mainNode_->addChild(camera_);

    regions_.clear();
    startTime_ = 0;
    showDuration_ = 0;



}

osg::ref_ptr<osg::Camera> PresentationParser::createCamera(unsigned int width, unsigned int height) {

    // create a camera to set up the projection and model view matrices, and the subgraph to draw in the HUD
    osg::ref_ptr<osg::Camera> camera = new osg::Camera;


    // set the projection matrix
    camera->setProjectionMatrix(osg::Matrix::ortho2D(0,width,0,height));

    // set the view matrix    
    camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    camera->setViewMatrix(osg::Matrix::identity());

    // only clear the depth buffer
    camera->setClearMask(GL_DEPTH_BUFFER_BIT);

    // draw subgraph after main camera view.
    camera->setRenderOrder(osg::Camera::POST_RENDER);

    // we don't want the camera to grab event focus from the viewers main camera(s).
    camera->setAllowEventFocus(false);
    
    osg::StateSet* stateset = camera->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    stateset->setMode(GL_BLEND, osg::StateAttribute::ON);
    stateset->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );

    return camera;
}


void PresentationParser::insert2FileList(std::string fileName ) {

    fileList_[fileList_.size()] = fileName; 
}

void PresentationParser::readFileList(std::string list ) {

    int counter = 0;
    TiXmlNode* rootNode;
    std::cout << "Loading presentation list: " << list << std::endl;

    TiXmlDocument*  doc = new TiXmlDocument(list);
    if(!doc->LoadFile())
        return;

    rootNode = doc->FirstChild("presentationList");
    if(rootNode) {
        for ( TiXmlNode* listNode = rootNode->FirstChild("presentation"); listNode; listNode = listNode->NextSibling("presentation")) {
            std::cout <<  "adding presentation: " << listNode->ToElement()->Attribute("src") << std::endl;
            fileList_[counter] = listNode->ToElement()->Attribute("src"); 
            counter++;
        }
    }
   
    delete doc;
}



void PresentationParser::load( const std::string& fileName ) {

    fileName_ = fileName;
    std::cout << "Loading presentation: " << fileName << std::endl;

    TiXmlDocument*  doc = new TiXmlDocument(fileName);
    if(!doc->LoadFile())
        return;

    TiXmlNode* rootNode;
    rootNode = doc->FirstChild();

     if(rootNode->ValueStr() == "smil") {

        // parse layout
        parseLayout(rootNode);

        // parse content
        TiXmlNode* body = rootNode->FirstChild("body");
        if(body) {

            parseTimeline(body, 0.0);
        }

        doc->SaveFile("tmp/tmp.smil");

        // find out duration of the whole show
        std::map<std::string, osg::ref_ptr<BaseRegion> >::iterator i = regions_.begin();
        float dur = 0;
        for( ; i != regions_.end(); ++i )
        {
            dur = i->second->getDuration();
            if(dur > showDuration_) showDuration_ = dur;

        }

        int secs = (int)showDuration_ % 60;
        int mins = (int)showDuration_ / 60;
        showDurationStr_ = "kesto: " + convertToString(mins) +  ":" + convertToString(secs) ;
        debugText_->setText(showDurationStr_);

        std::cout << "DURATION: "  << showDurationStr_ << std::endl;
    }


}
/*
     } else if (rootNode->ValueStr() == "presentationList") {
        readFileList(fileName);
        currentFile_ = 1;
        load(fileList_[currentFile_]);
     }
*/
void PresentationParser::parseLayout (TiXmlNode* rootNode) {

    enum StringValue { NotDefined, 
                        RegionImage, 
                        Region3D,
                        RegionText};

    std::map <std::string, int> tagList;
    tagList["no"] = NotDefined;
    tagList["regionImage"] =  RegionImage;
    tagList["region3D"] =  Region3D;
    tagList["regionText"] =  RegionText;

    // make queries and insert results to presentation
    preParse(rootNode);

        TiXmlNode* head = rootNode->FirstChild("head");
        if(head) {
            TiXmlNode* layout = head->FirstChild("layout");
            if(layout) {
                TiXmlNode* rootLayout = layout->FirstChild("root-layout");
                if(rootLayout) {
                    TiXmlNode* region;
                    // pick regions
                    for ( region = rootLayout->FirstChild(); region; region = region->NextSibling()) {
                        if(region) {
                            // skip comments
                            if(region->Type() == TiXmlNode::COMMENT) 
                                continue;

                            std::string id = convertToString(region->ToElement()->Attribute("id")); 
                            float left = convertToFloat(region->ToElement()->Attribute("left")); 
                            float top = convertToFloat(region->ToElement()->Attribute("top")); 
                            float width = convertToFloat(region->ToElement()->Attribute("width")); 
                            float height = convertToFloat(region->ToElement()->Attribute("height")); 
                            int z = convertToInt(region->ToElement()->Attribute("z")); 
                            std::string fit = convertToString(region->ToElement()->Attribute("fit")); 
                            std::string src = convertToString(region->ToElement()->Attribute("src")); 
                           // const char* fit = region->ToElement()->Attribute("fit"); 

                            if(!id.empty()) {
                                std::cout << "Creating region:" << id << std::endl;
                                switch (tagList[region->Value()]) {

                                case RegionImage: {
                                    osg::ref_ptr<SmilRegionImage>  reg = new SmilRegionImage(left,top,width,height,z,id);
                                    reg->setFit(fit);
                                   // reg->setFadeInVal(fadeIn);
                                   // reg->setFadeOutVal(fadeOut);
                                    camera_->addChild(reg->getTransform(displaySize));
                                    if(!src.empty()) {
                                        reg->loadFile(src);
                                        reg->setAlpha(1.0f);
                                    }

                                    regions_[id] = reg;
                                    }
                                break;

                                case Region3D: {
                                    osg::ref_ptr<SmilRegion3D>  reg = new SmilRegion3D(left,top,width,height,z,id);
                                    camera_->addChild(reg->getTransform(displaySize));
                                    mainNode_->addChild(reg->getCamera());
                                    regions_[id] = reg;
                                    }
                                break;

                                case RegionText: {
                                    osg::ref_ptr<SmilRegionText>  reg = new SmilRegionText(left,top,width,height,z,id);
                                    camera_->addChild(reg->getTransform(displaySize));
                                    reg->setTextSize(convertToFloat(region->ToElement()->Attribute("fontSize")));
                                  //  if(!src.empty()) {
                                    std::string textVal = convertToString(region->ToElement()->GetText());
                                    reg->setText(textVal);
                                   // }
                                    regions_[id] = reg;
                                    }
                                break;


                                default:
                                    std::cout << "INVALID  region: " << id << std::endl;
                                }
                            }
                    }

                }
            }
        }
    }


}

// make queries 
void PresentationParser::preParse(TiXmlNode* node) {

    if(node) {
        for ( TiXmlNode* node_2 = node->FirstChild(); node_2; node_2 = node_2->NextSibling()) {
            if(node_2) {
                switch (tagList[node_2->Value()]) {

                case Img: {
                    if(node_2->ToElement()->Attribute("callBack")) {

                        // add a par node and move image node under it
                        TiXmlNode* imgCopy = node_2->ToElement()->Clone();
                        TiXmlElement parImg("par");
                        parImg.LinkEndChild(imgCopy);

                        // fetch image note and add it to par node
                        SmilNet::getImageMeta(imgCopy);
                        imgCopy->ToElement()->RemoveAttribute("callBack");
                        node->ReplaceChild(node_2, parImg);
                    } 
                }
                break;

                case Query: {
                    std::cout << node_2->ToElement()->Attribute("src") << std::endl;

                    SmilNet::makeQuery(convertToString(node_2->ToElement()->Attribute("src")));
                    SmilNet::readTmp(node_2);
                    node_2->SetValue("seq");
                }
                break;

                default:
                    preParse(node_2);
                break;
                }
            }
        }
    }


}

float PresentationParser::parseTimeline(TiXmlNode* node, float time) {

    float startFrameTime = time;    // inner time
    float duration = 0;             // returned value
    float t = 0;

    if(node) {
        for ( TiXmlNode* node_2 = node->FirstChild(); node_2; node_2 = node_2->NextSibling()) {
            if(node_2) {
                switch (tagList[node_2->Value()]) {

                case Seq:
                    std::cout << node->Value() << "->SEQUENCE (" << startFrameTime << ")" << std::endl;
                    // in sequence time is added
                    t =  parseTimeline(node_2, startFrameTime);
                    if(tagList[node->Value()] != Par){
                        duration += t;
                        startFrameTime += t;
                    } else {

                    //    duration += t;
                    }
                    std::cout << "sequence end duration (" << duration << ")" << std::endl;
                break;

                case Par: {

                    std::cout << "PARALLEL (" << startFrameTime << ")" << std::endl;
                    t = parseTimeline(node_2, startFrameTime);
                    // if parent node is sequence, then duration is duration + duration + duration...
                    if(tagList[node->Value()] != Par) {
                        duration += t;
                        startFrameTime += t;
                    }
                
                }

                break;

                case Img: {
                    std::cout << node->Value() <<"->IMAGE (" << time+duration << ")" << std::endl;

                    // media element must have target region and duration
                    if(node_2->ToElement()->Attribute("region")) {
                        std::string id = node_2->ToElement()->Attribute("region");
                        try {
                            regions_.at(id)->parse(node_2, startFrameTime);
                             getRightTime(node_2, node, startFrameTime, duration);
                        } catch (...) {
                            std::cout << "region " << id << " parse failed!" << std::endl; 
                        }

                    }
                }
                break;


                case Model: {
                    // TODO: check that region is 3D region!
                    std::cout << node->Value() <<"->MODEL (" << startFrameTime << ")" << std::endl;
                    // media element must have target region and duration
                    if(node_2->ToElement()->Attribute("region") && node_2->ToElement()->Attribute("dur")) {
                        std::string id = node_2->ToElement()->Attribute("region");
                        try {
                            regions_.at(id)->parse(node_2, startFrameTime);
                            getRightTime(node_2, node, startFrameTime, duration);
                        } catch (...) {
                            std::cout << "region " << id << " parse failed!" << std::endl; 
                        }
                    }
                }
                break;

                case Text: {
                    std::cout << node->Value() <<"->TEXT (" << startFrameTime << ")" << std::endl;
                    // media element must have target region and duration
                    if(node_2->ToElement()->Attribute("region") && node_2->ToElement()->Attribute("dur")) {
                        try {
                            std::string id = node_2->ToElement()->Attribute("region");
                            regions_.at(id)->parse(node_2, startFrameTime);
                            getRightTime(node_2, node, startFrameTime, duration);
                        } catch (...) {

                        }

                    }
                }
                break;

                default:
                    std::cout << "Unidentified node! (" << node_2->Value() << ")" << std::endl;
                break;
                }
            }
        }
    }


   // if (tagList[node->Value()] == Body) 
     //   return startFrameTime;
   // else
        return duration;
}


// calculates duration and timelinetime according to parent node (seq/par)
void PresentationParser::getRightTime(TiXmlNode* node, TiXmlNode* parent,  float& startFrameTime, float& duration) {

    float dur = convertToFloat(node->ToElement()->Attribute("dur"));
    float begin = convertToFloat(node->ToElement()->Attribute("begin"));

    // if parent is sequence then duration is duration + duration + duration etc.
    if(tagList[parent->Value()] == Seq) {
        startFrameTime += dur;
        duration = duration + dur; 
    }

    // if parent is "par" then duration is the biggest duration
    if(tagList[parent->Value()] == Par) {
        std::cout << "Parent is PAR" << std::endl;
        // if there is a begin value, then duration is begin + duration
        if(begin + dur > duration)
            duration = begin + dur;
    }
}

