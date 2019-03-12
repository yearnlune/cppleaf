#include "LeafTransform.h"

namespace cppLeaf {

	// html �� xml������ �д� �޼ҵ�
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

	// thymeleaf������ html�� ��带 �о� �ش� ������ ������ html���·� �ٲ�
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

	// ����� type�� ELEMENT_NODE�� ��쿡�� ���� ex) <div ..... >
	void LeafTransform::replaceTag(xmlNodePtr curNode) {
		if (curNode->type == XML_ELEMENT_NODE) {
			findAttr(curNode);
		}
	}

	// �ش� ����� Attribute�� ã��
	void LeafTransform::findAttr(xmlNodePtr curNode) {
		for (xmlAttrPtr attrNode = curNode->properties; attrNode; attrNode = attrNode->next) {
			findThymeleafTag(attrNode);
		}
	}

	// �ش� ����� Attribute�� thymeleaf���·� �� ���� ã��
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

	// �ش� ����� thtmleaf���¸� ������ ���·� ����
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

	// �ش� ����� �������� ���Ͽ� ������ ���� ����
	std::string LeafTransform::replaceAttrContent(xmlChar *c) {
		std::regex patternExtact("[$#@]\\{(.+)\\}");
		return std::string(std::regex_replace((char*)c, patternExtact, "$1"));
	}

	// ��� Ž��
	xmlNodePtr LeafTransform::findNextNode(xmlNodePtr curNode) {
		if (!curNode) {
			return NULL;
		}
		if (!curNode->next) {
			return findNextNode(curNode->parent);
		}

		return curNode->next;
	}
	
	// Mapping �����ͷ� Ȱ��� xml������ �Ľ��ϱ� ���� ��� ����
	void LeafTransform::initXml(xmlDocPtr xmlDoc) {
		parseXML(xmlDocGetRootElement(xmlDoc)->children);
	}
	
	// �ش� XML�����͸� �Ľ��Ͽ� ã�Ƴ�
	void LeafTransform::parseXML(xmlNodePtr curNode) {
		std::pair<char *, char *> node;

		while (curNode) {
			setXMLValueLIst(getXMLValue(curNode));
			curNode = curNode->next;
		}
	}

	// �Ľ��� �����͸� ����Ʈ�� ����
	void LeafTransform::setXMLValueLIst(std::pair<char *, char *> node) {
		if (node.first && node.second) {
			list.push_back(node);
		}
	}

	// �ش� xml�� �����͸� pair<> �������� ã�Ƽ� ��ȯ
	std::pair<char*, char*> LeafTransform::getXMLValue(xmlNodePtr curNode) {
		if (curNode->type != XML_ELEMENT_NODE) {
			return std::pair<char *, char *>(NULL, NULL);
		}
		if (!curNode->children) {
			return std::pair<char *, char *>((char *)curNode->properties->children->content, (char *)"");
		}

		return std::pair<char *, char *>((char *)curNode->properties->children->content, (char *)curNode->children->content);
	}

	// key�� ���ؼ� mapping data Ȯ��
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
	
	// Thymeleaf�����͸� ������ html�����ͷ� �ٲٱ� ���� �Ϸ��� ����
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

	// Thymeleaf�����͸� ������ html�����ͷ� �ٲٱ� ���� �Ϸ��� ����
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
	
	// �ش� html�� �����͸� string ���·� ��ȯ
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