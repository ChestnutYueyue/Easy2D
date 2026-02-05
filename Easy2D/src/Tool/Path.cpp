#include <algorithm>
#include <easy2d/core/e2dtool.h>
#include <filesystem> // C++17 filesystem
#include <list>
#include <path/portable-file-dialogs.h>
#include <shlobj.h> // SHGetFolderPathA, CSIDL_LOCAL_APPDATA

static easy2d::String s_sLocalAppDataPath;
static easy2d::String s_sTempPath;
static easy2d::String s_sDataSavePath;
static std::list<easy2d::String> s_vPathList;
// 文件路径缓存，避免重复搜索和磁盘 I/O
static std::map<easy2d::String, easy2d::String> s_filePathCache;

bool easy2d::Path::__init(const String &uniqueName) {
  char szPath[MAX_PATH];
  HRESULT hr =
      SHGetFolderPathA(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, szPath);

  if (SUCCEEDED(hr)) {
    s_sLocalAppDataPath = szPath;
  } else {
    E2D_WARNING("Get local AppData path failed!");
    return false;
  }

  // 获取数据的默认保存路径
  s_sDataSavePath = s_sLocalAppDataPath + "\\Easy2DGameData\\";
  if (!uniqueName.empty()) {
    s_sDataSavePath.append(uniqueName).append("\\");
  }
  if (!Path::exists(s_sDataSavePath)) {
    if (!Path::createFolder(s_sDataSavePath)) {
      s_sDataSavePath = "";
    }
  }
  s_sDataSavePath.append("Data.ini");

  // 获取临时文件目录
  char path[_MAX_PATH];
  if (0 == ::GetTempPathA(_MAX_PATH, path)) {
    return false;
  }

  s_sTempPath.append(path).append("\\Easy2DGameTemp\\");
  if (!uniqueName.empty()) {
    s_sTempPath.append(uniqueName).append("\\");
  }

  if (!Path::exists(s_sTempPath)) {
    if (!Path::createFolder(s_sTempPath)) {
      s_sTempPath = "";
    }
  }

  return true;
}

void easy2d::Path::add(String path) {
  // 检查路径是否为空，避免访问空字符串导致越界
  if (!path.empty() && path[path.length() - 1] != '\\' &&
      path[path.length() - 1] != '/') {
    path.append("\\");
  }
  auto iter = std::find(s_vPathList.cbegin(), s_vPathList.cend(), path);
  if (iter == s_vPathList.cend()) {
    s_vPathList.push_front(path);
    // 添加新路径后，清空文件缓存（因为搜索优先级可能改变）
    s_filePathCache.clear();
  }
}

easy2d::String easy2d::Path::getTempPath() { return s_sTempPath; }

easy2d::String easy2d::Path::getExecutableFilePath() {
  char szPath[_MAX_PATH] = {0};
  if (::GetModuleFileNameA(nullptr, szPath, _MAX_PATH) != 0) {
    return String(szPath);
  }
  return String();
}

easy2d::String easy2d::Path::searchForFile(const String &path) {
  // 先检查缓存
  auto cacheIter = s_filePathCache.find(path);
  if (cacheIter != s_filePathCache.end()) {
    // 缓存命中，验证文件是否仍然存在
    if (Path::exists(cacheIter->second)) {
      return cacheIter->second;
    }
    // 文件已不存在，从缓存中移除
    s_filePathCache.erase(cacheIter);
  }

  // 缓存未命中，执行搜索
  String fullPath;
  if (Path::exists(path)) {
    fullPath = path;
  } else {
    for (auto &resPath : s_vPathList) {
      String candidatePath = resPath + path;
      if (Path::exists(candidatePath)) {
        fullPath = candidatePath;
        break;
      }
    }
  }

  // 将结果存入缓存（即使是空字符串也缓存，表示文件不存在）
  if (!fullPath.empty()) {
    s_filePathCache[path] = fullPath;
  }

  return fullPath;
}

easy2d::String easy2d::Path::extractResource(int resNameId,
                                             const String &resType,
                                             const String &destFileName) {
  String destFilePath = s_sTempPath + destFileName;
  HANDLE hFile =
      ::CreateFileA(destFilePath.c_str(), GENERIC_WRITE, 0, nullptr,
                    CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, nullptr);
  if (hFile == INVALID_HANDLE_VALUE)
    return String();

  // 查找资源文件中、加载资源到内存、得到资源大小
  Resource res{resNameId, resType};
  auto data = res.loadData();

  if (data.isValid()) {
    // 写入文件
    DWORD dwWrite = 0;
    ::WriteFile(hFile, data.buffer, (DWORD)data.size, &dwWrite, NULL);
    ::CloseHandle(hFile);
    return destFilePath;
  } else {
    ::CloseHandle(hFile);
    ::DeleteFileA(destFilePath.c_str());
    return String();
  }
}

easy2d::String easy2d::Path::getDataSavePath() { return s_sDataSavePath; }

/**
 * @brief 获取保存文件路径（使用 portable-file-dialogs）
 * @param title 对话框标题
 * @param defExt 默认扩展名
 * @return 用户选择的文件路径，取消则返回空字符串
 */
easy2d::String easy2d::Path::getSaveFilePath(const String &title,
                                             const String &defExt) {
  // 使用 portable-file-dialogs 创建保存文件对话框
  pfd::opt options = pfd::opt::none;

  // 构建过滤器列表
  std::vector<std::string> filters;
  if (!defExt.empty()) {
    // 如果有默认扩展名，添加对应的过滤器
    filters.push_back(defExt + " Files");
    filters.push_back("*." + std::string(defExt.c_str()));
  }
  filters.push_back("All Files");
  filters.push_back("*");

  // 创建保存文件对话框
  pfd::save_file dialog(title.empty() ? "Save File" : title.c_str(),
                        "", // 默认路径
                        filters, options);

  // 获取结果
  std::string result = dialog.result();

  // 如果用户选择了文件，自动添加默认扩展名（如果没有扩展名）
  if (!result.empty() && !defExt.empty()) {
    // 检查是否已有扩展名
    size_t dotPos = result.rfind('.');
    size_t sepPos = result.rfind('\\');
    size_t slashPos = result.rfind('/');
    size_t lastSep = std::string::npos;
    if (sepPos == std::string::npos) {
      lastSep = slashPos;
    } else if (slashPos == std::string::npos) {
      lastSep = sepPos;
    } else {
      lastSep = (sepPos > slashPos) ? sepPos : slashPos;
    }

    // 如果没有扩展名或扩展名部分为空
    if (dotPos == std::string::npos || dotPos < lastSep) {
      result += ".";
      result += defExt.c_str();
    }
  }

  return result.empty() ? String() : String(result.c_str());
}

/**
 * @brief 创建文件夹（递归创建所有父目录）
 * @param dirPath 目录路径
 * @return 创建成功返回 true，失败返回 false
 */
bool easy2d::Path::createFolder(const String &dirPath) {
  if (dirPath.empty()) {
    return false;
  }

  try {
    std::filesystem::path path(dirPath.c_str());
    // create_directories 会递归创建所有不存在的父目录
    return std::filesystem::create_directories(path);
  } catch (const std::filesystem::filesystem_error &) {
    // 如果目录已存在，create_directories 返回 false，但这不算错误
    // 所以再检查一次目录是否存在
    return exists(dirPath);
  }
}

/**
 * @brief 检查路径是否存在（文件或目录）
 * @param path 文件或目录路径
 * @return 存在返回 true，不存在返回 false
 */
bool easy2d::Path::exists(const String &path) {
  if (path.empty()) {
    return false;
  }

  try {
    std::filesystem::path fsPath(path.c_str());
    return std::filesystem::exists(fsPath);
  } catch (const std::filesystem::filesystem_error &) {
    return false;
  }
}
