#pragma once
#include "SADXModLoader.h"
#include "Keys.h"

struct MM2_SAMDL_Header {
	char str[7];
	union {
		NJS_OBJECT* object;
		NJS_MOTION* motion;
	} header;
};

void getNameOfFile(std::string* str, std::string path, Sint32 fileNum, Bool isDir);
NJS_TEXLIST* importTLS(const char* path);
NJS_OBJECT* importSA1MDL(void* data, const char* path, Bool needsRootModel);
NJS_MOTION* importSAANIM(void* data, const char* path);