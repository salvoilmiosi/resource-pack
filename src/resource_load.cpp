#include "resource_load.h"

#include <map>
#include <fstream>



static const int ID_MAXSIZE = 32;

struct resource {
	size_t size = 0;
	size_t ptr = 0;
	std::string filename;
};

static std::map<std::string, resource> resFiles;

static unsigned int readInt(std::ifstream &ifs) {
	char data[4];
	ifs.read(data, 4);
	unsigned int num =
		(data[0] << 24 & 0xff000000) |
		(data[1] << 16 & 0x00ff0000) |
		(data[2] << 8 & 0x0000ff00) |
		(data[3] & 0x000000ff);
	return num;
}

bool openResourceFile(const char *filename) {
	std::ifstream ifs(filename, std::ios::binary);

	if (ifs.fail()) {
		return false;
	}

	if (readInt(ifs) != 0x255435f4) {
		return false;
	}

	int numRes = readInt(ifs);

	resource res;
	char res_id[ID_MAXSIZE];

	while (numRes > 0) {
		memset(res_id, 0, sizeof(res_id));

		ifs.read(res_id, ID_MAXSIZE);

		res.size = readInt(ifs);
		res.ptr = readInt(ifs);
		res.filename = filename;

		resFiles[res_id] = res;

		--numRes;
	}

	return true;
}

SDL_RWops *getResourceRW(const char *RES_ID) {
	auto it = resFiles.find(RES_ID);
	if (it != resFiles.end()) {
		resource &res = it->second;
		char *data = new char[res.size];

		std::ifstream ifs(res.filename, std::ios::binary);
		ifs.seekg(res.ptr);
		ifs.read(data, res.size);

		return SDL_RWFromConstMem(data, res.size);
	} else {
		return nullptr;
	}
}

std::string loadStringFromResource(const char *RES_ID) {
	auto it = resFiles.find(RES_ID);
	if (it != resFiles.end()) {
		resource &res = it->second;
		std::string data(res.size, '\0');

		std::ifstream ifs(res.filename, std::ios::binary);
		ifs.seekg(res.ptr);
		ifs.read(&data[0], res.size);

		return data;
	} else {
		return "";
	}
}