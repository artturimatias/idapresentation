
#include "smilImageFlow.h"
class SmilImageFlow;

class ImageFlowCallback : public osg::NodeCallback
{
public:
    META_Object(osgAnimation, ImageFlowCallback);

    ImageFlowCallback() 
    {
        //_sampler = new osgAnimation::Vec3CubicBezierSampler;
        _sampler = new osgAnimation::FloatLinearSampler;
        _playing = false;
        _lastUpdate = 0;
        _prevVal = 0;
        _prevDir = 0;
    }
    ImageFlowCallback(const ImageFlowCallback& val, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY):
        osg::Object(val, copyop),
        osg::NodeCallback(val, copyop),
        _sampler(val._sampler),
        _startTime(val._startTime),
        _currentTime(val._currentTime),
        _playing(val._playing),
        _lastUpdate(val._lastUpdate)
    {
    }

    /** Callback method called by the NodeVisitor when visiting a node.*/
    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
    { 
        if (nv->getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR && 
            nv->getFrameStamp() && 
            nv->getFrameStamp()->getFrameNumber() != _lastUpdate) {

            _lastUpdate = nv->getFrameStamp()->getFrameNumber();
            _currentTime = osg::Timer::instance()->tick();

            if (_playing && _sampler.get() && _sampler->getKeyframeContainer()) {
            // get user defined class
                osg::ref_ptr<SmilImageFlow> hudEle = 
                    dynamic_cast<SmilImageFlow*> (node->getUserData() );


                if (hudEle) { 
                    float osa;
                    float t = osg::Timer::instance()->delta_s(_startTime, _currentTime);
                    float duration = _sampler->getEndTime() - _sampler->getStartTime();
                    t = fmod(t, duration);
                    t += _sampler->getStartTime();
                    _sampler->getValueAt(t, osa);

                    float dir = osa - _prevVal;
                    if(dir > 0.0f && _prevDir < 0.0f) {

                        std::cout << "suunta vaihtuu:  "   << std::endl;
                        //hudEle->nextImage();
                    }

                   // hudEle->setAlpha(osa);
                    _prevVal = osa;
                    _prevDir = dir;
                    
                //std::cout << "osa:" << _sampler->getEndTime()  << std::endl;
                }
            } 
        }
        // note, callback is responsible for scenegraph traversal so
        // they must call traverse(node,nv) to ensure that the
        // scene graph subtree (and associated callbacks) are traversed.
        traverse(node,nv);
    }

    void start() { _startTime = osg::Timer::instance()->tick(); _currentTime = _startTime; _playing = true;}
    void stop() { _currentTime = _startTime; _playing = false;}

    //osg::ref_ptr<osgAnimation::Vec3CubicBezierSampler> _sampler;
    osg::ref_ptr<osgAnimation::FloatLinearSampler> _sampler;
    osg::Timer_t _startTime;
    osg::Timer_t _currentTime;
    bool _playing;
    int _lastUpdate;
    float _prevVal;
    float _prevDir;
};


