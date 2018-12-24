#include "leafTransform.h"
#include <Windows.h>
#define FILENAME "serviceUI.html"

#pragma execution_character_set( "utf-8" )

int main() {
	SetConsoleOutputCP(65001);
	leafTransform* obj = new leafTransform();
	obj->procThymeLeaf((char*) FILENAME);
	delete(obj);
	system("pause");
	return 0;
}