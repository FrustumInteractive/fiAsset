
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace FI {

class Asset
{
public:
	Asset();   // create asset package mode
	Asset(const std::string &path);    // we're in load asset package mode; use add() to populate
	virtual ~Asset();

	bool find(const std::string &path);     // find a file path in asset package

	void add(const std::string &path);      // add file to end of file list in asset package

	size_t read(uint8_t *buffer);           // read data to provided buffer for current found file

	size_t write(const std::string &path);  // write out combined faile asset package

	FILE *file();    // get C file pointer

	long offset();   // offset in bytes of current file
	long size();     // size in bytes of current file

	std::string path(); // path of current file

	std::vector<std::string> list(); // get list of files in the asset package

private:
	struct FileInfo
	{
		uint64_t strSz = 0;
		std::string path;
		uint64_t
			offset = 0,
			size = 0;
	};

	struct FileHeader
	{
		unsigned char id[4] = {'F', 'I', 'A', 'S'};
		uint32_t listSize = 0;
		uint64_t dataOffset = 0;
	};

	std::unordered_map<std::string, FileInfo> m_fileMap; // enclosed filesnames and their offset & size in pkg.
	std::vector<FileInfo> m_fileList;                    // list maintaining entry order of the added filenames.
	FileInfo m_currentFileInfo;
	FileHeader m_header;            // loaded file header info
	std::string m_path;             // path of (ex. '*.fas') file that is the asset package
	FILE *m_filePtr = nullptr;
	std::vector<uint8_t> m_buffer;  // raw data buffer to pack files into
};


} // NS FI