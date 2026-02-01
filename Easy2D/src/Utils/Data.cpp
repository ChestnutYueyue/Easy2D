#include <easy2d/e2dtool.h>
#include <SimpleIni.h>
#include <mutex>

// 静态全局变量
static CSimpleIniA s_ini;
static std::mutex s_iniMutex;
static bool s_iniLoaded = false;

/**
 * @brief 确保 INI 文件已加载
 */
static void EnsureIniLoaded()
{
    std::lock_guard<std::mutex> lock(s_iniMutex);
    if (s_iniLoaded) return;

    const char* iniPath = easy2d::Path::getDataSavePath().c_str();
    SI_Error rc = s_ini.LoadFile(iniPath);
    if (rc < 0)
    {
        // 文件不存在则创建空配置
        s_ini.Reset();
    }
    s_iniLoaded = true;
}

/**
 * @brief 保存 INI 文件
 */
static void SaveIni()
{
    std::lock_guard<std::mutex> lock(s_iniMutex);
    if (!s_iniLoaded) return;

    const char* iniPath = easy2d::Path::getDataSavePath().c_str();
    s_ini.SaveFile(iniPath);
}

/**
 * @brief 保存整数值
 * @param key 键名
 * @param value 整数值
 * @param field 字段名（节名）
 */
void easy2d::Data::saveInt(const String& key, int value, const String& field)
{
    EnsureIniLoaded();
    std::lock_guard<std::mutex> lock(s_iniMutex);
    s_ini.SetLongValue(field.c_str(), key.c_str(), static_cast<long>(value));
    SaveIni();
}

/**
 * @brief 保存浮点值
 * @param key 键名
 * @param value 浮点值
 * @param field 字段名（节名）
 */
void easy2d::Data::saveDouble(const String& key, float value, const String& field)
{
    EnsureIniLoaded();
    std::lock_guard<std::mutex> lock(s_iniMutex);
    s_ini.SetDoubleValue(field.c_str(), key.c_str(), static_cast<double>(value));
    SaveIni();
}

/**
 * @brief 保存布尔值
 * @param key 键名
 * @param value 布尔值
 * @param field 字段名（节名）
 */
void easy2d::Data::saveBool(const String& key, bool value, const String& field)
{
    EnsureIniLoaded();
    std::lock_guard<std::mutex> lock(s_iniMutex);
    s_ini.SetBoolValue(field.c_str(), key.c_str(), value);
    SaveIni();
}

/**
 * @brief 保存字符串
 * @param key 键名
 * @param value 字符串值
 * @param field 字段名（节名）
 */
void easy2d::Data::saveString(const String& key, const String& value, const String& field)
{
    EnsureIniLoaded();
    std::lock_guard<std::mutex> lock(s_iniMutex);
    s_ini.SetValue(field.c_str(), key.c_str(), value.c_str());
    SaveIni();
}

/**
 * @brief 获取整数值
 * @param key 键名
 * @param defaultValue 默认值
 * @param field 字段名（节名）
 * @return 整数值，不存在则返回默认值
 */
int easy2d::Data::getInt(const String& key, int defaultValue, const String& field)
{
    EnsureIniLoaded();
    std::lock_guard<std::mutex> lock(s_iniMutex);
    return static_cast<int>(s_ini.GetLongValue(
        field.c_str(),
        key.c_str(),
        static_cast<long>(defaultValue)
    ));
}

/**
 * @brief 获取浮点值
 * @param key 键名
 * @param defaultValue 默认值
 * @param field 字段名（节名）
 * @return 浮点值，不存在则返回默认值
 */
float easy2d::Data::getDouble(const String& key, float defaultValue, const String& field)
{
    EnsureIniLoaded();
    std::lock_guard<std::mutex> lock(s_iniMutex);
    return static_cast<float>(s_ini.GetDoubleValue(
        field.c_str(),
        key.c_str(),
        static_cast<double>(defaultValue)
    ));
}

/**
 * @brief 获取布尔值
 * @param key 键名
 * @param defaultValue 默认值
 * @param field 字段名（节名）
 * @return 布尔值，不存在则返回默认值
 */
bool easy2d::Data::getBool(const String& key, bool defaultValue, const String& field)
{
    EnsureIniLoaded();
    std::lock_guard<std::mutex> lock(s_iniMutex);
    return s_ini.GetBoolValue(field.c_str(), key.c_str(), defaultValue);
}

/**
 * @brief 获取字符串
 * @param key 键名
 * @param defaultValue 默认值
 * @param field 字段名（节名）
 * @return 字符串值，不存在则返回默认值
 */
easy2d::String easy2d::Data::getString(const String& key, const String& defaultValue, const String& field)
{
    EnsureIniLoaded();
    std::lock_guard<std::mutex> lock(s_iniMutex);
    const char* value = s_ini.GetValue(field.c_str(), key.c_str(), defaultValue.c_str());
    return value ? value : defaultValue.c_str();
}
