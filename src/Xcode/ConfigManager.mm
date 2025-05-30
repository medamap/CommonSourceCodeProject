/*
    ConfigManager.mm
    ユニバーサル設定管理システム実装 (UserDefaults + INI ハイブリッド)
    
    Author: Medamap and Claude
    Date: 2025.05.30
*/

#include "ConfigManager.h"
#include "osd.h"
#include "../config.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>

#ifdef __APPLE__
#import <Foundation/Foundation.h>
#if TARGET_OS_OSX
#import <AppKit/AppKit.h>
#elif TARGET_OS_IOS
#import <UIKit/UIKit.h>
#endif
#endif

// 外部参照
extern OSD* osd;
extern config_t config;

////////////////////////////////////////////////////////////////////////////////
// ConfigManagerシングルトン実装
////////////////////////////////////////////////////////////////////////////////

ConfigManager& ConfigManager::getInstance() {
    static ConfigManager instance;
    return instance;
}

ConfigManager::ConfigManager() : initialized(false) {
#ifdef __APPLE__
    user_defaults = nullptr;
#endif
}

ConfigManager::~ConfigManager() {
    if (initialized) {
        shutdown();
    }
}

void ConfigManager::initialize(const std::string& app_name, const std::string& ini_path) {
    if (initialized) {
        printf("[ConfigManager] 既に初期化済みです\n");
        return;
    }
    
    this->app_name = app_name.empty() ? "CSCPEmulator" : app_name;
    this->ini_file_path = ini_path.empty() ? getDefaultINIPath() : ini_path;
    
    printf("[ConfigManager] 初期化中: app=%s, ini=%s\n", this->app_name.c_str(), this->ini_file_path.c_str());
    
    // まず適切なデフォルト値を設定
    setDefaultValues();
    
#ifdef __APPLE__
    // UserDefaultsの初期化（ARC対応）
    user_defaults = (__bridge void*)([NSUserDefaults standardUserDefaults]);
    
    // 既存のUserDefaults設定を読み込み
    loadFromUserDefaults();
#endif
    
    // INIファイルから設定を読み込み（UserDefaultsより優先）
    bool ini_loaded = loadFromINI();
    
    if (!ini_loaded) {
        printf("[ConfigManager] INIファイルが見つからないため、デフォルト設定を使用\n");
        // 初回起動時：デフォルト値をINIファイルに保存
        saveToINI();
#ifdef __APPLE__
        saveToUserDefaults();
#endif
    }
    
    // config.hグローバル変数にも反映
    syncWithGlobalConfig();
    
    initialized = true;
    printf("[ConfigManager] 初期化完了\n");
}

void ConfigManager::shutdown() {
    if (!initialized) {
        return;
    }
    
    printf("[ConfigManager] 終了処理中...\n");
    
    // 現在の設定を保存
    saveToINI();
#ifdef __APPLE__
    saveToUserDefaults();
#endif
    
    // コールバックをクリア
    change_callbacks.clear();
    settings.clear();
    
#ifdef __APPLE__
    // ARCが有効なのでリリースは不要
    user_defaults = nullptr;
#endif
    
    initialized = false;
    printf("[ConfigManager] 終了処理完了\n");
}

////////////////////////////////////////////////////////////////////////////////
// 設定値の取得・設定
////////////////////////////////////////////////////////////////////////////////

int ConfigManager::getInt(const std::string& key, int default_value) {
    ConfigValue default_val(default_value);
    ConfigValue result = getValue(key, default_val);
    return result.int_value;
}

float ConfigManager::getFloat(const std::string& key, float default_value) {
    ConfigValue default_val(default_value);
    ConfigValue result = getValue(key, default_val);
    return result.float_value;
}

bool ConfigManager::getBool(const std::string& key, bool default_value) {
    ConfigValue default_val(default_value);
    ConfigValue result = getValue(key, default_val);
    return result.bool_value;
}

std::string ConfigManager::getString(const std::string& key, const std::string& default_value) {
    ConfigValue default_val(default_value);
    ConfigValue result = getValue(key, default_val);
    return result.string_value;
}

void ConfigManager::setInt(const std::string& key, int value) {
    ConfigValue config_val(value);
    setValue(key, config_val);
}

void ConfigManager::setFloat(const std::string& key, float value) {
    ConfigValue config_val(value);
    setValue(key, config_val);
}

void ConfigManager::setBool(const std::string& key, bool value) {
    ConfigValue config_val(value);
    setValue(key, config_val);
}

void ConfigManager::setString(const std::string& key, const std::string& value) {
    ConfigValue config_val(value);
    setValue(key, config_val);
}

////////////////////////////////////////////////////////////////////////////////
// 音声設定専用メソッド（リアルタイム適用）
////////////////////////////////////////////////////////////////////////////////

void ConfigManager::setSoundFrequency(int frequency) {
    printf("[ConfigManager] 音声周波数設定: %dHz\n", frequency);
    
    // 周波数値をインデックスに変換
    int frequencies[] = {2000, 4000, 8000, 11025, 22050, 44100, 62500, 96000};
    int freq_index = 5;  // デフォルト: 44100Hz
    
    for (int i = 0; i < 8; i++) {
        if (frequencies[i] == frequency) {
            freq_index = i;
            break;
        }
    }
    
    int old_frequency_index = getSoundFrequency();
    setInt(CONFIG_SOUND_FREQUENCY_KEY, freq_index);
    
    // config.hの設定も更新（インデックス）
    config.sound_frequency = freq_index;
    
    printf("[ConfigManager] 周波数インデックス設定: %d → %d\n", old_frequency_index, freq_index);
    
    // リアルタイムでサウンドシステムを再初期化
    if (osd && old_frequency_index != freq_index) {
        osd->reinitialize_sound_if_needed(frequency, osd->sound_samples);
    }
    
#ifdef __APPLE__
    syncKeyWithUserDefaults(CONFIG_SOUND_FREQUENCY_KEY, true);
#endif
}

void ConfigManager::setSoundLatency(int latency) {
    printf("[ConfigManager] 音声レイテンシ設定: %dms\n", latency);
    
    // latencyの値をインデックスに変換
    int latencies[] = {50, 100, 200, 300, 400};
    int latency_index = 1;  // デフォルト: 100ms
    
    for (int i = 0; i < 5; i++) {
        if (latencies[i] == latency) {
            latency_index = i;
            break;
        }
    }
    
    setInt(CONFIG_SOUND_LATENCY_KEY, latency_index);
    config.sound_latency = latency_index;
    
    printf("[ConfigManager] レイテンシインデックス設定: %d (%dms)\n", latency_index, latency);
    
#ifdef __APPLE__
    syncKeyWithUserDefaults(CONFIG_SOUND_LATENCY_KEY, true);
#endif
}

void ConfigManager::setSoundVolume(int channel, int volume_l, int volume_r) {
    printf("[ConfigManager] 音量設定: ch%d L=%d R=%d\n", channel, volume_l, volume_r);
    
    std::string key_l = CONFIG_SOUND_VOLUME_L_PREFIX + std::to_string(channel);
    std::string key_r = CONFIG_SOUND_VOLUME_R_PREFIX + std::to_string(channel);
    
    setInt(key_l, volume_l);
    setInt(key_r, volume_r);
    
#ifdef USE_SOUND_VOLUME
    if (channel >= 0 && channel < MAX_VOLUME_TMP) {
        config.sound_volume_l[channel] = volume_l;
        config.sound_volume_r[channel] = volume_r;
    }
#endif
    
#ifdef __APPLE__
    syncKeyWithUserDefaults(key_l, true);
    syncKeyWithUserDefaults(key_r, true);
#endif
}

void ConfigManager::setSoundEnabled(bool enabled) {
    printf("[ConfigManager] 音声有効設定: %s\n", enabled ? "有効" : "無効");
    
    setBool(CONFIG_SOUND_ENABLED_KEY, enabled);
    
    if (osd) {
        osd->soundEnable = enabled;
        if (enabled) {
            osd->start_sound();
        } else {
            osd->stop_sound();
        }
    }
    
#ifdef __APPLE__
    syncKeyWithUserDefaults(CONFIG_SOUND_ENABLED_KEY, true);
#endif
}

int ConfigManager::getSoundFrequency() {
    return getInt(CONFIG_SOUND_FREQUENCY_KEY, 5);  // デフォルトはインデックス5 (44100Hz)
}

int ConfigManager::getSoundLatency() {
    return getInt(CONFIG_SOUND_LATENCY_KEY, 1);  // デフォルトはインデックス1 (100ms)
}

int ConfigManager::getSoundVolumeL(int channel) {
    std::string key = CONFIG_SOUND_VOLUME_L_PREFIX + std::to_string(channel);
    return getInt(key, 100);
}

int ConfigManager::getSoundVolumeR(int channel) {
    std::string key = CONFIG_SOUND_VOLUME_R_PREFIX + std::to_string(channel);
    return getInt(key, 100);
}

bool ConfigManager::getSoundEnabled() {
    return getBool(CONFIG_SOUND_ENABLED_KEY, true);
}

////////////////////////////////////////////////////////////////////////////////
// 設定変更通知システム
////////////////////////////////////////////////////////////////////////////////

void ConfigManager::registerChangeCallback(const std::string& key, ConfigChangeCallback callback) {
    change_callbacks[key] = callback;
    printf("[ConfigManager] コールバック登録: %s\n", key.c_str());
}

void ConfigManager::unregisterChangeCallback(const std::string& key) {
    change_callbacks.erase(key);
    printf("[ConfigManager] コールバック削除: %s\n", key.c_str());
}

////////////////////////////////////////////////////////////////////////////////
// ファイル操作（INI）
////////////////////////////////////////////////////////////////////////////////

bool ConfigManager::loadFromINI(const std::string& file_path) {
    std::string path = file_path.empty() ? ini_file_path : file_path;
    
    std::ifstream file(path);
    if (!file.is_open()) {
        printf("[ConfigManager] INIファイルが見つかりません: %s\n", path.c_str());
        return false;
    }
    
    printf("[ConfigManager] INIファイルから読み込み中: %s\n", path.c_str());
    
    std::string line;
    int line_number = 0;
    
    while (std::getline(file, line)) {
        line_number++;
        
        // コメント行と空行をスキップ
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }
        
        std::string key, value;
        if (parseINILine(line, key, value)) {
            // 値をConfigValueに変換して保存
            std::string unescaped = unescapeINIValue(value);
            
            // 型を推定して適切なConfigValueを作成
            if (unescaped == "true" || unescaped == "false") {
                setBool(key, unescaped == "true");
            } else if (unescaped.find('.') != std::string::npos) {
                try {
                    float f_val = std::stof(unescaped);
                    setFloat(key, f_val);
                } catch (...) {
                    setString(key, unescaped);
                }
            } else {
                try {
                    int i_val = std::stoi(unescaped);
                    setInt(key, i_val);
                } catch (...) {
                    setString(key, unescaped);
                }
            }
        } else {
            printf("[ConfigManager] 無効なINI行 %d: %s\n", line_number, line.c_str());
        }
    }
    
    file.close();
    printf("[ConfigManager] INIファイル読み込み完了: %zu項目\n", settings.size());
    return true;
}

bool ConfigManager::saveToINI(const std::string& file_path) {
    std::string path = file_path.empty() ? ini_file_path : file_path;
    
    std::ofstream file(path);
    if (!file.is_open()) {
        printf("[ConfigManager] INIファイル書き込みエラー: %s\n", path.c_str());
        return false;
    }
    
    printf("[ConfigManager] INIファイルに保存中: %s\n", path.c_str());
    
    // ヘッダーコメント
    file << "# CommonSourceCodeProject Emulator Configuration\n";
    file << "# Generated by ConfigManager on " << __DATE__ << " " << __TIME__ << "\n";
    file << "# Do not edit this file directly while the emulator is running\n\n";
    
    // 設定をソートして保存（見やすくするため）
    std::vector<std::pair<std::string, ConfigValue>> sorted_settings(settings.begin(), settings.end());
    std::sort(sorted_settings.begin(), sorted_settings.end());
    
    for (const auto& pair : sorted_settings) {
        const std::string& key = pair.first;
        const ConfigValue& value = pair.second;
        
        std::string escaped_value = escapeINIValue(value.string_value);
        file << key << "=" << escaped_value << "\n";
    }
    
    file.close();
    printf("[ConfigManager] INIファイル保存完了: %zu項目\n", settings.size());
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// Apple固有機能（UserDefaults）
////////////////////////////////////////////////////////////////////////////////

#ifdef __APPLE__
bool ConfigManager::loadFromUserDefaults() {
    if (!user_defaults) {
        return false;
    }
    
    NSUserDefaults* defaults = (__bridge NSUserDefaults*)(user_defaults);
    printf("[ConfigManager] UserDefaultsから読み込み中...\n");
    
    // 音声設定（古い値を無視して新しいデフォルト値を使用）
    // 以前の実装では周波数値が保存されていたが、現在はインデックスを使用
    // UserDefaultsの古い値は無視する
    printf("[ConfigManager] UserDefaults音声設定は無視（新しいINI形式を優先）\n");
    
    // 旧音声設定も無視（INI形式を優先）
    printf("[ConfigManager] 旧UserDefaults設定を無視してINI/デフォルト値を使用\n");
    
    // 画面設定
    if ([defaults objectForKey:@CONFIG_WINDOW_MODE_KEY]) {
        config.window_mode = (int)[defaults integerForKey:@CONFIG_WINDOW_MODE_KEY];
        setInt(CONFIG_WINDOW_MODE_KEY, config.window_mode);
    }
    
    if ([defaults objectForKey:@CONFIG_ROTATE_TYPE_KEY]) {
        config.rotate_type = (int)[defaults integerForKey:@CONFIG_ROTATE_TYPE_KEY];
        setInt(CONFIG_ROTATE_TYPE_KEY, config.rotate_type);
    }
    
    // 制御設定
    if ([defaults objectForKey:@CONFIG_CPU_POWER_KEY]) {
        config.cpu_power = (int)[defaults integerForKey:@CONFIG_CPU_POWER_KEY];
        setInt(CONFIG_CPU_POWER_KEY, config.cpu_power);
    }
    
    if ([defaults objectForKey:@CONFIG_FULL_SPEED_KEY]) {
        config.full_speed = [defaults boolForKey:@CONFIG_FULL_SPEED_KEY];
        setBool(CONFIG_FULL_SPEED_KEY, config.full_speed);
    }
    
    printf("[ConfigManager] UserDefaults読み込み完了\n");
    return true;
}

bool ConfigManager::saveToUserDefaults() {
    if (!user_defaults) {
        return false;
    }
    
    NSUserDefaults* defaults = (__bridge NSUserDefaults*)(user_defaults);
    printf("[ConfigManager] UserDefaultsに保存中...\n");
    
    // 全ての設定をUserDefaultsに保存
    for (const auto& pair : settings) {
        const std::string& key = pair.first;
        const ConfigValue& value = pair.second;
        
        NSString* nsKey = [NSString stringWithUTF8String:key.c_str()];
        
        switch (value.type) {
            case CONFIG_TYPE_INTEGER:
                [defaults setInteger:value.int_value forKey:nsKey];
                break;
                
            case CONFIG_TYPE_FLOAT:
                [defaults setFloat:value.float_value forKey:nsKey];
                break;
                
            case CONFIG_TYPE_BOOLEAN:
                [defaults setBool:value.bool_value forKey:nsKey];
                break;
                
            case CONFIG_TYPE_STRING:
                [defaults setObject:[NSString stringWithUTF8String:value.string_value.c_str()] forKey:nsKey];
                break;
        }
    }
    
    [defaults synchronize];
    printf("[ConfigManager] UserDefaults保存完了: %zu項目\n", settings.size());
    return true;
}

void ConfigManager::syncWithUserDefaults() {
    // 双方向同期：UserDefaultsとINIの最新値を比較して統合
    if (!user_defaults) {
        return;
    }
    
    printf("[ConfigManager] UserDefaultsとの同期中...\n");
    
    // まずUserDefaultsから読み込み
    loadFromUserDefaults();
    
    // 次にINIファイルから読み込み（より新しい可能性があるため）
    loadFromINI();
    
    // 最後にUserDefaultsに保存（統合結果を反映）
    saveToUserDefaults();
    
    printf("[ConfigManager] UserDefaults同期完了\n");
}

void ConfigManager::syncKeyWithUserDefaults(const std::string& key, bool to_userdefaults) {
    if (!user_defaults) {
        return;
    }
    
    NSUserDefaults* defaults = (__bridge NSUserDefaults*)(user_defaults);
    NSString* nsKey = [NSString stringWithUTF8String:key.c_str()];
    
    if (to_userdefaults) {
        // ConfigManagerからUserDefaultsへ
        auto it = settings.find(key);
        if (it != settings.end()) {
            const ConfigValue& value = it->second;
            
            switch (value.type) {
                case CONFIG_TYPE_INTEGER:
                    [defaults setInteger:value.int_value forKey:nsKey];
                    break;
                case CONFIG_TYPE_FLOAT:
                    [defaults setFloat:value.float_value forKey:nsKey];
                    break;
                case CONFIG_TYPE_BOOLEAN:
                    [defaults setBool:value.bool_value forKey:nsKey];
                    break;
                case CONFIG_TYPE_STRING:
                    [defaults setObject:[NSString stringWithUTF8String:value.string_value.c_str()] forKey:nsKey];
                    break;
            }
            [defaults synchronize];
        }
    } else {
        // UserDefaultsからConfigManagerへ
        if ([defaults objectForKey:nsKey]) {
            // NSObjectの型を判定して適切に設定
            id obj = [defaults objectForKey:nsKey];
            if ([obj isKindOfClass:[NSNumber class]]) {
                NSNumber* num = (NSNumber*)obj;
                if (strcmp([num objCType], @encode(BOOL)) == 0) {
                    setBool(key, [num boolValue]);
                } else if (strcmp([num objCType], @encode(float)) == 0 || strcmp([num objCType], @encode(double)) == 0) {
                    setFloat(key, [num floatValue]);
                } else {
                    setInt(key, [num intValue]);
                }
            } else if ([obj isKindOfClass:[NSString class]]) {
                setString(key, [(NSString*)obj UTF8String]);
            }
        }
    }
}

std::string ConfigManager::userDefaultsKeyForConfigKey(const std::string& config_key) {
    // 必要に応じてキー名を変換（例：プレフィックス追加）
    return app_name + "." + config_key;
}
#endif

////////////////////////////////////////////////////////////////////////////////
// デバッグ・状態確認
////////////////////////////////////////////////////////////////////////////////

void ConfigManager::printAllSettings() {
    printf("\n[ConfigManager] 全設定一覧 (%zu項目):\n", settings.size());
    printf("=====================================\n");
    
    for (const auto& pair : settings) {
        const std::string& key = pair.first;
        const ConfigValue& value = pair.second;
        
        printf("  %s = ", key.c_str());
        
        switch (value.type) {
            case CONFIG_TYPE_INTEGER:
                printf("%d (int)\n", value.int_value);
                break;
            case CONFIG_TYPE_FLOAT:
                printf("%.3f (float)\n", value.float_value);
                break;
            case CONFIG_TYPE_BOOLEAN:
                printf("%s (bool)\n", value.bool_value ? "true" : "false");
                break;
            case CONFIG_TYPE_STRING:
                printf("\"%s\" (string)\n", value.string_value.c_str());
                break;
        }
    }
    
    printf("=====================================\n\n");
}

bool ConfigManager::hasKey(const std::string& key) {
    return settings.find(key) != settings.end();
}

void ConfigManager::removeKey(const std::string& key) {
    settings.erase(key);
    change_callbacks.erase(key);
    printf("[ConfigManager] キー削除: %s\n", key.c_str());
}

void ConfigManager::clearAll() {
    settings.clear();
    change_callbacks.clear();
    printf("[ConfigManager] 全設定をクリアしました\n");
}

////////////////////////////////////////////////////////////////////////////////
// 設定プロファイル機能
////////////////////////////////////////////////////////////////////////////////

void ConfigManager::saveProfile(const std::string& profile_name) {
    std::string profile_path = getProfilePath(profile_name);
    saveToINI(profile_path);
    printf("[ConfigManager] プロファイル保存: %s\n", profile_name.c_str());
}

bool ConfigManager::loadProfile(const std::string& profile_name) {
    std::string profile_path = getProfilePath(profile_name);
    bool result = loadFromINI(profile_path);
    if (result) {
        printf("[ConfigManager] プロファイル読み込み: %s\n", profile_name.c_str());
    }
    return result;
}

std::vector<std::string> ConfigManager::getAvailableProfiles() {
    std::vector<std::string> profiles;
    // TODO: ディレクトリスキャンを実装
    return profiles;
}

////////////////////////////////////////////////////////////////////////////////
// 内部ヘルパーメソッド
////////////////////////////////////////////////////////////////////////////////

void ConfigManager::setValue(const std::string& key, const ConfigValue& value) {
    std::string old_value;
    auto it = settings.find(key);
    if (it != settings.end()) {
        old_value = it->second.string_value;
    }
    
    settings[key] = value;
    
    // 変更通知
    if (old_value != value.string_value) {
        notifyChangeCallback(key, old_value, value.string_value);
    }
}

ConfigValue ConfigManager::getValue(const std::string& key, const ConfigValue& default_value) {
    auto it = settings.find(key);
    if (it != settings.end()) {
        return it->second;
    }
    return default_value;
}

void ConfigManager::notifyChangeCallback(const std::string& key, const std::string& old_value, const std::string& new_value) {
    auto it = change_callbacks.find(key);
    if (it != change_callbacks.end() && it->second) {
        it->second(key, old_value, new_value);
    }
}

bool ConfigManager::parseINILine(const std::string& line, std::string& key, std::string& value) {
    size_t eq_pos = line.find('=');
    if (eq_pos == std::string::npos) {
        return false;
    }
    
    key = line.substr(0, eq_pos);
    value = line.substr(eq_pos + 1);
    
    // 前後の空白を削除
    key.erase(0, key.find_first_not_of(" \t"));
    key.erase(key.find_last_not_of(" \t") + 1);
    value.erase(0, value.find_first_not_of(" \t"));
    value.erase(value.find_last_not_of(" \t") + 1);
    
    return !key.empty();
}

std::string ConfigManager::escapeINIValue(const std::string& value) {
    std::string result = value;
    // 特殊文字のエスケープ
    std::replace(result.begin(), result.end(), '\n', '\\');
    std::replace(result.begin(), result.end(), '\r', '\\');
    return result;
}

std::string ConfigManager::unescapeINIValue(const std::string& value) {
    std::string result = value;
    // エスケープ文字の復元
    size_t pos = 0;
    while ((pos = result.find("\\n", pos)) != std::string::npos) {
        result.replace(pos, 2, "\n");
        pos += 1;
    }
    pos = 0;
    while ((pos = result.find("\\r", pos)) != std::string::npos) {
        result.replace(pos, 2, "\r");
        pos += 1;
    }
    return result;
}

std::string ConfigManager::getDefaultINIPath() {
#ifdef __APPLE__
    // Documentsディレクトリに保存
    NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString* documentsDirectory = [paths objectAtIndex:0];
    NSString* iniPath = [documentsDirectory stringByAppendingPathComponent:[NSString stringWithFormat:@"%s.ini", app_name.c_str()]];
    return std::string([iniPath UTF8String]);
#else
    return app_name + ".ini";
#endif
}

std::string ConfigManager::getProfilePath(const std::string& profile_name) {
#ifdef __APPLE__
    NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString* documentsDirectory = [paths objectAtIndex:0];
    NSString* profilePath = [documentsDirectory stringByAppendingPathComponent:[NSString stringWithFormat:@"%s_%s.ini", app_name.c_str(), profile_name.c_str()]];
    return std::string([profilePath UTF8String]);
#else
    return app_name + "_" + profile_name + ".ini";
#endif
}

void ConfigManager::setDefaultValues() {
    printf("[ConfigManager] デフォルト値を設定中...\n");
    
    // 音声設定のデフォルト値
    setInt(CONFIG_SOUND_FREQUENCY_KEY, 5);      // インデックス5 = 44.1kHz
    setInt(CONFIG_SOUND_LATENCY_KEY, 1);        // インデックス1 = 100ms
    setBool(CONFIG_SOUND_ENABLED_KEY, true);    // 音声有効
    
    // 音量設定のデフォルト値（各チャンネル）
    for (int i = 0; i < 8; i++) {
        std::string key_l = CONFIG_SOUND_VOLUME_L_PREFIX + std::to_string(i);
        std::string key_r = CONFIG_SOUND_VOLUME_R_PREFIX + std::to_string(i);
        setInt(key_l, 100);  // 100%
        setInt(key_r, 100);  // 100%
    }
    
    // 画面設定のデフォルト値
    setInt(CONFIG_WINDOW_MODE_KEY, 0);              // ウィンドウモード
    setInt(CONFIG_WINDOW_STRETCH_TYPE_KEY, 0);      // 標準拡大
    setInt(CONFIG_FULLSCREEN_STRETCH_TYPE_KEY, 0);  // 標準拡大
    setInt(CONFIG_ROTATE_TYPE_KEY, 0);              // 回転なし
    setInt(CONFIG_FILTER_TYPE_KEY, 0);              // フィルターなし
    
    // 制御設定のデフォルト値
    setInt(CONFIG_CPU_POWER_KEY, 0);                // CPU x1
    setBool(CONFIG_FULL_SPEED_KEY, false);          // フルスピード無効
    setBool(CONFIG_DRIVE_VM_IN_OPECODE_KEY, false); // 通常実行
    
    printf("[ConfigManager] デフォルト値設定完了\n");
}

void ConfigManager::syncWithGlobalConfig() {
    printf("[ConfigManager] グローバル設定と同期中...\n");
    
    // 音声設定（インデックス形式）
    config.sound_frequency = getInt(CONFIG_SOUND_FREQUENCY_KEY, 5);  // インデックス5 = 44100Hz
    config.sound_latency = getInt(CONFIG_SOUND_LATENCY_KEY, 1);      // インデックス1 = 100ms
    
    // 画面設定
    config.window_mode = getInt(CONFIG_WINDOW_MODE_KEY, 0);
    config.window_stretch_type = getInt(CONFIG_WINDOW_STRETCH_TYPE_KEY, 0);
    config.fullscreen_stretch_type = getInt(CONFIG_FULLSCREEN_STRETCH_TYPE_KEY, 0);
    config.rotate_type = getInt(CONFIG_ROTATE_TYPE_KEY, 0);
    
    // 制御設定
    config.cpu_power = getInt(CONFIG_CPU_POWER_KEY, 0);
    config.full_speed = getBool(CONFIG_FULL_SPEED_KEY, false);
    config.drive_vm_in_opecode = getBool(CONFIG_DRIVE_VM_IN_OPECODE_KEY, false);
    
#ifdef USE_SOUND_VOLUME
    // 音量設定
    for (int i = 0; i < MAX_VOLUME_TMP && i < 8; i++) {
        config.sound_volume_l[i] = getSoundVolumeL(i);
        config.sound_volume_r[i] = getSoundVolumeR(i);
    }
#endif

#ifdef USE_SCREEN_FILTER
    config.filter_type = getInt(CONFIG_FILTER_TYPE_KEY, 0);
#endif

#if defined(__ANDROID__)
    config.sound_on = getBool(CONFIG_SOUND_ENABLED_KEY, true);
#endif
    
    // 周波数インデックスを実際の周波数に変換して表示
    int frequencies[] = {2000, 4000, 8000, 11025, 22050, 44100, 62500, 96000};
    int actual_frequency = (config.sound_frequency >= 0 && config.sound_frequency < 8) ? 
                          frequencies[config.sound_frequency] : 44100;
    
    printf("[ConfigManager] グローバル設定同期完了: 音声=インデックス%d(%dHz), CPU=%d\n", 
           config.sound_frequency, actual_frequency, config.cpu_power);
}

////////////////////////////////////////////////////////////////////////////////
// C言語インターフェース
////////////////////////////////////////////////////////////////////////////////

extern "C" {
    void config_manager_init(const char* app_name, const char* ini_path) {
        std::string app_str = app_name ? std::string(app_name) : "";
        std::string ini_str = ini_path ? std::string(ini_path) : "";
        ConfigManager::getInstance().initialize(app_str, ini_str);
    }
    
    void config_manager_shutdown(void) {
        ConfigManager::getInstance().shutdown();
    }
    
    int config_get_int(const char* key, int default_value) {
        return ConfigManager::getInstance().getInt(std::string(key), default_value);
    }
    
    void config_set_int(const char* key, int value) {
        ConfigManager::getInstance().setInt(std::string(key), value);
    }
    
    int config_get_bool(const char* key, int default_value) {
        return ConfigManager::getInstance().getBool(std::string(key), default_value != 0) ? 1 : 0;
    }
    
    void config_set_bool(const char* key, int value) {
        ConfigManager::getInstance().setBool(std::string(key), value != 0);
    }
    
    const char* config_get_string(const char* key, const char* default_value) {
        static std::string result;
        std::string default_str = default_value ? std::string(default_value) : "";
        result = ConfigManager::getInstance().getString(std::string(key), default_str);
        return result.c_str();
    }
    
    void config_set_string(const char* key, const char* value) {
        std::string value_str = value ? std::string(value) : "";
        ConfigManager::getInstance().setString(std::string(key), value_str);
    }
    
    void config_save_to_ini(const char* file_path) {
        std::string path_str = file_path ? std::string(file_path) : "";
        ConfigManager::getInstance().saveToINI(path_str);
    }
    
    void config_load_from_ini(const char* file_path) {
        std::string path_str = file_path ? std::string(file_path) : "";
        ConfigManager::getInstance().loadFromINI(path_str);
    }
    
#ifdef __APPLE__
    void config_save_to_userdefaults(void) {
        ConfigManager::getInstance().saveToUserDefaults();
    }
    
    void config_load_from_userdefaults(void) {
        ConfigManager::getInstance().loadFromUserDefaults();
    }
#endif
}