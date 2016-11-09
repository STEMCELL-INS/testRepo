# include "helper.h"

void getFileList(string folderDir, vector<string>& fileList) 
{
	path p(folderDir);
	for (auto i=directory_iterator(p); i!= directory_iterator(); ++i) {
		if(!is_directory(i->path())) {
			std::string temp_name = i->path().filename().string();
			std::cout << temp_name << "\n";
			fileList.push_back(temp_name);
		}
		else 
			continue;
	}
}

