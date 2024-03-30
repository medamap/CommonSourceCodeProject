using System;
using CSPConfig.Config.Domain;
using VContainer.Unity;

namespace CSPConfig.Config.Application
{
    /// <summary>コンフィグサービス</summary>
    // ReSharper disable once ClassNeverInstantiated.Global
    public class ConfigService : IInitializable, IDisposable
    {
        /// <summary>コンフィグプロバイダ</summary>
        private readonly IConfigProvider _configProvider;
        /// <summary>コンフィグ更新プロバイダ</summary>
        private readonly IUpdateConfigProvider _updateConfigProvider;
        
        /// <summary>依存注入</summary>
        public ConfigService(
            IConfigProvider configProvider,
            IUpdateConfigProvider updateConfigProvider
            )
        {
            _configProvider = configProvider;
            _updateConfigProvider = updateConfigProvider;
        }
        
        /// <summary>サービス初期化</summary>
        public void Initialize()
        {
            // コンフィグ更新プロバイダ初期化
            _updateConfigProvider.Initialize();
        }

        /// <summary>コンフィグファイルパス</summary>
        public string ConfigPath => _configProvider.ConfigPath;

        /// <summary>コンフィグファイルの存在チェック</summary>
        public bool IsExistsConfig() => _configProvider.IsExistsConfig();

        /// <summary>コンフィグへ int 値の書き込み</summary>
        public bool WriteConfigInt(string section, string key, int value) => _configProvider.WriteConfigInt(section, key, value);

        /// <summary>コンフィグから int 値の読み込み</summary>
        public int ReadConfigInt(string section, string key) => _configProvider.ReadConfigInt(section, key);
        
        /// <summary>コンフィグへ string 値の書き込み</summary>
        public bool WriteConfigString(string section, string key, string value) => _configProvider.WriteConfigString(section, key, value);
        
        /// <summary>コンフィグから string 値の読み込み</summary>
        public string ReadConfigString(string section, string key) => _configProvider.ReadConfigString(section, key);

        /// <summary>リソース解放</summary>
        public void Dispose()
        {
        }
    }
}