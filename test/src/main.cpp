#include "fi/fiUtil.h"
#include "fi/asset/asset.h"

#include <sys/stat.h>
#include <string>
#include <vector>
#include <filesystem>

void removeTrailingSlash(std::string &str)
{
	if (str.length() > 0)
	{
		std::string::iterator it = str.end() - 1;
		if (*it == '/')
		{
			str.erase(it);
		}
	}
}

// Function to append sufficient tabs based on the previous text's length
// and the desired field size
std::string strcell(const std::string& text, int fieldSize)
{
	int length = text.length();

	// Determine if truncation is needed
	if (length > fieldSize) {
		// Truncate the text to fit the field size exactly
		return text.substr(0, fieldSize);
	}

	int totalSpaceNeeded = fieldSize - length;  // Calculate the needed spacing

	// Construct the string with tabs and additional spaces if necessary
	return text + std::string(totalSpaceNeeded, ' ');
}

int main(int argc, char *argv[])
{
	FI::Asset asset; // for one-by-one manaully adding individual files to it
	std::string fname = "package.fas";
	std::string extractPath="extracted";

	enum Operation
	{
		HELP_MODE,
		INFO_MODE,
		EXTRACT_MODE,
		PACK_MODE
	};

	Operation mode = PACK_MODE;

	enum ArgType // <0 is switch, >0 has val that follows, 0 no op.
	{
		TEST = -2,
		HELP,
		NONE,
		INFO,
		OUTPUT,
		EXTRACT
	};

	FI::UTIL::CmdLineArgMap argMap =
	{
		{"-o", OUTPUT},
		{"-e", EXTRACT},
		{"-i", INFO},
		{"-t", TEST},
		{"-h", HELP}
	};

	FI::UTIL::processCmdLineArgs(argc, argv, argMap, [&](int arg, std::string str)
	{
		switch (arg)
		{
			case HELP:
				mode = HELP_MODE;
				break;

			case OUTPUT:
				fname = str;
				break;

			case EXTRACT:
				mode = EXTRACT_MODE;
				std::filesystem::create_directory(str);
				removeTrailingSlash(str);
				extractPath = str + "/";
				break;

			case INFO:
				mode = INFO_MODE;
				asset = FI::Asset(str);
				break;

			case TEST:
				break;

			case NONE:
				if (str == FI_CMD_OPT_NONE)
				{
					mode = HELP_MODE;
					break;
				}

				// input file specified
				// check if path or dir
				struct stat s;
				if (stat(str.c_str(), &s) == 0)
				{
					if (s.st_mode & S_IFDIR)
					{
						// it's a directory
						if (mode == EXTRACT_MODE)
						{
							FI::LOG("error: cannot extract directories.");
							mode = HELP_MODE;
							break;
						}
						// remove trailing slash if its there
						removeTrailingSlash(str);

						FI::UTIL::listFiles(str, [&](const std::string &p)
						{
							asset.add(p); // rebuild relative path
						});
					}
					else if (s.st_mode & S_IFREG)
					{
						// it's a file
						if (mode == EXTRACT_MODE)
						{
							// input is an asset package to extract
							FI::Asset _asset(str); // let's use a different, isolated asset instance here
							auto list = _asset.list();
							for (auto path : list)
							{
								if (_asset.find(path))
								{
									auto size = _asset.size();
									uint8_t *buf = new uint8_t[size];
									_asset.read(buf);

									std::string outPath = extractPath + path;
									auto loc = outPath.find_last_of('/');
									std::string outPathDirs = outPath.substr(0, loc);
									std::filesystem::create_directories(outPathDirs);
									FILE *fp = fopen(outPath.c_str(), "wb");
									if (fp)
									{
										fwrite(buf, size, 1, fp);
										fclose(fp);
									}
									else
									{
										FI::LOG("error: failed to open file for writing:", outPath);
									}

									delete[] buf;
								}
								else
								{
									FI::LOG("error: path not found:", path);
								}
							}
						}
						else
						{
							// individual file to add
							asset.add(str);
						}
					}
					else
					{
						// something else
					}
				}
				else
				{
					// error
					FI::LOG("failed to determin path as file or directory.");
				}
				break;
		}
	});

	FI::PRINT("\nfipack: a tool for packaging multiple assets together in one file.");
	FI::PRINT("C Frustum Interactive 2024");

	switch (mode)
	{
		case HELP_MODE:
			FI::PRINT("\nfipack: a tool for packaging multiple assets together in one file.");
			FI::PRINT("\tusage: fipack <options> value infile1 infile2..infileN");
			FI::PRINT("\toptions: \n\t\t-o <file> output packed archive filename.");
			FI::PRINT("\t\t-e <path> extract all packed files to specified path.");
			FI::PRINT("\t\t-i display contents info of packed input file.");
			FI::PRINT("\n");
			break;

		case INFO_MODE:
			// input is an asset package to display info about
			{
				std::string header = "\t | " + strcell("Path:", asset.longestPathLength()+1) + "| " + strcell("Offset:", 10) + "| " + strcell("Size:", 10) + "|";
				std::string sep = "\t |" + std::string(header.length()-4, '-') + "|"; // subtract to account for spaces inserted by PRINT 
				
				FI::PRINT("\npackage contents:\n");
				FI::PRINT(sep);
				FI::PRINT(header);
				FI::PRINT(sep);
				for (const auto& fileInfo : asset.fileInfoList())
				{
					auto entry = "| " + strcell(fileInfo.path, asset.longestPathLength()+1);
					entry += "| " + strcell(std::to_string(fileInfo.offset), 10) + "| " + strcell(std::to_string(fileInfo.size), 10) + "|";
					FI::PRINT("\t", entry);
				}
				FI::PRINT(sep);
				FI::PRINT("");
			}
			break;

		case EXTRACT_MODE:
			break;
		
		case PACK_MODE:
			asset.write(fname);
			break;

		default:
			break;
	}

	return 0;
}