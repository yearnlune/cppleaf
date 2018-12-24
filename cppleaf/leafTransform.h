#pragma once

#ifndef LEAFTRANSFORM
#define LEAFTRANSFORM

#include <iostream>
#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>
#include <libxml/tree.h>
#include <hash_map>
#include <vector>
#include <string>

#define ENCODING "UTF-8"
#define XML_DOC "messages.xml"



class leafTransform {
private:
	xmlDocPtr doc = NULL;
	xmlDocPtr xmlDoc = NULL;
	xmlNodePtr curNode = NULL;
	std::hash_map<char*, char*> hsmap;
	

public: 
	leafTransform() {};
	~leafTransform() {
		
	};

	//read Html
	xmlDocPtr readHtml(char* fileName) {
		xmlDocPtr doc = NULL;

		doc = htmlParseFile(fileName, ENCODING);
		
		return doc;
	}

	//read xml
	xmlDocPtr readXml(char* fileName) {
		xmlDocPtr doc = NULL;

		doc = xmlParseFile(fileName);

		return doc;
	}
	
	void thymeleaf2Html(htmlDocPtr doc, xmlNodePtr curNode) {
		bool beginNode = true;
		while (curNode) {
			std::string buf = "";
			if (beginNode) {
				if (curNode->type == XML_ELEMENT_NODE) {
					
					for (xmlAttrPtr attrNode = curNode->properties; attrNode; attrNode = attrNode->next) {
						if (xmlStrstr(attrNode->name, (xmlChar*)"th:")) {
							
							attrNode->name = xmlStrsub(attrNode->name, 3, xmlStrlen(attrNode->name));

							if (xmlStrstr(attrNode->name, (xmlChar*)"text")) {
								// read xml mapping
								
							}
							else if(xmlStrstr(attrNode->name, (xmlChar*)"src") || xmlStrstr(attrNode->name, (xmlChar*)"img")){
								// @{abadsfa} @{} remove
								printf("======================== %s", attrNode->name);
								attrNode->children->content = xmlStrsub(attrNode->children->content, 2, xmlStrlen(attrNode->children->content) - 1);
							}

							
						}
						xmlNodePtr contents = attrNode->children;
						buf += std::string((char*) attrNode->name);
						buf += "= ";
						buf += "\"" + std::string((char*)contents->content) + "\" ";
				
					}
					buf = "<" + std::string((char*)curNode->name) + " " + buf + ">";
					
				}
				else if (curNode->type == XML_TEXT_NODE)
				{
					printf("%s", curNode->content);
				}
				else if (curNode->type == XML_COMMENT_NODE)
				{
					printf("/* %s */ ", curNode->name);
				}
			}
			if (beginNode && curNode->children) {
				curNode = curNode->children;
				beginNode = true;
			}	
			else if (beginNode && curNode->next) {
				curNode = curNode->next;
				beginNode = true;
			}
			else {
				curNode = curNode->parent->next;
				beginNode = true;

				if (curNode && curNode->type == XML_ELEMENT_NODE)
				{
					buf += "</"+ std::string((char*)curNode->name) +">";
				}
			}
				
			printf("%s", buf.c_str());
		}
	}

	void initXmlValue(xmlDocPtr xmlDoc) {
		xmlNodePtr curNode = getRootNode(xmlDoc);
		curNode = curNode->children;
		while (curNode) {
			if (curNode->type == XML_ELEMENT_NODE) {
				char* key = (char*)curNode->properties->children->content;
				char* value = (char*)"";
				if (curNode->children) {				
					value = (char*)curNode->children->content;
				}
				printf("%s : %s\n", key, value);
				hsmap.insert(key, value);
			}
			curNode = curNode->next;
		}
		
	}
	
	xmlNodePtr getRootNode(xmlDocPtr doc) {
		return xmlDocGetRootElement(doc);
	}
	
	
	void procThymeLeaf(char* fileName) {
		/*doc = readHtml(fileName);*/
		xmlDoc = readXml((char*)XML_DOC);
		initXmlValue(xmlDoc);
		/*if (doc == NULL) {
			printf("document is empty!!");
			return;
		}
		
		curNode = getRootNode(doc);
		
		if (curNode == NULL) {
			printf("Can't be found to RootNode");
			return;
		}

		thymeleaf2Html(doc, curNode);*/


	}



};

#endif // !1
