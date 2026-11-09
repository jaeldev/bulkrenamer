#include <iostream>
#include <filesystem>
#include <algorithm>
#include <system_error>
#include <string>
#include <vector>
#include <cctype>
#include <unordered_set>

namespace fs = std::filesystem;

struct FilePair{
        fs::path oldPath;
        fs::path newPath;
        std::string oldName;
        std::string newName;
    };

// Helping function
bool getUserChoice(const std::string& prompt, bool defaultResponse = true){
    while (true){
    	std::string choice;
    	std::cout << prompt << " (y/n): " << std::flush;
    	if (!(std::cin >> choice)) return defaultResponse;    
    	std::transform(choice.begin(), choice.end(), choice.begin(), [](unsigned char c){
    	    return std::tolower(c);
    	});
    	if (choice == "y" || choice == "yes") return true;
    	if (choice == "n" || choice == "no") return false;
    	std::cout << "Invalid choice. Enter 'y' or 'n'.\n";
    }
}

int main(){
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::cout << "Bulk renamer started...\n";
// Enter folder path
    std::string strFolderPath;
    std::cout << "Enter folder path: " << std::flush;
    std::cin >> std::ws; 
    if (!std::getline(std::cin, strFolderPath) || strFolderPath.empty()){
        std::cout << "Folder cannot be empty.\n";
        return 1;
    }
    fs::path folderPath = strFolderPath;
// Verify if the path is valid
    if (!fs::exists(folderPath) || !fs::is_directory(folderPath)){
        std::cerr << "Error: Path does not exist or not a directory.\n";
        return 1;
    }
// [Options]
// 1.Prefix
    std::string prefix;
    if (getUserChoice("Do you want to add a custom prefix?")){
        std::cout << "Enter prefix: " << std::flush;
        std::cin >> prefix;
    }

// 2.Numbering
    bool useNumbering = getUserChoice("Do you want to add sequential numbering?");
    std::cout << "Listing & processing...\n";

// Calculate original and renamed names for preview
	std::cout << "\nPreview:\n";
	std::cout << "\n";
    std::error_code dir_ec;
    std::vector<fs::path> rawFiles;
    int counter = 1;
    for (const auto& item : fs::directory_iterator(folderPath, dir_ec)){
    	if (dir_ec) break;
    	if (!item.is_regular_file()) continue;
        const fs::path& oldFilePath = item.path();
        std::string oldFileName = oldFilePath.filename().string();
// Skip hidden file
        if (!oldFileName.empty() && oldFileName[0] == '.') continue;

		rawFiles.push_back(item.path());

		if (dir_ec){
			std::cerr << "Error: Encounter error while reading: " << strFolderPath << "\n";
		}
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
    
    for (const auto& item : batchList){
        std::cout << item.oldName << " -> " << item.newName << '\n';
    }
    
    if (!getUserChoice("\nDo you want to apply changes?")){
        std::cout << "Canceling...\n";
        return 0;
    }

    std::cout << "Applying changes...\n";
    for (const auto& item : batchList){
        std::error_code ec;
        fs::rename(item.oldPath, item.newPath, ec);
        if (ec) {
            std::cerr << "Fail: " << item.oldName << " -> " << ec.message() << '\n';
        }
    }

    std::cout << "Bulk renamer ended...\n";
    return 0;
}
