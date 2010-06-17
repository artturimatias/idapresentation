//###########################################################################
//
//############################################################################

#ifndef CAMERACALLBACK_H
#define CAMERACALLBACK_H


class CameraUpdateCallback: public osg::NodeCallback {
   public:
    CameraUpdateCallback() {

        camPosSampler_ = new osgAnimation::Vec3LinearSampler;
        camTargetPosSampler_ = new osgAnimation::Vec3LinearSampler;
        osgAnimation::Vec3KeyframeContainer* camPosKeys = camPosSampler_->getOrCreateKeyframeContainer();
        camPosKeys->push_back(osgAnimation::Vec3Keyframe(-1, camPos_));

        startTime_ = osg::Timer::instance()->tick();
}

    void insert (float time, osg::Vec3 pos, const std::string& attr) {

        if(attr == "position") {

            osgAnimation::Vec3KeyframeContainer* camPosKeys = camPosSampler_->getOrCreateKeyframeContainer();
            // add zero key
            if(camPosKeys->size() == 0) {
                camPosKeys->push_back(osgAnimation::Vec3Keyframe(0, pos));
                camPosKeys->push_back(osgAnimation::Vec3Keyframe(time, pos));
            }

            float lastKeyTime = camPosKeys->at(camPosKeys->size()-1).getTime();

            camPosKeys->push_back(osgAnimation::Vec3Keyframe(lastKeyTime, pos));


        } else if (attr == "targetPosition") {

            osgAnimation::Vec3KeyframeContainer* camTargetPosKeys = camTargetPosSampler_->getOrCreateKeyframeContainer();
            camTargetPosKeys->push_back(osgAnimation::Vec3Keyframe(time, pos));
        }
    }



    void setStartTime () {

        startTime_ = osg::Timer::instance()->tick();

    }

    osgAnimation::Vec3KeyframeContainer* getCamPosKeys ()          { return camPosSampler_->getOrCreateKeyframeContainer(); }
    osgAnimation::Vec3KeyframeContainer* getCamTargetKeys ()          { return camTargetPosSampler_->getOrCreateKeyframeContainer(); }
    void setCamPos (osg::Vec3 pos)          {    camPos_ = pos; }
    void setCamTargetPos (osg::Vec3 pos)    {    camTargetPos_ = pos; }

    void clear() {

        osgAnimation::Vec3KeyframeContainer* camPosKeys = camPosSampler_->getOrCreateKeyframeContainer();
        osgAnimation::Vec3KeyframeContainer* camTargetPosKeys = camTargetPosSampler_->getOrCreateKeyframeContainer();
        camPosKeys->clear();
        camTargetPosKeys->clear();
    }

       virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
       {

            currentTime_ = osg::Timer::instance()->tick();
            osgAnimation::Vec3KeyframeContainer* camPosKeys = camPosSampler_->getOrCreateKeyframeContainer();
            osgAnimation::Vec3KeyframeContainer* camTargetPosKeys = camTargetPosSampler_->getOrCreateKeyframeContainer();

            // return if we do not have keyframes
            if(camPosKeys->size() > 1 ) {

                osg::Vec3 val;
                osg::Vec3 valTarget;

                // camera position
                float t = osg::Timer::instance()->delta_s(startTime_, currentTime_);
                float duration = camPosSampler_->getEndTime() - camPosSampler_->getStartTime();
                t = fmod(t, duration);
                t += camPosSampler_->getStartTime();
                camPosSampler_->getValueAt(t, val);

                if(camTargetPosKeys->size() != 0 ) {
                    camTargetPosSampler_->getValueAt(t, valTarget);
                } else {
                    valTarget = camTargetPos_;
                }
                osg::ref_ptr<osg::Camera> cam = dynamic_cast<osg::Camera*> (node );

                if(cam) {
                    cam->setViewMatrixAsLookAt(val, valTarget, osg::Vec3(0.0f,0.0f,1.0f));
                } 
            }

          traverse(node, nv); 
       }
    private:
        bool _first;
        osg::Timer_t startTime_;
        osg::Timer_t currentTime_;
        osg::Vec3 camTargetPos_;
        osg::Vec3 camPos_;

        osg::ref_ptr<osgAnimation::Vec3LinearSampler> camPosSampler_;
        osg::ref_ptr<osgAnimation::Vec3LinearSampler> camTargetPosSampler_;
};

#endif
