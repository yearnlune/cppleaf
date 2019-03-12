#include "LeafTransform.h"

namespace cppLeaf {

	// html 및 xml파일을 읽는 메소드
	xmlDocPtr LeafTransform::readFile(char* fileName) {
		// check html file
		if (std::string(fileName).find(".html") != std::string::npos) {
			return htmlParseFile(fileName, ENCODING);
		}

		return xmlParseFile(fileName);
	}

	xmlDocPtr LeafTransform::readStream(char *fileStream, long fileSize, std::string type) {
		if (type.find("html") != std::string::npos) {
			return htmlReadMemory((char*)fileStream, fileSize, "", ENCODING, HTML_PARSE_NOERROR);
		}
		
		return xmlParseMemory(fileStream, fileSize);
	}

	// thymeleaf형태의 html의 노드를 읽어 해당 정보로 기존의 html형태로 바꿈
	void LeafTransform::thymeleaf2Html(htmlDocPtr doc) {
		xmlNodePtr curNode = xmlDocGetRootElement(doc);

		while (curNode) {
			replaceTag(curNode);

			if (curNode->children) {
				curNode = curNode->children;
				continue;
			}

			curNode = findNextNode(curNode);
		}
	}

	// 노드의 type이 ELEMENT_NODE인 경우에만 진행 ex) <div ..... >
	void LeafTransform::replaceTag(xmlNodePtr curNode) {
		if (curNode->type == XML_ELEMENT_NODE) {
			findAttr(curNode);
		}
	}

	// 해당 노드의 Attribute를 찾음
	void LeafTransform::findAttr(xmlNodePtr curNode) {
		for (xmlAttrPtr attrNode = curNode->properties; attrNode; attrNode = attrNode->next) {
			findThymeleafTag(attrNode);
		}
	}

	// 해당 노드의 Attribute가 thymeleaf형태로 된 것을 찾음
	void LeafTransform::findThymeleafTag(xmlAttrPtr attrNode) {
		if (xmlStrstr(attrNode->name, (xmlChar*)"th:")) {
			
			replaceAttr(attrNode);
		}
	}

	void LeafTransform::removeDuplicationAttr(xmlAttrPtr attrNode, const xmlChar* thymeleafTagName) {
		xmlAttrPtr curAttrNode = attrNode->parent->properties;
		
		while (curAttrNode) {
			xmlAttrPtr nxtAttrNode;
			xmlAttrPtr prvAttrNode;
			nxtAttrNode = curAttrNode->next;
			prvAttrNode = curAttrNode->prev;

			if (xmlStrstr(curAttrNode->name, thymeleafTagName) && curAttrNode != attrNode) {
				if (prvAttrNode) {
					prvAttrNode->next = nxtAttrNode;
				}
				xmlUnlinkNode((xmlNodePtr)curAttrNode);
			}

			if (xmlStrstr(curAttrNode->name, (xmlChar*)"text") && (xmlStrstr(curAttrNode->name, thymeleafTagName))) {
				if (prvAttrNode) {
					prvAttrNode->next = nxtAttrNode;
				}
				xmlUnlinkNode((xmlNodePtr)curAttrNode);
			}

			curAttrNode = nxtAttrNode;
		}

	}

	// 해당 노드의 thtmleaf형태를 기존의 형태로 변경
	void LeafTransform::replaceAttr(xmlAttrPtr attrNode) {
		std::regex patternForMap("([u]?text|alt|placeholder)");
		
		// name
		attrNode->name = xmlStrsub(attrNode->name, 3, xmlStrlen(attrNode->name));
		// content
		strcpy((char *)attrNode->children->content,replaceAttrContent((xmlChar*)attrNode->children->content).c_str());
		
		if (std::regex_match((char*)attrNode->name, patternForMap)) {
			if (attrNode->parent->children && attrNode->parent->children->type == XML_TEXT_NODE) {
				strcpy((char *)attrNode->parent->children->content, findMappingValue((char*)attrNode->children->content, attrNode));
			}
		}
		removeDuplicationAttr(attrNode, attrNode->name);
	}

	// 해당 노드의 컨텐츠에 한하여 기존의 경우로 변경
	std::string LeafTransform::replaceAttrContent(xmlChar *c) {
		std::regex patternExtact("[$#@]\\{(.+)\\}");
		return std::string(std::regex_replace((char*)c, patternExtact, "$1"));
	}

	// 노드 탐색
	xmlNodePtr LeafTransform::findNextNode(xmlNodePtr curNode) {
		if (!curNode) {
			return NULL;
		}
		if (!curNode->next) {
			return findNextNode(curNode->parent);
		}

		return curNode->next;
	}
	
	// Mapping 데이터로 활용될 xml파일을 파싱하기 위한 노드 설정
	void LeafTransform::initXml(xmlDocPtr xmlDoc) {
		parseXML(xmlDocGetRootElement(xmlDoc)->children);
	}
	
	// 해당 XML데이터를 파싱하여 찾아냄
	void LeafTransform::parseXML(xmlNodePtr curNode) {
		std::pair<char *, char *> node;

		while (curNode) {
			setXMLValueLIst(getXMLValue(curNode));
			curNode = curNode->next;
		}
	}

	// 파싱한 데이터를 리스트에 저장
	void LeafTransform::setXMLValueLIst(std::pair<char *, char *> node) {
		if (node.first && node.second) {
			list.push_back(node);
		}
	}

	// 해당 xml의 데이터를 pair<> 형식으로 찾아서 반환
	std::pair<char*, char*> LeafTransform::getXMLValue(xmlNodePtr curNode) {
		if (curNode->type != XML_ELEMENT_NODE) {
			return std::pair<char *, char *>(NULL, NULL);
		}
		if (!curNode->children) {
			return std::pair<char *, char *>((char *)curNode->properties->children->content, (char *)"");
		}

		return std::pair<char *, char *>((char *)curNode->properties->children->content, (char *)curNode->children->content);
	}

	// key를 통해서 mapping data 확인
	char* LeafTransform::findMappingValue(char *key, xmlAttrPtr attrNode) {
		std::list<std::pair<char*, char*>>::iterator it = list.begin();

		if (list.empty()) {
			return (char*)"";
		}

		for ( ; it != list.end(); it++) {
			if (strcmp(key, it->first) == 0) {
				return it->second;
			}
		}
		
		return (char*)"";
	}
	
	// Thymeleaf데이터를 기존의 html데이터로 바꾸기 위한 일련의 과정
	std::string LeafTransform::procThymeLeaf(char *fileName) {
		doc = readFile(fileName);
		metaDoc = readFile((char*)XML_DOC);

		if (!(doc && metaDoc)) {
			return NULL;
		}

		initXml(metaDoc);
		thymeleaf2Html(doc);
		return html2string(doc);
	}

	// Thymeleaf데이터를 기존의 html데이터로 바꾸기 위한 일련의 과정
	std::string LeafTransform::procThymeLeaf(char *htmlName, char *xmlName) {
		doc = readFile(htmlName);
		metaDoc = readFile(xmlName);

		if (!(doc && metaDoc)) {
			return NULL;
		}

		initXml(metaDoc);
		thymeleaf2Html(doc);
		return html2string(doc);
	}

	std::string LeafTransform::procThymeLeaf(char *fileStream, int fileSize, char *xmlStream, int xmlSize) {
		doc = readStream((char *)fileStream, fileSize, "html");
		metaDoc = readStream((char *)xmlStream, xmlSize, "xml");
		
		if (!(doc && metaDoc)) {
			return "";
		}

		initXml(metaDoc);
		thymeleaf2Html(doc);
		return html2string(doc);
	}

	std::string LeafTransform::procThymeLeaf(char *fileStream, int fileSize) {
		doc = readStream(fileStream, fileSize, "html");
		metaDoc = readFile((char*)XML_DOC);

		if (!(doc && metaDoc)) {
			return NULL;
		}

		initXml(metaDoc);
		thymeleaf2Html(doc);
		return html2string(doc);
	}
	
	// 해당 html의 데이터를 string 형태로 반환
	std::string LeafTransform::html2string(xmlDocPtr doc) {
		xmlChar *c;
		int size;
		htmlSetMetaEncoding(doc, (const xmlChar*)"UTF-8");
		htmlDocDumpMemory(doc, &c, &size);

		return removeCarriageReturn(std::string((char*)c));
	}

	// Remove Carriage Return '&#13;' 
	std::string LeafTransform::removeCarriageReturn(std::string html) {
		std::regex pattern("&#13;");
		return std::regex_replace(html, pattern, "");
	}
	
}