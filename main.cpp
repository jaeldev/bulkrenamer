#include <iostream>
#include <filesystem>
#include <algorithm>
#include <system_error>
#include <string>
#include <vector>
#include <unordered_set>
#include <cstdio>
#include <cstdlib>

#ifndef VERSION_STRING
#define VERSION_STRING "2.0.0"
#endif

namespace fs = std::filesystem;

struct FilePair{
	fs::path oldPath;
	fs::path newPath;
	std::string oldName;
	std::string newName;
};

// Helping function
static void printUsage(std::ostream& out){
	out << "Usage: bulkrenamer --dir <path> [--prefix <text>] [--number] [--execute]\n"
	"Options:\n"
	"-d, --dir <path>     Folder with files to rename (required)\n"
	"-p, --prefix <text>  Add <text> to start of each name\n"
	"-n, --number         Add sequential numbering (001, 002, ...)\n"
	"-e, --execute        Apply the changes (default is preview only)\n"
	"-v, --version        Show version\n"
	"-h, --help           Show this help\n";
}

int main(int argc, char* argv[]){
    if (argc <= 1){
		printUsage(std::cerr);
        return 1;
   	}
   	
// Enter folder path and prefix
    fs::path folderPath;
    std::string prefix;
    bool useNumbering = false;
    bool preview = true;
// Loop through arguments for flags
    for (int i = 1; i < argc; i++){
    	std::string arg = argv[i];
    		if (arg == "--help" || arg == "-h"){
    			printUsage(std::cout);
    			return 0;
    		}
    		else if (arg == "--version" || arg == "-v"){
    			std::cout << "bulkrenamer " << VERSION_STRING << "\n";
    			return 0;
    		}
    		else if ((arg == "--dir" || arg == "-d") && i + 1 < argc){
    		folderPath = argv[++i];
// Handle when user give path with '~' in quotes to make full path
    		std::string pathStr = folderPath.string();
    		const char* homeDir = std::getenv("HOME");
    		if (homeDir && !pathStr.empty() && pathStr[0] == '~') {
    			if (pathStr == "~"){
    				folderPath = homeDir;
    			}
    			else if (pathStr.rfind("~/", 0) == 0){
    				folderPath = fs::path(homeDir) / pathStr.substr(2);
    			}
    		}
    	}
    	else if ((arg == "--prefix" || arg == "-p") && i + 1 < argc){
    		prefix = argv[++i];
    	}
    	else if (arg == "--number" || arg == "-n"){
    		useNumbering = true;
    	}
    	else if (arg == "--execute" || arg == "-e"){
    		preview = false;
    	}
    	else{
// Handle unknown arguments to prevent crash
    		std::cerr << "Error: Unknown or incomplete flag '" << arg << "'\n";
    		printUsage(std::cerr);
    		return 1;
    	}
    }

    if (folderPath.empty()){
    	std::cerr << "Error: --dir <path> is required.\n";
    	printUsage(std::cerr);
    	return 1;
    }
    
    std::error_code check_ec;
    if (!fs::is_directory(folderPath, check_ec)){
    	std::cerr << "Error: Path does not exist or is not a directory.\n";
    	return 1;
    }
    
// Collect regular and not hidden files
    std::error_code dir_ec;
    std::vector<fs::path> rawFiles;
    fs::directory_iterator it(folderPath, dir_ec);
    const fs::directory_iterator end;
    for (; !dir_ec && it != end; it.increment(dir_ec)){
    	std::error_code type_ec;
    	if (!it->is_regular_file(type_ec)) continue;
        std::string name = it->path().filename().string();
// Skip hidden file
        if (!name.empty() && name[0] == '.') continue;

		rawFiles.push_back(it->path());
    }

    if (dir_ec){
        std::cerr << "Error: Encounter error while reading: " << folderPath.string() << "\n";
        return 1;
    }

    int counter = 1;

    if (rawFiles.empty()){
    	std::cout << "No files to rename.\n";
    	return 0;
    }

    std::sort(rawFiles.begin(), rawFiles.end());
    
    std::vector<FilePair> batchList;
    for (const auto& oldFilePath : rawFiles){
    	std::string oldFileName = oldFilePath.filename().string();
    	std::string fileName = oldFilePath.stem().string();
    	std::string extension = oldFilePath.extension().string();
// Replace with underscore, add prefix, and numbering
		std::replace(fileName.begin(), fileName.end(), ' ', '_');

		std::string newFileName;
		newFileName.reserve(prefix.size() + fileName.size() + extension.size() + 8);
// Adjust file name according to user option
		if (!prefix.empty()){
			newFileName += prefix + "_";
		}

		if (useNumbering){
			char numBuf[12];
			snprintf(numBuf, sizeof(numBuf), "%03d_", counter++);
			newFileName += numBuf;
		}

		newFileName += fileName + extension;
		fs::path newFilePath = oldFilePath.parent_path() / newFileName;

// Prevent string reallocation
		batchList.push_back({oldFilePath, std::move(newFilePath), std::move(oldFileName), std::move(newFileName)});
    }
    
// Check and fix file name collision
    	std::unordered_set<std::string> usedPaths;
    	usedPaths.reserve(batchList.size() * 2);
    	for (auto& item : batchList){
    		fs::path basePath = item.newPath;
    		std::string stem = basePath.stem().string();
    		std::string ext = basePath.extension().string();
    		int collisionCounter = 1;
    		while ((fs::exists(item.newPath) && item.oldPath != item.newPath) || usedPaths.count(item.newPath.string()) > 0){
    			std::string newFileName = stem + "_" + std::to_string(collisionCounter++) + ext;
    			item.newPath = basePath.parent_path() / newFileName;
    			item.newName = newFileName;
    		}
    		usedPaths.insert(item.newPath.string());
    	}
	
    std::cout << "Preview:\n";
    for (const auto& item : batchList){
        std::cout << item.oldName << " -> " << item.newName << '\n';
    }

    if (preview){
    	std::cout << "Did not write changes to disk. Pls. write flag '--execute' or '-e' in order to add changes.\n";
    	return 0;
    }

    std::size_t renamed = 0;
    for (const auto& item : batchList){
        if (item.oldPath == item.newPath) continue;
        
        std::error_code ec;
        fs::rename(item.oldPath, item.newPath, ec);
        if (ec) {
            std::cerr << "\nFail: " << item.oldName << " -> " << ec.message() << '\n';
            std::cerr << "Stopping after renaming " << renamed << " of " << batchList.size() << " file(s).\n";
            return 1;
        }
        renamed++;
    }

    return 0;
}
