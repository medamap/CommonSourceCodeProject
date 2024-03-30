using UnityEngine;
using CSPConfig.Config.Domain;
using CSPConfig.Info.Domain;
using MessagePipe;

namespace CSPConfig.Config.Infrastructure
{
    // ReSharper disable once ClassNeverInstantiated.Global
    public class ConfigProvider : IConfigProvider
    {
        /// <summary>コンフィグファイルヘルパー</summary>
        private readonly IConfigFileHelper _configFileHelper;
        /// <summary>お知らせメッセージ送信</summary>
        private readonly IPublisher<InfoMessage> _infoMessagePublisher;
        
        /// <summary>コンフィグファイルパス</summary>
        public string ConfigPath => _configPath;
        private string _configPath = "";

        /// <summary>依存注入</summary>
        public ConfigProvider(
            IConfigFileHelper configHelper,
            IPublisher<InfoMessage> infoMessagePublisher
            )
        {
            _configFileHelper = configHelper;
            _infoMessagePublisher = infoMessagePublisher;
        }
        
        /// <summary>コンフィグファイルの存在チェック</summary>
        public bool IsExistsConfig()
        {
            using (var javaUnityPlayer = new AndroidJavaClass("com.unity3d.player.UnityPlayer")) {
                var currentActivity = javaUnityPlayer.GetStatic<AndroidJavaObject>("currentActivity");
                using (var permissionManager = new AndroidJavaClass("jp.megamin.android.cspconfig.CspConfig"))
                {
                    var resultPath = permissionManager.CallStatic<string>("getSdcardDownloadPath", currentActivity);
                    _configPath = resultPath + "/emulator/config.ini";
                }
            }
            return System.IO.File.Exists(_configPath);
        }

        /// <summary>コンフィグへ int 値の書き込み</summary>
        public bool WriteConfigInt(string section, string key, int value)
        {
            Debug.Log($"WriteConfigInt({section}, {key}, {value})");
            _infoMessagePublisher.Publish(InfoMessage.GetInfoMessage($"WriteConfigInt({section}, {key}, {value})"));
            return _configFileHelper.MyWritePrivateProfileString(section, key, value.ToString(), _configPath);
        }

        /// <summary>コンフィグから int 値の読み込み</summary>
        public int ReadConfigInt(string section, string key)
        {
            var value = _configFileHelper.MyGetPrivateProfileInt(section, key, 0, _configPath);
            Debug.Log($"ReadConfigInt({section}, {key}) = {value}");
            return value;
        }

        /// <summary>コンフィグへ string 値の書き込み</summary>
        public bool WriteConfigString(string section, string key, string value)
        {   
            Debug.Log($"WriteConfigString({section}, {key}, {value})");
            _infoMessagePublisher.Publish(InfoMessage.GetInfoMessage($"WriteConfigString({section}, {key}, {value})"));
            return _configFileHelper.MyWritePrivateProfileString(section, key, value, _configPath);
        }

        /// <summary>コンフィグから string 値の読み込み</summary>
        public string ReadConfigString(string section, string key)
        {
            var value = _configFileHelper.MyGetPrivateProfileString(section, key, "", _configPath);
            Debug.Log($"ReadConfigString({section}, {key}) = {value}");
            return value;
        }
    }
}
