#ifndef TOOLIO_H
#define TOOLIO_H



template <typename T>
std::ostream& operator<< (std::ostream& out, const std::vector<T>& v){
    out << "[";  for (const T& i : v){ out << i << " ";  } out << "]";
    return out;
}

template <typename T>
std::ostream& operator<< (std::ostream& out, const std::set<T>& v){
    out << "[";  for (const T& i : v){ out << i << " ";  } out << "]";
    return out;
}

void CheckOutputFolder(const char* folder_path){

    // std::filesystem::path dirPath(std::filesystem::path(file_path).parent_path().string());
    // if (!std::filesystem::exists(dirPath)) {
    //     if (std::filesystem::create_directories(dirPath)) {
    //         std::cout << "Directory created: " << folder_path << std::endl;
    //         return true;
    //     } else {
    //         std::cerr << "Failed to create directory: " << folder_path << std::endl;
    //         return false;
    //     }
    // }
    std::cout << "Checking output folder for " << folder_path << std::endl;
    if(!gSystem->AccessPathName(folder_path)){
        std::cout << "Folder not found, creating it ...\n";
        if (!gSystem->mkdir(folder_path, true)) {
            std::cout << "Directory created: " << folder_path << std::endl;
        } else {
            std::cerr << "Failed to create directory: " << folder_path << std::endl;
        }
    }

}
#endif