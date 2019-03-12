#pragma once

#ifndef LEAFTRANSFORM
#define LEAFTRANSFORM

#include <vector>
#include <string>
#include <list>
#include <regex>

#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>
#include <libxml/tree.h>
#include <libxml/HTMLtree.h>

#define ENCODING "UTF-8"
#define XML_DOC "messages.xml"

namespace cppLeaf {
	class LeafTransform {
	
	private:
		xmlDocPtr doc = NULL;
		xmlDocPtr metaDoc = NULL;
		
		std::list<std::pair<char*, char*>> list;

	public:
		LeafTransform() {};
		~LeafTransform() {};
		
		xmlDocPtr readFile(char* fileName);
		xmlDocPtr readStream(char *fileStream, long fileSize, std::string type);
		void thymeleaf2Html(htmlDocPtr doc);
		void replaceTag(xmlNodePtr curNode);
		void findAttr(xmlNodePtr curNode);
		void findThymeleafTag(xmlAttrPtr attrNode);
		void removeDuplicationAttr(xmlAttrPtr attrNode, const xmlChar* thymeleafTagName);
		std::string replaceAttrContent(xmlChar *c);
		void replaceAttr(xmlAttrPtr attrNode);
		xmlNodePtr findNextNode(xmlNodePtr curNode);
		void initXml(xmlDocPtr xmlDoc);
		void parseXML(xmlNodePtr curNode);
		std::pair<char *, char *> getXMLValue(xmlNodePtr curNode);
		void setXMLValueLIst(std::pair<char *, char *>);
		char* findMappingValue(char *qry, xmlAttrPtr attrNode);
		std::string procThymeLeaf(char *fileName);
		std::string procThymeLeaf(char *htmlName, char *xmlName);
		std::string html2string(xmlDocPtr doc);
		std::string removeCarriageReturn(std::string html);
		std::string procThymeLeaf(char *fileStream, int fileSize, char *xmlStream, int xmlSize);
		std::string procThymeLeaf(char *fileStream, int fileSize);
	};
}


#endif;