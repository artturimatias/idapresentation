#include "smilNet.h"
static int writer(char *data, size_t size, size_t nmemb,  
                    std::string *buffer)  
  {  
    // What we will return  
    int result = 0;  
    
    // Is there anything in the buffer?  
    if (buffer != NULL)  
    {  
      // Append the data to the buffer  
      buffer->append(data, size * nmemb);  
    
      // How much did we write?  
      result = size * nmemb;  
    }  
    
    return result;  
  } 



SmilNet::SmilNet() {

}


void SmilNet::makeQuery (std::string queryStr) {

    static char errorBuffer[CURL_ERROR_SIZE];  
    static std::string buffer;  
    std::ofstream myfile;

    CURL* c;
    CURLcode result; 

    std::cout << "sending query: " << queryStr << std::endl;

    c = curl_easy_init();
    std::string query = "xml="; 
    query.append(queryStr);

    if(c) {
        curl_easy_setopt( c, CURLOPT_URL, "localhost/IDA3/idacore/server/xmlin.php" );
        curl_easy_setopt( c , CURLOPT_POSTFIELDS , query.c_str() ) ;
        curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, writer);
        curl_easy_setopt(c, CURLOPT_WRITEDATA, &buffer); 

        result = curl_easy_perform( c );
        if (result == CURLE_OK) {
            std::cout << "www ok" << std::endl;
        } else    {  
             std::cout << "Error: [" << result << "] - " << errorBuffer;  
             exit(-1);  
       }

        curl_easy_cleanup( c );

        std::ofstream file("tmp_query_result.xml");
        file << buffer;
        file.close();
        buffer = "";

    }


}

// get image data when callback is defined and write data to image node's parent node
void SmilNet::getImageMeta (TiXmlNode* imgNode) {

    std::string ending= ".jpg";
    std::string fileName = convertToString(imgNode->ToElement()->Attribute("src"));
    std::string region = convertToString(imgNode->ToElement()->Attribute("region"));
    int pos = fileName.find(ending); 
    fileName.replace(pos, ending.size(), ""); 

    std::map<std::string, TiXmlNode*> l;
    float imageDur = convertToFloat(imgNode->ToElement()->Attribute("dur"));
    if(imageDur == 0)
        imageDur = 2;


    std::string res, namet, imgDate, imgDateExt;

    // fetch image metadata
    std::string query = "<search><Creation><has_created><Digital_Image><is_identified_by><filename>";
    query.append(fileName);
    query.append("</filename></is_identified_by></Digital_Image>");
    query.append("</has_created></Creation>");
    query.append("<result has_time-span='start_year'>");
    query.append("<has_created has_note='message' />");
    query.append("</result></search>");

    SmilNet::makeQuery(query);

    // read query result and make image nodes
    TiXmlDocument* doc = new TiXmlDocument("tmp_query_result.xml");
    doc->LoadFile();

    TiXmlHandle docHandle( doc );

    TiXmlElement* resNode;
    resNode = docHandle.FirstChild("results").FirstChild("Creation").ToElement();
   if(resNode) {

        TiXmlElement* iNode = resNode->FirstChild("has_created")->ToElement();
        if(iNode) {
            TiXmlElement* iiNode = iNode->FirstChild("Digital_Image")->ToElement();
            if(iiNode) {
                imgDate = convertToString(resNode->ToElement()->Attribute("start_year"));
                imgDateExt = convertToString(resNode->ToElement()->Attribute("start_extension"));
                SmilNet::createNoteNode(iiNode, imgNode->Parent(), imageDur, imgDate, imgDateExt);
            }
        }
    }

    delete doc;
}



float SmilNet::createImageNodes (TiXmlNode* queryNode,  rList l, TiXmlNode* seqNode) {

    float duration = 0;

    std::string defRegion = convertToString(queryNode->ToElement()->Attribute("region"));
    if(defRegion.empty()) {
        defRegion = "images";
    }

    float imageDur = convertToFloat(queryNode->ToElement()->Attribute("imageDur"));
    if(imageDur == 0)
        imageDur = 2;


    // read query result and make image nodes
    TiXmlDocument* doc = new TiXmlDocument("tmp_query_result.xml");
    doc->LoadFile();


    TiXmlElement* rootNode;
    rootNode = doc->FirstChildElement("results");
   if(rootNode) {
        for ( TiXmlNode* resNode = rootNode->FirstChild("Creation"); resNode; resNode = resNode->NextSibling("Creation")) {
           if(resNode) {
                duration += SmilNet::createImageNode(resNode, queryNode, defRegion, imageDur, l, seqNode);
           }
        }
    }
    delete doc;
    return duration;
}


void SmilNet::readTmp ( TiXmlNode* node) {

    std::string res, namet, imgDate, imgDateExt;

    std::map<std::string, TiXmlNode*>          replaceList;
    std::map <std::string, TiXmlNode*> :: const_iterator ite;

    // make list of replaced medias
    for ( TiXmlNode* replaceNode = node->FirstChild("replaceMedia"); replaceNode; replaceNode = replaceNode->NextSibling("replaceMedia")) {
            
        std::string src = convertToString(replaceNode->ToElement()->Attribute("src"));
        if(!src.empty()) {
           // TiXmlElement* holder = new TiXmlElement("holder");
            TiXmlNode* holder = replaceNode->Clone();
            holder->SetValue("seq");
           // replaceNode->RemoveChild(replaceNode->FirstChild());
            replaceList[src] = holder;
            replaceNode->Parent()->RemoveChild(replaceNode);
        }
    }

    //read XML
    namet = "";
    TiXmlDocument* doc = new TiXmlDocument("tmp_query_result.xml");
    doc->LoadFile();


    std::string defRegion = convertToString(node->ToElement()->Attribute("region"));
    if(defRegion.empty()) {
        defRegion = "images";
    }


    float imageDur = convertToFloat(node->ToElement()->Attribute("imageDur"));
    if(imageDur == 0)
        imageDur = 2;

    TiXmlElement* rootNode;
    rootNode = doc->FirstChildElement("results");
   if(rootNode) {
        for ( TiXmlNode* resNode = rootNode->FirstChild(); resNode; resNode = resNode->NextSibling()) {
          // TiXmlElement* imgNode = rootNode->FirstChildElement("Digital_Image");
           if(resNode) {

                // image query by creation
                if(resNode->Value() == std::string("Creation")) {

                    SmilNet::createImageNode(resNode, node, defRegion, imageDur, replaceList, node);

                // image query
                } else if(resNode->Value() == std::string("Digital_Image")) {



               } else {
                    
                    float duration = 0;
                    TiXmlElement * recpar = new TiXmlElement( "par" );
                    node->LinkEndChild(recpar);

                    TiXmlElement * seq = new TiXmlElement( "seq" );
                    std::string recId = convertToString(resNode->ToElement()->Attribute("id"));
                    recpar->LinkEndChild(seq);

                    // images ordere by time
                    std::string query = "<search><Creation><has_created><Digital_Image><represents><CRM_Entity id='";
                    query.append(recId);
                    query.append("' /></represents></Digital_Image></has_created></Creation>");
                    query.append("<result has_time-span='start_year'>");
                    query.append("<has_created is_identified_by='filename' has_note='message' />");
                    query.append("</result></search>");

                    SmilNet::makeQuery(query);
                    duration = SmilNet::createImageNodes(node, replaceList, (TiXmlNode*)seq);

                    // add title (recordName)
                    std::string name = convertToString(resNode->ToElement()->Attribute("name"));
                    TiXmlElement * text = new TiXmlElement( "text" );
                    TiXmlText * tt = new TiXmlText( name );
                    text->SetAttribute("region", "IDA_recordName");
                    text->SetAttribute("dur", duration);
                    text->LinkEndChild(tt);
                    recpar->LinkEndChild(text);


               }
           }
        }
   }


    delete doc;

}

float SmilNet::createImageNode (TiXmlNode* resNode, TiXmlNode* queryNode, std::string defRegion, float imageDur, rList replaceList, TiXmlNode* seqNode ) {

    std::string res, namet, imgDate, imgDateExt;
    std::map <std::string, TiXmlNode*> :: const_iterator ite;

    imgDate = convertToString(resNode->ToElement()->Attribute("start_year"));
    imgDateExt = convertToString(resNode->ToElement()->Attribute("start_extension"));
    TiXmlElement* cNode = resNode->FirstChildElement("has_created");
    if(cNode) {
        TiXmlElement* imgNode = cNode->FirstChildElement("Digital_Image");
        if(imgNode) {
            TiXmlElement * par = new TiXmlElement( "par" );
            seqNode->LinkEndChild(par);

            std::string fileName = convertToString(imgNode->ToElement()->Attribute("filename"));
            if(!fileName.empty()) {
                fileName.append(".jpg");
                std::cout << fileName << std::endl;
                
                // check if this should be replaced
                ite = replaceList.find(fileName);
                if(ite == replaceList.end()) {
                    TiXmlElement * img = new TiXmlElement( "img" );
                    img->SetAttribute("src", fileName);
                    img->SetAttribute("region", defRegion);
                    img->SetAttribute("dur", imageDur);
                    par->LinkEndChild(img);

                    SmilNet::createNoteNode(imgNode, par, imageDur, imgDate, imgDateExt);

                    // add default nodes if there are any
                    TiXmlNode* defaultNode = queryNode->FirstChild("defaultRegion");
                    if(defaultNode) {
                        for ( TiXmlNode* subDefNode = defaultNode->FirstChild(); subDefNode; subDefNode = subDefNode->NextSibling()) {
                            img->LinkEndChild(subDefNode->Clone());
                        }
                    }


                } else {
                    seqNode->LinkEndChild(ite->second);
                }
                return imageDur;
            }

        }
    }
    return 0.0;
}



void SmilNet::createNoteNode (TiXmlNode* imgNode, TiXmlNode* parNode, float imageDur, std::string imgDate, std::string imgDateExt) {
std::cout << imgNode->Value() << std::endl;
    std::string name = convertToString(imgNode->ToElement()->Attribute("message"));
    if(name.empty()) {
        //name = b->Value();
        name = "";
    }
    if(imgDateExt.empty())
        name += " (" + imgDate + ")";
    else
        name += " (" + imgDate + "-luku)";

    //std::cout << name << std::endl;
    TiXmlElement * text = new TiXmlElement( "text" );
    TiXmlText * tt = new TiXmlText( name );
    text->SetAttribute("region", "IDA_imageNote");
    text->SetAttribute("dur", imageDur);
    text->LinkEndChild(tt);
    parNode->LinkEndChild(text);
}





