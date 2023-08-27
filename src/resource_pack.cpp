#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>
#include <cstring>

#include <bzlib.h>



const size_t ID_MAXSIZE = 32;

struct fileData {
	char res_id[ID_MAXSIZE];
	char filename[FILENAME_MAX];
	char *data = nullptr;
	size_t size;
	size_t uncompressed_size;

	~fileData() {
		if (data) free(data);
		data = nullptr;
	}
};

void trim(std::string &s) {
	auto is_not_space = [](char c) { return !isspace(c); };
	s.erase(s.begin(), find_if(s.begin(), s.end(), is_not_space));
	s.erase(find_if(s.rbegin(), s.rend(), is_not_space).base(), s.end());
}

bool parseResource(const char *filename, std::vector<fileData> &files) {
	std::ifstream ifs(filename);

	std::string resource_file_dir = filename;
	size_t slash_pos = resource_file_dir.find_last_of("/\\");
	resource_file_dir = resource_file_dir.substr(0, slash_pos);

	if (!resource_file_dir.empty()) {
		resource_file_dir.append("/");
	}

	if (ifs.fail()) {
		std::cerr << "Could not open input file \"" << filename << "\"\n";
		return false;
	}

	std::string line;
	fileData file;

	int line_num = 0;
	bool escape;

	while (getline(ifs, line)) {
		trim(line);
		
		++line_num;

		if (line.empty()) continue;
		if (line.at(0) == '#') continue;

		int state = 0;
		// 0 Reading ID
		// 1 Space between ID and filename
		// 2 Reading filename
		// 3 Over filename

		memset(file.res_id, 0, sizeof(file.res_id));
		memset(file.filename, 0, sizeof(file.filename));

		char *i = file.res_id;
		
		escape = false;

		for (char c : line) {
			switch (state) {
			case 0:
				if (isspace(c)) {
					++state;
				} else {
					*i++ = c;
					break;
				}
				// fall through
			case 1:
				if (isspace(c)) {
					break;
				} else {
					i = file.filename;

					if (!resource_file_dir.empty()) {
						strncpy(i, resource_file_dir.c_str(), FILENAME_MAX);
						i += resource_file_dir.size();
					}

					++state;
					if (c == '\"') {
						break;
					}
				}
				// fall through
			case 2:
				if (!escape) {
					if (c == '\\') {
						escape = true;
						break;
					} else if (c == '\"') {
						++state;
						break;
					}
				} else {
					escape = false;
				}
				*i++ = c;
				break;
			case 3:
				file.filename[0] = '\0';
				break;
			}
		}

		if (file.res_id[0] == '\0' || file.filename[0] == '\0') {
			std::cerr << "Syntax error at line #" << line_num << ":\n" << line << "\n";
			return false;
		}

		files.push_back(file);
	}

	return true;
}

bool openResources(std::vector<fileData> &files) {
	for (fileData &file : files) {
		FILE *f_input = fopen(file.filename, "rb");
		if (f_input == NULL) {
			fprintf(stderr, "Can't open %s for reading\n", file.filename);
			return -1;
		}

		// Get the file length
		fseek(f_input, 0, SEEK_END);
		file.uncompressed_size = ftell(f_input);
		fseek(f_input, 0, SEEK_SET);

		char *buf = (char *) malloc(file.uncompressed_size);

		fread(buf, file.uncompressed_size, 1, f_input);
		fclose(f_input);

		// allocate for bz2.
		file.size = (file.uncompressed_size + file.uncompressed_size / 100 + 1) + 600; // as per the documentation

		file.data = (char *) malloc(file.size);

		// compress the data
		int status = BZ2_bzBuffToBuffCompress(file.data, (unsigned int *) &file.size, buf, file.uncompressed_size, 9, 1, 0);

		if (status != BZ_OK) {
			fprintf(stderr, "Failed to compress data: error %i\n", status);
			return false;
		}

		// and be very lazy
		free(buf);
	}

	return true;
}

void writeInt(std::ofstream &ofs, const int num) {
	char str[4];
	str[0] = (num & 0xff000000) >> (8 * 3);
	str[1] = (num & 0x00ff0000) >> (8 * 2);
	str[2] = (num & 0x0000ff00) >> (8 * 1);
	str[3] = (num & 0x000000ff) >> (8 * 0);

	ofs.write(str, 4);
}

bool saveResources(const char *filename, std::vector<fileData> &files) {
	std::ofstream ofs(filename, std::ios::binary);

	if (ofs.fail()) return false;

	const int tableSize = ID_MAXSIZE + sizeof(int) * 3;
	int ptr = tableSize * files.size() + sizeof(int) * 2;

	writeInt(ofs, 0x255435f4);

	writeInt(ofs, files.size());

	for (fileData &file : files) {
		ofs.write(file.res_id, ID_MAXSIZE);
		
		writeInt(ofs, file.size);
		writeInt(ofs, file.uncompressed_size);
		writeInt(ofs, ptr);

		if (ofs.fail()) return false;

		ptr += file.size;
	}

	for (fileData &file : files) {
		ofs.write(file.data, file.size);

		if (ofs.fail()) return false;
	}

	return true;
}

void changeExtension(char *filename, const char *ext) {
	char *c = filename + strlen(filename) - 1;
	for (; c > filename; --c) {
		if (*c == '.') {
			break;
		} else {
			*c = '\0';
		}
	}
	
	for (const char *e = ext; *e != '\0'; ++e) {
		++c;
		*c = *e;
	}
}

int main(int argc, char **argv) {
	if (argc < 2) {
		std::cout << "Usage: " << argv[0] << " input [output]\n";
		return -1;
	}

	const char *input = argv[1];
	char output[FILENAME_MAX];

	if (argc >= 3) {
		strcpy(output, argv[2]);
	} else {
		strcpy(output, input);
		changeExtension(output, "dat");
	}

	std::vector<fileData> files;

	if (!parseResource(input, files)) {
		std::cerr << "Could not parse input file\n";
		return -2;
	}

	if (!openResources(files)) {
		std::cerr << "Could not open files\n";
		return -3;
	}

	if (!saveResources(output, files)) {
		std::cerr << "Could not save resource file\n";
		return -4;
	}
	
	return 0;
}