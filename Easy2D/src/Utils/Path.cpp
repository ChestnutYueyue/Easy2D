#include <easy2d/e2dtool.h>
#include <algorithm>
#include <list>
#include <filesystem>
#include <fstream>
#include <easy2d/portable-file-dialogs.h>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#endif

namespace fs = std::filesystem;

static easy2d::String s_sLocalAppDataPath;
static easy2d::String s_sTempPath;
static easy2d::String s_sDataSavePath;
static std::list<easy2d::String> s_vPathList;

/**
 * @brief 获取本地应用数据目录（跨平台实现）
 */
static easy2d::String GetLocalAppDataPath()
{
#ifdef _WIN32
    char szPath[MAX_PATH];
    HRESULT hr = SHGetFolderPathA(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, szPath);
    if (SUCCEEDED(hr))
    {
        return easy2d::String(szPath);
    }
#elif defined(__APPLE__)
    const char* home = getenv("HOME");
    if (!home)
    {
        struct passwd* pw = getpwuid(getuid());
        home = pw ? pw->pw_dir : "";
    }
    return easy2d::String(home) + "/Library/Application Support";
#else
    // Linux - 使用 XDG 规范
    const char* xdgData = getenv("XDG_DATA_HOME");
    if (xdgData)
    {
        return easy2d::String(xdgData);
    }
    const char* home = getenv("HOME");
    if (!home)
    {
        struct passwd* pw = getpwuid(getuid());
        home = pw ? pw->pw_dir : "";
    }
    return easy2d::String(home) + "/.local/share";
#endif
    return "";
}

/**
 * @brief 获取临时目录（跨平台实现）
 */
static easy2d::String GetTempPath()
{
#ifdef _WIN32
    char path[MAX_PATH];
    if (::GetTempPathA(MAX_PATH, path) != 0)
    {
        return easy2d::String(path);
    }
#else
    const char* tmp = getenv("TMPDIR");
    if (tmp)
    {
        return easy2d::String(tmp);
    }
    return "/tmp";
#endif
    return "";
}

bool easy2d::Path::__init(const String& uniqueName)
{
    s_sLocalAppDataPath = GetLocalAppDataPath();
    if (s_sLocalAppDataPath.empty())
    {
        E2D_WARNING("Get local AppData path failed!");
        return false;
    }

    // 获取数据的默认保存路径
#ifdef _WIN32
    s_sDataSavePath = s_sLocalAppDataPath + "\\Easy2DGameData\\";
#else
    s_sDataSavePath = s_sLocalAppDataPath + "/Easy2DGameData/";
#endif
    if (!uniqueName.empty())
    {
        s_sDataSavePath.append(uniqueName);
#ifdef _WIN32
        s_sDataSavePath.append("\\");
#else
        s_sDataSavePath.append("/");
#endif
    }
    if (!Path::exists(s_sDataSavePath))
    {
        if (!Path::createFolder(s_sDataSavePath))
        {
            s_sDataSavePath = "";
        }
    }
    s_sDataSavePath.append("Data.ini");

    // 获取临时文件目录
    s_sTempPath = GetTempPath();
#ifdef _WIN32
    s_sTempPath.append("\\Easy2DGameTemp\\");
#else
    s_sTempPath.append("/Easy2DGameTemp/");
#endif
    if (!uniqueName.empty())
    {
        s_sTempPath.append(uniqueName);
#ifdef _WIN32
        s_sTempPath.append("\\");
#else
        s_sTempPath.append("/");
#endif
    }

    if (!Path::exists(s_sTempPath))
    {
        if (!Path::createFolder(s_sTempPath))
        {
            s_sTempPath = "";
        }
    }

    return true;
}

void easy2d::Path::add(String path)
{
    // 处理空路径，空路径表示当前目录
    if (path.empty())
    {
        path = ".";
    }

    // 确保路径以分隔符结尾
    if (path.length() > 0 && path[path.length() - 1] != L'\\' && path[path.length() - 1] != L'/')
    {
#ifdef _WIN32
        path.append("\\");
#else
        path.append("/");
#endif
    }
    auto iter = std::find(s_vPathList.cbegin(), s_vPathList.cend(), path);
    if (iter == s_vPathList.cend())
    {
        s_vPathList.push_front(path);
    }
}

easy2d::String easy2d::Path::getTempPath()
{
    return s_sTempPath;
}

easy2d::String easy2d::Path::getExecutableFilePath()
{
#ifdef _WIN32
    char szPath[MAX_PATH] = { 0 };
    if (::GetModuleFileNameA(nullptr, szPath, MAX_PATH) != 0)
    {
        return String(szPath);
    }
#elif defined(__linux__)
    char path[4096];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len != -1)
    {
        path[len] = '\0';
        return String(path);
    }
#elif defined(__APPLE__)
    char path[4096];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) == 0)
    {
        return String(path);
    }
#endif
    return String();
}

easy2d::String easy2d::Path::searchForFile(const String& path)
{
    if (Path::exists(path))
    {
        return path;
    }
    else
    {
        for (auto& resPath : s_vPathList)
        {
            if (Path::exists(resPath + path))
            {
                return resPath + path;
            }
        }
    }
    return String();
}

easy2d::String easy2d::Path::extractResource(int resNameId, const String& resType, const String& destFileName)
{
    String destFilePath = s_sTempPath + destFileName;

    // 使用标准 C++ fstream 替代 Windows API
    std::ofstream file(destFilePath.c_str(), std::ios::binary | std::ios::trunc);
    if (!file.is_open())
    {
        return String();
    }

    // 查找资源文件中、加载资源到内存、得到资源大小
    Resource res{ resNameId, resType };
    auto data = res.loadData();

    if (data.isValid())
    {
        // 写入文件
        file.write(static_cast<const char*>(data.buffer), static_cast<std::streamsize>(data.size));
        file.close();
        return destFilePath;
    }
    else
    {
        file.close();
        // 删除文件
        std::remove(destFilePath.c_str());
        return String();
    }
}

easy2d::String easy2d::Path::getDataSavePath()
{
    return s_sDataSavePath;
}

easy2d::String easy2d::Path::getSaveFilePath(const String& title, const String& defExt)
{
    // 使用 portable-file-dialogs 替代 Windows API
    std::vector<std::string> filters;
    if (!defExt.empty())
    {
        filters.push_back(defExt.c_str());
        filters.push_back("*." + std::string(defExt.c_str()));
    }
    else
    {
        filters.push_back("All Files");
        filters.push_back("*");
    }

    pfd::save_file dialog(
        title.c_str(),
        "",
        filters,
        pfd::opt::force_overwrite
    );

    std::string result = dialog.result();
    return result.empty() ? "" : result.c_str();
}

bool easy2d::Path::createFolder(const String& dirPath)
{
    if (dirPath.empty())
    {
        return false;
    }

    try
    {
        return fs::create_directories(fs::path(dirPath.c_str()));
    }
    catch (const fs::filesystem_error& e)
    {
        E2D_ERROR("Create folder failed: %s", e.what());
        return false;
    }
}

bool easy2d::Path::exists(const String& path)
{
    if (path.empty())
    {
        return false;
    }

    try
    {
        return fs::exists(fs::path(path.c_str()));
    }
    catch (const fs::filesystem_error&)
    {
        return false;
    }
}
