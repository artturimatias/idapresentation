#include <osgViewer/Viewer>
#include <osg/ShapeDrawable>
#include <osg/Geometry>
#include <osgDB/ReadFile>
#include <osg/Texture2D>
#include <osg/Material>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>
#include <osgAnimation/Sampler>
#include <iostream>

//#include <osgViewer/ViewerEventHandlers>
//#include <osgGA/StateSetManipulator>


class AnimationCallback : public osg::StateSet::Callback
{
public:

    AnimationCallback() 
    {
        _sampler = new osgAnimation::Vec4LinearSampler;
        _playing = false;
        _lastUpdate = 0;
    }
    /** Callback method called by the NodeVisitor when visiting a node.*/
    virtual void operator()(osg::StateSet* state, osg::NodeVisitor* nv)
    { 
        if (state && 
            nv->getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR && 
            nv->getFrameStamp() && 
            nv->getFrameStamp()->getFrameNumber() != _lastUpdate) 
        {
            _lastUpdate = nv->getFrameStamp()->getFrameNumber();
            _currentTime = osg::Timer::instance()->tick();

            if (_playing && _sampler.get() && _sampler->getKeyframeContainer()) 
            {
                osg::Material* material = dynamic_cast<osg::Material*>(state->getAttribute(osg::StateAttribute::MATERIAL));
                if (material) 
                {
                    osg::Vec4 result;
                    float t = osg::Timer::instance()->delta_s(_startTime, _currentTime);
                    float duration = _sampler->getEndTime() - _sampler->getStartTime();
                    t = fmod(t, duration);
                    t += _sampler->getStartTime();
                    _sampler->getValueAt(t, result);
                    //material->setDiffuse(osg::Material::FRONT_AND_BACK, result);
                    material->setAlpha(osg::Material::FRONT_AND_BACK, result.w());
                    //std::cout<<"duration "<< duration << "current" << t <<std::endl;
                }
            }
        }
    }

    void start() { _startTime = osg::Timer::instance()->tick(); _currentTime = _startTime; _playing = true;}
    void stop() { _currentTime = _startTime; _playing = false;}

    osg::ref_ptr<osgAnimation::Vec4LinearSampler> _sampler;
    osg::Timer_t _startTime;
    osg::Timer_t _currentTime;
    bool _playing;
    int _lastUpdate;
};





int main()
{
	//Creating the viewer	
    	osgViewer::Viewer viewer ;

	//Creating the root node
	osg::ref_ptr<osg::Group> root (new osg::Group);
	
	//The geode containing our shpae
   	osg::ref_ptr<osg::Geode> geode (new osg::Geode);
   	osg::ref_ptr<osg::Geode> geode2 (new osg::Geode);
    
    // geometry holder
    osg::ref_ptr<osg::Geometry> plane (new osg::Geometry);
    osg::ref_ptr<osg::Geometry> plane2 (new osg::Geometry);

    //geode->setUpdateCallback( new UpdateCallback );

    AnimationCallback* callback = new AnimationCallback;
    osgAnimation::Vec4KeyframeContainer* keys = callback->_sampler->getOrCreateKeyframeContainer();
    keys->push_back(osgAnimation::Vec4Keyframe(0, osg::Vec4(0,0,0,0)));
    keys->push_back(osgAnimation::Vec4Keyframe(4, osg::Vec4(0,0,0,1.0)));
    //std::cout << keys[0] << std::endl;
    //keys->push_back(osgAnimation::Vec4Keyframe(20, osg::Vec4(0.0,0,0,1.0)));
    //keys->push_back(osgAnimation::Keyframe(0, 0.0));
    //keys->push_back(osgAnimation::Vec4Keyframe(0, osg::Vec4(0,0,0,0)));
    callback->start();


    // vertex array
    osg::ref_ptr<osg::Vec3Array> verts (new osg::Vec3Array);
    plane->setVertexArray(verts.get());
    plane2->setVertexArray(verts.get());

    verts->push_back( osg::Vec3(0, 0, 0) ); // front left
    verts->push_back( osg::Vec3(100, 0, 0) ); // front right
    verts->push_back( osg::Vec3(100,100, 0) ); // back right 
    verts->push_back( osg::Vec3( 0,100, 0) ); // back left 

       // normal
    osg::ref_ptr<osg::Vec3Array> n (new osg::Vec3Array);
    plane->setNormalArray(n.get());
    plane2->setNormalArray(n.get());
    plane->setNormalBinding( osg::Geometry::BIND_OVERALL );
    plane2->setNormalBinding( osg::Geometry::BIND_OVERALL );
    n->push_back( osg::Vec3( 0.f, 0.f, 1.f ));


    // transform

    // color
 /*
    osg::ref_ptr<osg::Vec4Array> colors (new osg::Vec4Array);
    plane->setColorArray( colors.get() );
    plane->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
    colors->push_back(osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f) ); //index 0 red
    colors->push_back(osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f) ); //index 0 red
    colors->push_back(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f) ); //index 0 red
    colors->push_back(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f) ); //index 0 red
   
    // pointer to colors array
    osg::Vec2Array* verts_tmp; 
    verts_tmp =  (osg::Vec2Array *) plane->getColorArray();
    osg::Array koe = verts_tmp[0];
*/
    //std::cout<<"color size "<< verts_tmp->size()  <<std::endl;

    // texture coordinates
    osg::ref_ptr<osg::Vec2Array> texCoords (new osg::Vec2Array());

    texCoords->push_back (osg::Vec2 (0.0, 0.0));
    texCoords->push_back (osg::Vec2 (0.0, 1.0));
    texCoords->push_back (osg::Vec2 (1.0, 1.0));
    texCoords->push_back (osg::Vec2 (1.0, 0.0));

    plane->setTexCoordArray (0, texCoords.get());

    // texture
    osg::ref_ptr<osg::Image> image = osgDB::readImageFile( "koe.jpg" );
    // Create state
    osg::StateSet* state = geode->getOrCreateStateSet();
    osg::StateSet* state2 = geode2->getOrCreateStateSet();

    // create 2D texture
    osg::ref_ptr<osg::Texture2D> tex (new osg::Texture2D);
    tex->setImage(image.get());

    // put texture 0 ON
    state->setAttribute(new osg::Material, true);
    state->setTextureAttributeAndModes(0,tex.get(),osg::StateAttribute::ON);
    // disable lighting
    state->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    state2->setMode( GL_LIGHTING, osg::StateAttribute::ON );
    // enable blending
    state->setMode( GL_BLEND, osg::StateAttribute::ON );

    state->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );

    state->setUpdateCallback(callback);


  //  plane->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 4));
    plane->addPrimitiveSet( new osg::DrawArrays (osg::PrimitiveSet::QUADS, // how to render?
                              0,                                          // index of first vertex
                              verts->size()));                           // how many vertices?
    plane2->addPrimitiveSet( new osg::DrawArrays (osg::PrimitiveSet::QUADS, // how to render?
                              0,                                          // index of first vertex
                              verts->size()));                           // how many vertices?


	geode->addDrawable(plane.get());
	geode2->addDrawable(plane2.get());


    // transform
    osg::ref_ptr<osg::PositionAttitudeTransform> siirto (new osg::PositionAttitudeTransform );
    osg::Vec3 pos(0,0,.1);
    siirto->setPosition( pos );

	root->addChild(geode.get());
	root->addChild(siirto.get());
	siirto->addChild(geode2.get());

    viewer.getCamera()->setClearColor(osg::Vec4(0,0,0,1));
	viewer.setSceneData( root.get() ); 
    viewer.getCamera()->setViewMatrixAsLookAt(osg::Vec3d(200.0, 0.0, 10.0), osg::Vec3d(0.0, 0.0, -25.0), osg::Vec3d(0.0, 1.0, 0.0));
	return (viewer.run());

	}
