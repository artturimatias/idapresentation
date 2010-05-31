//###########################################################################
//  File:       smilNet.h
//  Purpose:    Net traffic (CURL)
//############################################################################

#ifndef SMILNET_H
#define SMILNET_H

#include <osg/Geode>
#include <string>
#include <fstream>
#include <iostream>
#include "tinyxml.h"
#include <curl/curl.h>
#include "func.h"

typedef std::map<std::string, TiXmlNode*> rList;

class SmilNet: public osg::Referenced {
public:
    SmilNet();

    static void makeQuery       (std::string queryStr);
    static void readTmp         ( TiXmlNode* node); 
    static float makeImageQuery  (std::string recId, rList list, TiXmlNode* node); 
    static float createImageNodes (TiXmlNode* queryNode, rList l, TiXmlNode* seqNode);
    static float createImageNode (TiXmlNode* resNode, TiXmlNode* queryNode, std::string defRegion, float imageDur, rList replaceList, TiXmlNode* node);
    static void getImageMeta (TiXmlNode* seq);
    static void createNoteNode (TiXmlNode* imgNode, TiXmlNode* parNode, float imageDur, std::string imgDate, std::string imgDateExt);
private:

};


//static int writer(char *data, size_t size, size_t nmemb,  std::string *buffer) ; 

#endif
