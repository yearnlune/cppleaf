#include "LeafTransform.h"
#include <iostream>
#include <Windows.h>
#include <fstream>

#define FILENAME "serviceUI.html"
#define METANAME "messages.xml"

#pragma execution_character_set( "utf-8" )

std::string file2memory(const char *fileName, std::string str) {
	std::string temp;
	std::ifstream file;

	file.open(fileName, std::ios::in);
	while (std::getline(file, temp)) {
		str += temp + "\n";
	}

	file.close();
	return str;
}

int main() {
	SetConsoleOutputCP(65001);

	std::string test;
	cppLeaf::LeafTransform *obj = new cppLeaf::LeafTransform();
	//test = obj->procThymeLeaf((char*) FILENAME);

	std::string html = "";
	std::string xml = "";
	
	html = file2memory(FILENAME, html);
	xml = file2memory(METANAME, xml);
	test = obj->procThymeLeaf((char *)html.c_str(), html.length(), (char *)xml.c_str(), xml.length());
	if (test.length() != 0)
		printf("%s", test.c_str());
	
	delete(obj);
	system("pause");
	return 0;
}

