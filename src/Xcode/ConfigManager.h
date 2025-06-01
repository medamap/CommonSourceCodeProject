/*
    ConfigManager.h
    ユニバーサル設定管理システム (UserDefaults + INI ハイブリッド)
    
    Author: Medamap and Claude
    Date: 2025.05.30
    
    Features:
    - UserDefaults（Apple）とINIファイル（クロスプラットフォーム）の両方をサポート
    - 音声設定のリアルタイム適用
    - 設定変更の自動検出と通知システム
    - iOS/macOS固有の最適化
*/

#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#ifdef __APPLE__
#ifdef __OBJC__
#import <Foundation/Foundation.h>
@class NSUserDefaults;
#else
// C++ファイルではObjective-Cヘッダーを直接インクルードしない
// 代わりにvoidポインタで処理
#endif
#endif

#include <string>
#include <unordered_map>
#include <functional>

// 設定変更通知用コールバック型定義
typedef std::function<void(const std::string& key, const std::string& old_value, const std::string& new_value)> ConfigChangeCallback;

// 設定データ型
enum ConfigValueType {
    CONFIG_TYPE_INTEGER,
    CONFIG_TYPE_FLOAT,
    CONFIG_TYPE_BOOLEAN,
    CONFIG_TYPE_STRING
};

// 設定値格納構造体
struct ConfigValue {
    ConfigValueType type;
    std::string string_value;
    int int_value;
    float float_value;
    bool bool_value;
    
    ConfigValue() : type(CONFIG_TYPE_STRING), string_value(""), int_value(0), float_value(0.0f), bool_value(false) {}
    
    explicit ConfigValue(int value) : type(CONFIG_TYPE_INTEGER), int_value(value), float_value(0.0f), bool_value(false) {
        string_value = std::to_string(value);
    }
    
    explicit ConfigValue(float value) : type(CONFIG_TYPE_FLOAT), float_value(value), int_value(0), bool_value(false) {
        string_value = std::to_string(value);
    }
    
    explicit ConfigValue(bool value) : type(CONFIG_TYPE_BOOLEAN), bool_value(value), int_value(value ? 1 : 0), float_value(0.0f) {
        string_value = value ? "true" : "false";
    }
    
    explicit ConfigValue(const std::string& value) : type(CONFIG_TYPE_STRING), string_value(value), int_value(0), float_value(0.0f), bool_value(false) {}
    
    // 比較演算子（ソート用）
    bool operator<(const ConfigValue& other) const {
        return string_value < other.string_value;
    }
};

class ConfigManager {
public:
    // シングルトンパターン
    static ConfigManager& getInstance();
    
    // 初期化・終了処理
    void initialize(const std::string& app_name = "CSCPEmulator", const std::string& ini_path = "");
    void initializeForMachine(const std::string& machine_name); // 機種別パス自動生成版
    void shutdown();
    
    // 設定値の取得
    int getInt(const std::string& key, int default_value = 0);
    float getFloat(const std::string& key, float default_value = 0.0f);
    bool getBool(const std::string& key, bool default_value = false);
    std::string getString(const std::string& key, const std::string& default_value = "");
    
    // 設定値の設定
    void setInt(const std::string& key, int value);
    void setFloat(const std::string& key, float value);
    void setBool(const std::string& key, bool value);
    void setString(const std::string& key, const std::string& value);
    
    // 音声設定専用メソッド（リアルタイム適用）
    void setSoundFrequency(int frequency);
    void setSoundLatency(int latency);
    void setSoundVolume(int channel, int volume_l, int volume_r);
    void setSoundEnabled(bool enabled);
    
    int getSoundFrequency();
    int getSoundLatency();
    int getSoundVolumeL(int channel);
    int getSoundVolumeR(int channel);
    bool getSoundEnabled();
    
    // 設定変更通知システム
    void registerChangeCallback(const std::string& key, ConfigChangeCallback callback);
    void unregisterChangeCallback(const std::string& key);
    
    // ファイル操作
    bool loadFromINI(const std::string& file_path = "");
    bool saveToINI(const std::string& file_path = "");
    
#ifdef __APPLE__
    // Apple固有機能
    bool loadFromUserDefaults();
    bool saveToUserDefaults();
    void syncWithUserDefaults(); // UserDefaultsとINIの同期
#endif
    
    // デバッグ・状態確認
    void printAllSettings();
    bool hasKey(const std::string& key);
    void removeKey(const std::string& key);
    void clearAll();
    
    // 設定プロファイル機能
    void saveProfile(const std::string& profile_name);
    bool loadProfile(const std::string& profile_name);
    std::vector<std::string> getAvailableProfiles();
    
private:
    ConfigManager();
    ~ConfigManager();
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    
    // 内部データ
    std::unordered_map<std::string, ConfigValue> settings;
    std::unordered_map<std::string, ConfigChangeCallback> change_callbacks;
    std::string app_name;
    std::string ini_file_path;
    bool initialized;
    
    // 内部ヘルパーメソッド
    std::string selectOrCreateEmulatorFolder();
    std::string getDefaultEmulatorPath();
    void createDirectoryIfNotExists(const std::string& path);
    
#ifdef __APPLE__
    void* user_defaults; // NSUserDefaults*のvoidポインタ（ARC対応）
#endif
    
    // 内部ヘルパーメソッド
    void setValue(const std::string& key, const ConfigValue& value);
    ConfigValue getValue(const std::string& key, const ConfigValue& default_value);
    void notifyChangeCallback(const std::string& key, const std::string& old_value, const std::string& new_value);
    void setDefaultValues();
    void syncWithGlobalConfig();
    
    // INIファイル解析
    bool parseINILine(const std::string& line, std::string& key, std::string& value);
    std::string escapeINIValue(const std::string& value);
    std::string unescapeINIValue(const std::string& value);
    
    // プラットフォーム固有のパス取得
    std::string getDefaultINIPath();
    std::string getProfilePath(const std::string& profile_name);
    
#ifdef __APPLE__
    // UserDefaults変換ヘルパー
    std::string userDefaultsKeyForConfigKey(const std::string& config_key);
    void syncKeyWithUserDefaults(const std::string& key, bool to_userdefaults);
#endif
};

// C言語インターフェース（既存コードとの互換性のため）
extern "C" {
    void config_manager_init(const char* app_name, const char* ini_path);
    void config_manager_shutdown(void);
    
    int config_get_int(const char* key, int default_value);
    void config_set_int(const char* key, int value);
    
    int config_get_bool(const char* key, int default_value);
    void config_set_bool(const char* key, int value);
    
    const char* config_get_string(const char* key, const char* default_value);
    void config_set_string(const char* key, const char* value);
    
    void config_save_to_ini(const char* file_path);
    void config_load_from_ini(const char* file_path);
    
#ifdef __APPLE__
    void config_save_to_userdefaults(void);
    void config_load_from_userdefaults(void);
#endif
}

// Android版メニューシステムとの互換性マクロ
#define CONFIG_SOUND_FREQUENCY_KEY "sound_frequency"
#define CONFIG_SOUND_LATENCY_KEY "sound_latency"
#define CONFIG_SOUND_ENABLED_KEY "sound_enabled"
#define CONFIG_SOUND_VOLUME_L_PREFIX "sound_volume_l_"
#define CONFIG_SOUND_VOLUME_R_PREFIX "sound_volume_r_"

#define CONFIG_WINDOW_MODE_KEY "window_mode"
#define CONFIG_WINDOW_STRETCH_TYPE_KEY "window_stretch_type"
#define CONFIG_FULLSCREEN_STRETCH_TYPE_KEY "fullscreen_stretch_type"
#define CONFIG_ROTATE_TYPE_KEY "rotate_type"
#define CONFIG_FILTER_TYPE_KEY "filter_type"

#define CONFIG_CPU_POWER_KEY "cpu_power"
#define CONFIG_FULL_SPEED_KEY "full_speed"
#define CONFIG_DRIVE_VM_IN_OPECODE_KEY "drive_vm_in_opecode"

#endif // CONFIGMANAGER_H