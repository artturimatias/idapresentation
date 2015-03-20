# Decencies #

  * curl
  * openscenegraph
  * boost-iostreams
  * tinyxml


On Debian-based distros this should install all depecies:

`apt-get install libcurl4-openssl-dev libopenscenegraph-dev libboost-iostreams-dev libtinyxml-dev`


# Compiling #

There is a makefile in the idapresentation directory:

`cd idapresentation`

`make`

# Running demo #

A simple demo is located in examples directory:

`./viewer --file examples/demo.smil`