# idapresentation

IDA-Presentation

IDA-Presentation is an experimental 3D-presentation tool currently for images, texts and 3D-models. It can use IDA-framework as an information source.
Introduction

IDA-Presentation is presentation tool based on OpenSceneGraph?. Mainly its purpose is to work with material coming from IDA-framework but it can be used also independently.
Details

    SMIL-like syntax in its presentations
    Supports (currently) texts, images and 3D-models 

Status

Prototype :) 

#summary Installation of IDA-Presentation

= Decencies =

  * curl 
  * openscenegraph 
  * boost-iostreams
  * tinyxml


On Debian-based distros this should install all depecies: 

`apt-get install libcurl4-openssl-dev libopenscenegraph-dev libboost-iostreams-dev libtinyxml-dev`


= Compiling =

There is a makefile in the idapresentation directory:

`cd idapresentation`

`make`

= Running demo =

A simple demo is located in examples directory:

`./viewer --file examples/demo.smil`
