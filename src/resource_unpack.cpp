#include <fstream>
#include <iostream>
#include <vector>
#include <cstring>
#include <string>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif



static const int ID_MAXSIZE = 32;

struct resource {
	size_t size;
	size_t ptr;
	char res_id[ID_MAXSIZE];
};

std::vector<resource> resFiles;
std::ifstream resourceIfs;

void removeExtension(char *filename) {
	char *c = filename + strlen(filename) - 1;
	for (; c > filename; --c) {
		if (*c == '.') {
			*c = '\0';
			break;
		}
		*c = '\0';
	}
}

unsigned int readInt(std::ifstream &ifs) {
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
	resourceIfs.open(filename, std::ios::binary);

	if (resourceIfs.fail()) {
		return false;
	}

	if (readInt(resourceIfs) != 0x255435f4) {
		return false;
	}

	resFiles.resize(readInt(resourceIfs));

	for (resource &res : resFiles) {
		resourceIfs.read(res.res_id, ID_MAXSIZE);
		res.size = readInt(resourceIfs);
		res.ptr = readInt(resourceIfs);
	}

	return true;
}

bool saveFiles(const char *output_dir) {
	for (resource &res : resFiles) {
		std::string filename = output_dir;
		filename.append(res.res_id);
		
		std::vector<char> data(res.size);
		
		resourceIfs.seekg(res.ptr);
		resourceIfs.read(data.data(), res.size);
		
		std::ofstream ofs(filename, std::ios::binary);
		ofs.write(data.data(), data.size());
		
		if (ofs.fail()) {
			std::cerr << "Could not save file \"" << filename << "\"\n";
			return false;
		}
	}
	
	return true;
}

int main(int argc, char **argv) {
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " input [output_dir]\n";
		return -1;
	}
	
	const char *input = argv[1];
	char output[FILENAME_MAX];
	memset(output, 0, sizeof(output));
	
	if (argc >= 3) {
		strncpy(output, argv[2], FILENAME_MAX);
	} else {
		strncpy(output, input, FILENAME_MAX);
		removeExtension(output);
	}
	
	char lastChar = output[strlen(output)-1];
	if (lastChar != '\\' && lastChar != '/') {
#ifdef _WIN32
		output[strlen(output)] = '\\';
#else
		output[strlen(output)] = '/';
#endif
	}
	
	if (output[0] == '\\' || output[0] == '/') {
		memset(output, 0, sizeof(output));
	} else {
		std::filesystem::create_directory(output);
	}
	
	if (!openResourceFile(input)) {
		std::cerr << "Could not open resource file \"" << input << "\"\n";
		return -2;
	}
	
	if (!saveFiles(output)) {
		std::cerr << "Error. Terminating.\n";
		return -3;
	}
	
	std::cout << "Operation completed succesfully.\n";
	return 0;
}

