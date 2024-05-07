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

int main(int argc, char *argv[])
{
	FI::Asset asset; // for one-by-one manaully adding individual files to it
	std::string fname = "package.fas";
	std::string extractPath="extracted";
	bool isExtractMode = false;

	enum ArgType // <0 is switch, >0 has val that follows, 0 no op.
	{
		TEST = -2,
		HELP,
		NONE,
		OUTPUT,
		EXTRACT
	};

	FI::UTIL::CmdLineArgMap argMap =
	{
		{"-o", OUTPUT},
		{"-e", EXTRACT},
		{"-t", TEST},
		{"-h", HELP}
	};

	FI::UTIL::processCmdLineArgs(argc, argv, argMap, [&](int arg, std::string str)
	{
		bool showHelp = false;
		switch (arg)
		{
			case HELP:
				showHelp = true;
				break;

			case OUTPUT:
				fname = str;
				break;

			case EXTRACT:
				isExtractMode = true;
				std::filesystem::create_directory(str);
				removeTrailingSlash(str);
				extractPath = str + "/";
				break;

			case TEST:
				break;

			case NONE:
				if (str == FI_CMD_OPT_NONE)
				{
					showHelp = true;
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
						if (isExtractMode)
						{
							FI::LOG("error: cannot extract directories.");
							showHelp = true;
							break;
						}
						// remove trailing slash if its there
						removeTrailingSlash(str);
						std::vector<std::string> files;
						FI::UTIL::listFiles(str, [&](const std::string &p)
						{
							files.push_back(p);
						});
						for (auto &file : files)
						{
							asset.add(file); // rebuild relative path
						}
					}
					else if (s.st_mode & S_IFREG)
					{
						// it's a file
						if (isExtractMode)
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

		if (showHelp)
		{
			FI::PRINT("\nfipack: a tool for packaging multiple assets together in one file.");
			FI::PRINT("\tusage: fipack -<options> value infile1 infile2..infileN");
			FI::PRINT("\toptions: \n\t-o output file name\n");
			exit(0);
		}
	});

	if (!isExtractMode)
	{
		asset.write(fname);
	}

	return 0;
}