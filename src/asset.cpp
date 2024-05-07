#include "fi/asset/asset.h"

#include "fi/fiUtil.h"

namespace FI {


Asset::Asset()
{
}

Asset::Asset(const std::string &path)
	: m_path(path)
{
	m_filePtr = fopen(m_path.c_str(), "rb");

	if (m_filePtr)
	{
		fread(&m_header, sizeof(FileHeader), 1, m_filePtr);

		for (int i=0; i < m_header.listSize; i++)
		{
			FileInfo info;
			fread(&info.strSz, sizeof(info.strSz), 1, m_filePtr);
			char *str = new char[info.strSz+1];
			memset(str, 0, info.strSz+1); // zero whole field and null terminator a the end
			fread(str, info.strSz, 1, m_filePtr);
			info.path = str;
			delete[] str;
			fread(&info.offset, sizeof(info.offset), 1, m_filePtr);
			fread(&info.size, sizeof(info.size), 1, m_filePtr);
			m_fileMap[info.path] = info;
			m_fileList.push_back(info);
		}
	}
	else
	{
		m_buffer.reserve(1024 * 1024 * 200); // reserve 200 MB
	}
}

Asset::~Asset()
{
	m_buffer.clear();
	m_fileList.clear();
	m_fileMap.clear();
	if (m_filePtr)
	{
		fclose(m_filePtr);
	}
}

bool Asset::find(const std::string &path)
{
	m_currentFileInfo = FileInfo();
	if (auto fileInfo = m_fileMap.find(path); fileInfo != m_fileMap.end())
	{
		m_currentFileInfo = fileInfo->second;
		int result = !fseek(m_filePtr, m_header.dataOffset + m_currentFileInfo.offset, SEEK_SET);
		return result;
	}
	else
	{
		return false;
	}
}

void Asset::add(const std::string &path)
{
	FILE *fp = fopen(path.c_str(), "rb");

	// determin file size
	fseek(fp, NULL, SEEK_END);

	// ftell() returns signed data size of minimum guaranteed 32bit on windows MSVC (ex. 2GB max size)
	// long is 8 bytes on mac/linux/GCC though.
	uint64_t sz = ftell(fp);
	rewind(fp);

	uint64_t bufOffset = m_buffer.size();
	m_buffer.resize(m_buffer.size() + sz);
	fread(m_buffer.data() + bufOffset, sz, 1, fp);

	FileInfo info = {path.size(), path, bufOffset, sz};
	m_fileList.push_back(info);

	fclose(fp);
}

size_t Asset::read(uint8_t *buf)
{
	return fread(buf, m_currentFileInfo.size, 1, m_filePtr);
}

size_t Asset::write(const std::string &path)
{
	FILE *fp = fopen(path.c_str(), "wb");

	// write header
	FileHeader header;
	header.listSize = m_fileList.size();
	header.dataOffset += sizeof(FileHeader);

	size_t bytesWritten = 0;

	// calculate where the data pool starts
	for (auto fileInfo : m_fileList)
	{
		header.dataOffset += sizeof(fileInfo.strSz);
		header.dataOffset += fileInfo.path.size();
		header.dataOffset += sizeof(fileInfo.offset);
		header.dataOffset += sizeof(fileInfo.size);
	}

	bytesWritten += fwrite(&header, sizeof(header), 1, fp);

	// write file info entries
	for (auto fileInfo : m_fileList)
	{
		bytesWritten += fwrite(&fileInfo.strSz, sizeof(fileInfo.strSz), 1, fp);
		bytesWritten += fwrite(fileInfo.path.c_str(), fileInfo.path.size(), 1, fp);
		bytesWritten += fwrite(&fileInfo.offset, sizeof(fileInfo.offset), 1, fp);
		bytesWritten += fwrite(&fileInfo.size, sizeof(fileInfo.size), 1, fp);
	}

	FI::LOG("calculated data start:", header.dataOffset, "actual:", ftell(fp));

	// write data blob
	bytesWritten += fwrite(m_buffer.data(), m_buffer.size(), 1, fp);
	fclose(fp);

	return bytesWritten;
}

FILE *Asset::file()
{
	return m_filePtr;
}

long Asset::offset()
{
	return m_currentFileInfo.offset;
}

long Asset::size()
{
	return m_currentFileInfo.size;
}

std::string Asset::path()
{
	return m_currentFileInfo.path;
}

std::vector<std::string> Asset::list()
{
	std::vector<std::string> out;
	for (const auto &fileInfo : m_fileList)
	{
		out.push_back(fileInfo.path);
	}
	return out;
}



} // NS FI