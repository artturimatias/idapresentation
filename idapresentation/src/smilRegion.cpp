#include "smilRegion.h"

SmilRegion::SmilRegion(int left, int top, int w, int h, int z, std::string id)
  :BaseRegion(left, top, w, h , z, id),
  HE_Geode(new osg::Geode()),
  HE_Geometry(new osg::Geometry()),
  HE_Vertices(new osg::Vec3Array()),
  texCoords(new osg::Vec2Array()),
  HE_Normals(new osg::Vec3Array()),
  stateSet_(new osg::StateSet())
{
    setRegionSize(w, h);

    HE_Geometry->addPrimitiveSet( new osg::DrawArrays (osg::PrimitiveSet::QUADS, // how to render?
                              0,                                          // index of first vertex
                              HE_Vertices->size()));

    // region is invisible by default
    color_->push_back(osg::Vec4(1.0f,0.3f,0.0f, 0.0f));
        
    // Set the normal in the positive z direction (torwards the user)
    // This is done to ensure that lighting falls on the front face of the hud
    // region polygon, and not the back face which we do not care about
    HE_Normals->push_back(osg::Vec3(0.0f,0.0f,1.0f));

    //Set the geometry options
    HE_Geometry->setNormalArray  (HE_Normals.get());
    HE_Geometry->setNormalBinding(osg::Geometry::BIND_OVERALL);
    HE_Geometry->addPrimitiveSet (HE_Indices.get());
    HE_Geometry->setVertexArray  (HE_Vertices.get());
    HE_Geometry->setColorArray   (color_.get());
    HE_Geometry->setColorBinding (osg::Geometry::BIND_OVERALL);

    // texture coordinates
    texCoords->push_back (osg::Vec2 (0.0, 0.0));
    texCoords->push_back (osg::Vec2 (1.0, 0.0));
    texCoords->push_back (osg::Vec2 (1.0, 1.0));
    texCoords->push_back (osg::Vec2 (0.0, 1.0));

    HE_Geometry->setTexCoordArray (0, texCoords.get());


    //Add the geometry to the geode
    HE_Geode->addDrawable(HE_Geometry.get());
    HE_Geometry->setDataVariance(osg::Object::DYNAMIC);   
    HE_Geode->setDataVariance(osg::Object::DYNAMIC);   

    HE_Geode->setStateSet(stateSet_.get());

    stateSet_->setMode(GL_BLEND, osg::StateAttribute::ON);
    stateSet_->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
    rot_->addChild(HE_Geode);

}


void SmilRegion::parse (const TiXmlNode* xmlNode, const double time) {

    BaseRegion::parse(xmlNode, time);

}



void SmilRegion::update(const double time) {

    BaseRegion::update(time);
}

void SmilRegion::setAlpha(float alpha) {

    osg::Material* material = dynamic_cast<osg::Material*>(stateSet_->getAttribute(osg::StateAttribute::MATERIAL));
    if(material) {
        material->setAlpha(osg::Material::FRONT_AND_BACK, alpha);
    }
}

void SmilRegion::setRegionSize(float x, float y){

    //Clear the vertices
    HE_Vertices->clear();
       
    float regX = displaySize.x()*(x/100);
    float regY = displaySize.y()*(y/100);

    //Lower-left vertex
    HE_Vertices->push_back( 
      osg::Vec3(
          -1*regX/2, 
          -1*regY/2, 
          0
      ) 
    );
    //Lower-right vertex
    HE_Vertices->push_back( 
      osg::Vec3(
          regX/2, 
          -1*regY/2, 
          0
      )
    );
    //Upper-right vertex
    HE_Vertices->push_back( 
      osg::Vec3(
          regX/2, 
          regY/2, 
          0
      )
    );
    //Upper-left vertex
    HE_Vertices->push_back( 
      osg::Vec3(
          -1*regX/2, 
          regY/2, 
          0
      )
    );

    HE_Geometry->dirtyDisplayList();

}

void SmilRegion::setRegionColor(float r, float g, float b, float a){

    regionColor.set(r,g,b);
    color_->clear();
    color_->push_back(osg::Vec4(regionColor, a));

    HE_Geometry->setColorArray   (color_.get());
    HE_Geometry->dirtyDisplayList();
}




