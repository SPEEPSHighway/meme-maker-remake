#include "ObjectImport.h"
#include <MemAccess.h>
#include "dirent.h"

//This file contains the code to import models and animations, used by the Animation Player and Prop Spawner.

//Temp ptrs
static NJS_OBJECT* importedObj;
static NJS_OBJECT* colObj;
static NJS_MOTION* importedMtn;

const char* mdlErrorMessage[] = {
	"\aERROR: Incorrect Object Format.\n\aIt must be an SA1MDL file.", NULL
};

const char* mtnErrorMessage[] = {
	"\aERROR: Incorrect Motion Format.\n\aIt must be an SAANIM file.", NULL
};

/// <summary>
/// Scans a directory and returns the name of a numbered directory or folder.
/// </summary>
/// <param name="str">name</param>
/// <param name="path">path</param>
/// <param name="fileNum">file number</param>
/// <param name="isDir">True if searching for directories, False if not.</param>
void getNameOfFile(std::string* str, std::string path, Sint32 fileNum, Bool isDir) {
	DIR* folder;

	struct dirent* entry;
	folder = opendir(path.c_str());
	int files = 0;

	if (!folder)
		return;

	Sint32 i = 0;
	*str = "";

	Bool isCorrectFileType;
	while (entry = readdir(folder))
	{
		isCorrectFileType = FALSE;
		if (isDir) {
			if (entry->d_type == DT_DIR && !strchr(entry->d_name, '.')) {
				isCorrectFileType = TRUE;
			}
		}
		else if (entry->d_type == DT_REG) {
			isCorrectFileType = TRUE;
		}


		if (isCorrectFileType) {
			if (i == fileNum) {
				*str = entry->d_name;
				break;
			}
			++i;
		}
	}
	closedir(folder);
}

/// <summary>
/// Adds the key to a pointer's address.
/// </summary>
/// <param name="base"></param>
/// <param name="addr"></param>
static void addPointerKey(void* base, void** addr) {
	WriteData((Uint32*)addr, (Uint32)((Uint32)base + (Uint32)*addr));
}

/// <summary>
/// Adds the pointer key to all pointers in an NJS_MOTION (MDATA2 only)
/// </summary>
/// <param name="object">NJS_MOTION pointer</param>
static void setMotionPointerKeys(void** addr, NJS_MOTION* motion) {
	if (motion->mdata) {
		addPointerKey(*addr, (void**)&motion->mdata);
		NJS_MDATA2* mdata = (NJS_MDATA2*)motion->mdata;
		NJS_MDATA3* mdata3 = (NJS_MDATA3*)motion->mdata;
		Uint16 inp_fn = motion->inp_fn; //MDATA size

		//End of mdata is the pointer to its base so use that to stop the loop
		while (mdata->p[0] != motion->mdata){
			if (mdata->p[0])
				addPointerKey(*addr, (void**)&mdata->p[0]);
			if (mdata->p[1])
				addPointerKey(*addr, (void**)&mdata->p[1]);

			if (inp_fn == 3)
			{
				//MDATA3
				mdata3 = (NJS_MDATA3*)mdata;
				if (mdata->p[2])
					addPointerKey(*addr, (void**)&mdata->p[2]);
				++mdata3;
				mdata = (NJS_MDATA2*)mdata3;
			}
			else
			{
				++mdata;
			}
		}
	}
}

/// <summary>
/// Adds the pointer key to all pointers in an NJS_OBJECT
/// </summary>
/// <param name="object">NJS_OBJECT pointer</param>
static void setObjectPointerKeys(void** addr, NJS_OBJECT* object) {

	NJS_OBJECT* this_obj = object;

	do {
		if (this_obj->model) {
			addPointerKey(*addr, (void**)&this_obj->model);
			NJS_MODEL* model = (NJS_MODEL*)this_obj->model;

			if (model->points)
				addPointerKey(*addr, (void**)&model->points);

			if (model->normals)
				addPointerKey(*addr, (void**)&model->normals);

			if (model->meshsets)
				addPointerKey(*addr, (void**)&model->meshsets);

			if (model->mats)
				addPointerKey(*addr, (void**)&model->mats);

			//Meshsets
			for (Sint32 i = 0; i < model->nbMeshset; ++i) {
				if (model->meshsets[i].meshes)
					addPointerKey(*addr, (void**)&model->meshsets[i].meshes);

				if (model->meshsets[i].normals)
					addPointerKey(*addr, (void**)&model->meshsets[i].normals);

				if (model->meshsets[i].vertcolor)
					addPointerKey(*addr, (void**)&model->meshsets[i].vertcolor);

				if (model->meshsets[i].vertuv)
					addPointerKey(*addr, (void**)&model->meshsets[i].vertuv);
			}

			//SA1MDL meshsets are 4 bytes short. Need to convert them to NJS_MESHSET_SADX
			NJS_MESHSET_SADX* meshset_test = (NJS_MESHSET_SADX*)model->meshsets;
			if (model->meshsets && meshset_test->buffer) { //Buffer isn't used until the model is drawn so if it's not 0 we know the struct needs correcting. 
				NJS_MESHSET_SADX* meshset = (NJS_MESHSET_SADX*)MAlloc(sizeof(NJS_MESHSET_SADX) * model->nbMeshset);

				for (Sint32 i = 0; i < model->nbMeshset; ++i) {
					meshset[i].attrs = model->meshsets[i].attrs;
					meshset[i].meshes = model->meshsets[i].meshes;
					meshset[i].normals = model->meshsets[i].normals;
					meshset[i].nbMesh = model->meshsets[i].nbMesh;
					meshset[i].type_matId = model->meshsets[i].type_matId;
					meshset[i].vertcolor = model->meshsets[i].vertcolor;
					meshset[i].vertuv = model->meshsets[i].vertuv;
					meshset[i].buffer = 0;
				}

				WriteData((Uint32*)&model->meshsets, (Uint32)meshset);


			}
		}

		if (this_obj->sibling)
			addPointerKey(*addr, (void**)&this_obj->sibling);

		if (this_obj->child) {
			addPointerKey(*addr, (void**)&this_obj->child);
			setObjectPointerKeys(addr, this_obj->child);
		}


		this_obj = this_obj->sibling;
	} while (this_obj);
}

//Searches a loaded texlist file (TLS) for a string
static Sint32 findTLSstr(char* binaddr, const char* str) {

	Sint32 result = 0;

	//Need to find out how big the file is.
	Sint32 size = 0;
	for (Sint32 i = 0; str[i]; ++i) {
		++size;
	}

	char* testEnd = (char*)late_alloca(size);

	for (Sint32 i = 0; binaddr[i]; ++i) {
		if (binaddr[i] != str[0]) //skip stuff that obviously won't be the start of the string.
			continue;

		for (Sint32 j = 0; j < size; ++j) {
			testEnd[j] = binaddr[i + j];
		}

		if (!strcmp(testEnd, str)) {
			result = i + size;
			break;
		}
	}
	syFree(testEnd);
	testEnd = 0;

	return result;

}

/// <summary>
/// Import a texlist from a .tls file
/// </summary>
/// <param name="path">filepath</param>
/// <returns></returns>
NJS_TEXLIST* importTLS(const char* path) {
	//Load Textures
	char* binaddr = (char*)njOpenBinary(path);

	//Find texture count
	char* texnumaddr = &binaddr[findTLSstr(binaddr, "TextureNum  ")];
	Sint32 texnum = 0;
	Sint32 i = 0;
	while (texnumaddr[i] != ',') {
		++i;
	}

	Sint32 j = 0;
	do {
		--i;
		texnum += (Sint32)(texnumaddr[i] - '0') * (Sint32)njPow(10.0f, (Float)j++);
	} while (i);

	//Create texnames and texlist
	NJS_TEXNAME* texturenames = (NJS_TEXNAME*)MAlloc(sizeof(NJS_TEXNAME) * texnum);
	NJS_TEXLIST* texlist_importedObj = (NJS_TEXLIST*)MAlloc(sizeof(NJS_TEXLIST));
	texlist_importedObj->textures = texturenames;
	texlist_importedObj->nbTexture = texnum;

	//Scan the .tls file for texture names and create an NJS_TEXNAME list out of them.
	char* texnamestart = &binaddr[findTLSstr(binaddr, "TEXN ( \"")];
	for (Sint32 k = 0; k < texnum; ++k) {


		Sint32 strSize = 0;
		while (texnamestart[strSize] != '\"') {
			++strSize;
		}

		char* texname_str = (char*)late_alloca(strSize);

		for (Sint32 l = 0; l < strSize; ++l) {
			texname_str[l] = texnamestart[l];
		}
		texlist_importedObj->textures[k].filename = texname_str;

		if (k < texnum - 1) //Don't do on last entry
			texnamestart = &texnamestart[findTLSstr(texnamestart, "TEXN ( \"")];

	}

	//Get rid of the loaded tls file, don't need it anymore.
	njCloseBinary(binaddr);

	return texlist_importedObj;
}


/// <summary>
/// Import an .saanim file.
/// </summary>
/// <param name="data">data</param>
/// <param name="path">filepath</param>
/// <returns>NJS_MOTION from file</returns>
NJS_MOTION* importSAANIM(void* data, const char* path) {
	data = (LPVOID*)MAlloc(sizeof(LPVOID));
	if (LoadFileWithMalloc(path, &data) != -1) {
		MM2_SAMDL_Header* header = (MM2_SAMDL_Header*)data;

		//Check the file is the correct type.
		if (!strcmp(header->str, "SAANIM")) {
			addPointerKey(data, (void**)&header->header.motion);
			importedMtn = header->header.motion;
			setMotionPointerKeys(&data, importedMtn);

			if (importedMtn->mdata) {
				return importedMtn;
			}
		}
		else {
			//Not correct type
			Serif_Play(866); // Sonic: "No way! Nuh-uh!"
			HintMainMessagesTime(mtnErrorMessage, 120);
		}

		PrintDebug("\nMeme Maker 2: ERROR - Failed to read saanim file.");
		syFree(data);
		data = 0;
	}
	else {
		PrintDebug("\nMeme Maker 2: ERROR - Failed to load saanim file.");
		Free(data);
		data = 0;
	}
	return NULL;
}


/// <summary>
/// Import an .sa1mdl file.
/// </summary>
/// <param name="data">data</param>
/// <param name="path">filepath</param>
/// <param name="needsRootModel">if true, only returns if obj->model exists in the root node.</param>
/// <returns>NJS_OBJECT from file</returns>
NJS_OBJECT* importSA1MDL(void* data, const char* namestr, Bool needsRootModel) {
	data = (LPVOID*)MAlloc(sizeof(LPVOID));
	if (LoadFileWithMalloc(namestr, &data) != -1) {
		MM2_SAMDL_Header* header = (MM2_SAMDL_Header*)data;

		//Check the file is the correct type.
		if (!strcmp(header->str, "SA1MDL")) {
			addPointerKey(data, (void**)&header->header.object);
			importedObj = header->header.object;
			setObjectPointerKeys(&data, importedObj);

			if (!needsRootModel)
				return importedObj;
			else if (importedObj->model)
				return importedObj;
		}
		else {
			//Not correct type
			Serif_Play(866); // Sonic: "No way! Nuh-uh!"
			HintMainMessagesTime(mdlErrorMessage, 120);
		}

		PrintDebug("\nMeme Maker 2: ERROR - Failed to read sa1mdl file.");
		syFree(data);
		data = 0;
	}
	else {
		PrintDebug("\nMeme Maker 2: ERROR - Failed to load sa1mdl file.");
		Free(data);
		data = 0;
	}
	return NULL;
}