#include <easy2d/e2dtool.h>
#include <easy2d/SimpleIni.h>
#include <mutex>
#include <shared_mutex>

// 读写锁，用于保护 SimpleIni 的并发访问
static std::shared_mutex s_iniMutex;

// 获取 SimpleIni 实例的辅助函数
// 使用单例模式确保线程安全
static CSimpleIniA& GetIniInstance()
{
	static CSimpleIniA ini;
	static std::once_flag initFlag;

	std::call_once(initFlag, [&]() {
		ini.SetUnicode(true);
		// 尝试加载现有文件，忽略错误（首次运行时文件可能不存在）
		SI_Error rc = ini.LoadFile(easy2d::Path::getDataSavePath().c_str());
		if (rc < 0) {
			// 文件不存在或其他错误，使用空配置
			E2D_WARNING("Failed to load INI file, starting with empty configuration");
		}
	});

	return ini;
}

// 保存数据到文件的辅助函数
// @return 保存成功返回 true，失败返回 false
static bool SaveIniData()
{
	CSimpleIniA& ini = GetIniInstance();
	SI_Error rc = ini.SaveFile(easy2d::Path::getDataSavePath().c_str());
	if (rc < 0) {
		E2D_ERROR("Failed to save INI file to: %s", easy2d::Path::getDataSavePath().c_str());
		return false;
	}
	return true;
}


/**
 * @brief 保存 int 类型的值到 INI 文件
 * @param key 键值
 * @param value 要保存的整数值
 * @param field 字段名称（节名），默认为 "Default"
 * @return 保存成功返回 true，失败返回 false
 */
bool easy2d::Data::saveInt(const String& key, int value, const String& field)
{
	std::unique_lock<std::shared_mutex> lock(s_iniMutex);
	CSimpleIniA& ini = GetIniInstance();
	ini.SetLongValue(field.c_str(), key.c_str(), value);
	return SaveIniData();
}

/**
 * @brief 保存 float 类型的值到 INI 文件
 * @param key 键值
 * @param value 要保存的浮点数值
 * @param field 字段名称（节名），默认为 "Default"
 * @return 保存成功返回 true，失败返回 false
 */
bool easy2d::Data::saveFloat(const String& key, float value, const String& field)
{
	std::unique_lock<std::shared_mutex> lock(s_iniMutex);
	CSimpleIniA& ini = GetIniInstance();
	ini.SetDoubleValue(field.c_str(), key.c_str(), static_cast<double>(value));
	return SaveIniData();
}

/**
 * @brief 保存 double 类型的值到 INI 文件
 * @param key 键值
 * @param value 要保存的双精度浮点数值
 * @param field 字段名称（节名），默认为 "Default"
 * @return 保存成功返回 true，失败返回 false
 */
bool easy2d::Data::saveDouble(const String& key, double value, const String& field)
{
	std::unique_lock<std::shared_mutex> lock(s_iniMutex);
	CSimpleIniA& ini = GetIniInstance();
	ini.SetDoubleValue(field.c_str(), key.c_str(), value);
	return SaveIniData();
}

/**
 * @brief 保存 bool 类型的值到 INI 文件
 * @param key 键值
 * @param value 要保存的布尔值
 * @param field 字段名称（节名），默认为 "Default"
 * @return 保存成功返回 true，失败返回 false
 */
bool easy2d::Data::saveBool(const String& key, bool value, const String& field)
{
	std::unique_lock<std::shared_mutex> lock(s_iniMutex);
	CSimpleIniA& ini = GetIniInstance();
	ini.SetBoolValue(field.c_str(), key.c_str(), value);
	return SaveIniData();
}

/**
 * @brief 保存字符串类型的值到 INI 文件
 * @param key 键值
 * @param value 要保存的字符串值
 * @param field 字段名称（节名），默认为 "Default"
 * @return 保存成功返回 true，失败返回 false
 */
bool easy2d::Data::saveString(const String& key, const String& value, const String& field)
{
	std::unique_lock<std::shared_mutex> lock(s_iniMutex);
	CSimpleIniA& ini = GetIniInstance();
	ini.SetValue(field.c_str(), key.c_str(), value.c_str());
	return SaveIniData();
}

/**
 * @brief 从 INI 文件获取 int 类型的值
 * @param key 键值
 * @param defaultValue 默认值，当键不存在时返回此值
 * @param field 字段名称（节名），默认为 "Default"
 * @return 返回键对应的整数值，若不存在则返回 defaultValue
 */
int easy2d::Data::getInt(const String& key, int defaultValue, const String& field)
{
	std::shared_lock<std::shared_mutex> lock(s_iniMutex);
	CSimpleIniA& ini = GetIniInstance();
	return static_cast<int>(ini.GetLongValue(field.c_str(), key.c_str(), defaultValue));
}

/**
 * @brief 从 INI 文件获取 float 类型的值
 * @param key 键值
 * @param defaultValue 默认值，当键不存在时返回此值
 * @param field 字段名称（节名），默认为 "Default"
 * @return 返回键对应的浮点数值，若不存在则返回 defaultValue
 */
float easy2d::Data::getFloat(const String& key, float defaultValue, const String& field)
{
	std::shared_lock<std::shared_mutex> lock(s_iniMutex);
	CSimpleIniA& ini = GetIniInstance();
	return static_cast<float>(ini.GetDoubleValue(field.c_str(), key.c_str(), defaultValue));
}

/**
 * @brief 从 INI 文件获取 double 类型的值
 * @param key 键值
 * @param defaultValue 默认值，当键不存在时返回此值
 * @param field 字段名称（节名），默认为 "Default"
 * @return 返回键对应的双精度浮点数值，若不存在则返回 defaultValue
 */
double easy2d::Data::getDouble(const String& key, double defaultValue, const String& field)
{
	std::shared_lock<std::shared_mutex> lock(s_iniMutex);
	CSimpleIniA& ini = GetIniInstance();
	return ini.GetDoubleValue(field.c_str(), key.c_str(), defaultValue);
}

/**
 * @brief 从 INI 文件获取 bool 类型的值
 * @param key 键值
 * @param defaultValue 默认值，当键不存在时返回此值
 * @param field 字段名称（节名），默认为 "Default"
 * @return 返回键对应的布尔值，若不存在则返回 defaultValue
 * @note SimpleIni 识别以下值为 true: "t", "y", "on", "1"
 *       识别以下值为 false: "f", "n", "of", "0"
 */
bool easy2d::Data::getBool(const String& key, bool defaultValue, const String& field)
{
	std::shared_lock<std::shared_mutex> lock(s_iniMutex);
	CSimpleIniA& ini = GetIniInstance();
	return ini.GetBoolValue(field.c_str(), key.c_str(), defaultValue);
}

/**
 * @brief 从 INI 文件获取字符串类型的值
 * @param key 键值
 * @param defaultValue 默认值，当键不存在时返回此值
 * @param field 字段名称（节名），默认为 "Default"
 * @return 返回键对应的字符串值，若不存在则返回 defaultValue
 */
easy2d::String easy2d::Data::getString(const String& key, const String& defaultValue, const String& field)
{
	std::shared_lock<std::shared_mutex> lock(s_iniMutex);
	CSimpleIniA& ini = GetIniInstance();
	const char* value = ini.GetValue(field.c_str(), key.c_str(), defaultValue.c_str());
	return value ? value : defaultValue;
}
