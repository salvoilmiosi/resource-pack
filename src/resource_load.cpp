#include "resource_load.h"

#include <unordered_map>
#include <fstream>
#include <bzlib.h>


static const int ID_MAXSIZE = 32;

struct resource {
	size_t uncompressed_size = 0;
	size_t size = 0;
	size_t ptr = 0;
	std::string filename;
	char *data = nullptr;

	~resource() {
		if (data) free(data);
		data = nullptr;
	}
};

static std::unordered_map<std::string, resource> resFiles;

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

		res.uncompressed_size = readInt(ifs);
		res.size = readInt(ifs);
		res.ptr = readInt(ifs);
		res.filename = filename;

		resFiles[res_id] = res;

		--numRes;
	}

	return true;
}

static resource *getUncompressedData(const char *RES_ID) {
	auto it = resFiles.find(RES_ID);
	if (it == resFiles.end()) return nullptr;

	resource &res = it->second;
	if (res.data) return &res;

	char *compressed = (char *)malloc (res.size);

	std::ifstream ifs(res.filename, std::ios::binary);
	ifs.seekg(res.ptr);
	ifs.read(compressed, res.size);

	unsigned int uncompressed_size = res.uncompressed_size;
	res.data = (char *) malloc(uncompressed_size);
	int status = BZ2_bzBuffToBuffDecompress(res.data, &uncompressed_size, compressed, res.size, 0, 0);

	free(compressed);
	if (status != BZ_OK) {
		free(res.data);
		res.data = nullptr;
		return nullptr;
	}

	return &res;
}

SDL_RWops *getResourceRW(const char *RES_ID) {
	resource *res = getUncompressedData(RES_ID);
	if (!res) return nullptr;
	return SDL_RWFromConstMem(res->data, res->uncompressed_size);
}

std::string loadStringFromResource(const char *RES_ID) {
	resource *res = getUncompressedData(RES_ID);
	if (!res) return "";
	return std::string(res->data, res->uncompressed_size);
}