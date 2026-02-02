#include <easy2d/e2dtool.h>
#include <easy2d/SimpleIni.h>
#include <mutex>

// 获取 SimpleIni 实例的辅助函数
// 使用单例模式确保线程安全
static CSimpleIniA& GetIniInstance()
{
	static CSimpleIniA ini;
	static std::once_flag initFlag;
	
	std::call_once(initFlag, [&]() {
		ini.SetUnicode(true);
		// 尝试加载现有文件
		ini.LoadFile(easy2d::Path::getDataSavePath().c_str());
	});
	
	return ini;
}

// 保存数据到文件的辅助函数
static void SaveIniData()
{
	CSimpleIniA& ini = GetIniInstance();
	ini.SaveFile(easy2d::Path::getDataSavePath().c_str());
}


/**
 * @brief 保存 int 类型的值到 INI 文件
 * @param key 键值
 * @param value 要保存的整数值
 * @param field 字段名称（节名），默认为 "Defalut"
 */
void easy2d::Data::saveInt(const String& key, int value, const String& field)
{
	CSimpleIniA& ini = GetIniInstance();
	ini.SetLongValue(field.c_str(), key.c_str(), value);
	SaveIniData();
}

/**
 * @brief 保存 float 类型的值到 INI 文件
 * @param key 键值
 * @param value 要保存的浮点数值
 * @param field 字段名称（节名），默认为 "Defalut"
 */
void easy2d::Data::saveDouble(const String& key, float value, const String& field)
{
	CSimpleIniA& ini = GetIniInstance();
	ini.SetDoubleValue(field.c_str(), key.c_str(), value);
	SaveIniData();
}

/**
 * @brief 保存 bool 类型的值到 INI 文件
 * @param key 键值
 * @param value 要保存的布尔值
 * @param field 字段名称（节名），默认为 "Defalut"
 */
void easy2d::Data::saveBool(const String& key, bool value, const String& field)
{
	CSimpleIniA& ini = GetIniInstance();
	ini.SetBoolValue(field.c_str(), key.c_str(), value);
	SaveIniData();
}

/**
 * @brief 保存字符串类型的值到 INI 文件
 * @param key 键值
 * @param value 要保存的字符串值
 * @param field 字段名称（节名），默认为 "Defalut"
 */
void easy2d::Data::saveString(const String& key, const String& value, const String& field)
{
	CSimpleIniA& ini = GetIniInstance();
	ini.SetValue(field.c_str(), key.c_str(), value.c_str());
	SaveIniData();
}

/**
 * @brief 从 INI 文件获取 int 类型的值
 * @param key 键值
 * @param defaultValue 默认值，当键不存在时返回此值
 * @param field 字段名称（节名），默认为 "Defalut"
 * @return 返回键对应的整数值，若不存在则返回 defaultValue
 */
int easy2d::Data::getInt(const String& key, int defaultValue, const String& field)
{
	CSimpleIniA& ini = GetIniInstance();
	return static_cast<int>(ini.GetLongValue(field.c_str(), key.c_str(), defaultValue));
}

/**
 * @brief 从 INI 文件获取 float 类型的值
 * @param key 键值
 * @param defaultValue 默认值，当键不存在时返回此值
 * @param field 字段名称（节名），默认为 "Defalut"
 * @return 返回键对应的浮点数值，若不存在则返回 defaultValue
 */
float easy2d::Data::getDouble(const String& key, float defaultValue, const String& field)
{
	CSimpleIniA& ini = GetIniInstance();
	return static_cast<float>(ini.GetDoubleValue(field.c_str(), key.c_str(), defaultValue));
}

/**
 * @brief 从 INI 文件获取 bool 类型的值
 * @param key 键值
 * @param defaultValue 默认值，当键不存在时返回此值
 * @param field 字段名称（节名），默认为 "Defalut"
 * @return 返回键对应的布尔值，若不存在则返回 defaultValue
 * @note SimpleIni 识别以下值为 true: "t", "y", "on", "1"
 *       识别以下值为 false: "f", "n", "of", "0"
 */
bool easy2d::Data::getBool(const String& key, bool defaultValue, const String& field)
{
	CSimpleIniA& ini = GetIniInstance();
	return ini.GetBoolValue(field.c_str(), key.c_str(), defaultValue);
}

/**
 * @brief 从 INI 文件获取字符串类型的值
 * @param key 键值
 * @param defaultValue 默认值，当键不存在时返回此值
 * @param field 字段名称（节名），默认为 "Defalut"
 * @return 返回键对应的字符串值，若不存在则返回 defaultValue
 */
easy2d::String easy2d::Data::getString(const String& key, const String& defaultValue, const String& field)
{
	CSimpleIniA& ini = GetIniInstance();
	const char* value = ini.GetValue(field.c_str(), key.c_str(), defaultValue.c_str());
	return value ? value : defaultValue;
}
