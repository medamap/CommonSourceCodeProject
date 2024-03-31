namespace CSPConfig.Config.Domain
{
    /// <summary>コンフィグプロバイダ</summary>
    public interface IConfigProvider
    {
        /// <summary>コンフィグファイルパス</summary>
        string ConfigPath { get; }
        
        /// <summary>コンフィグファイルの存在チェック</summary>
        bool IsExistsConfig();
        
        /// <summary>コンフィグへ int 値の書き込み</summary>
        bool WriteConfigInt(string section, string key, int value);

        /// <summary>コンフィグから int 値の読み込み</summary>
        int ReadConfigInt(string section, string key);
        
        /// <summary>コンフィグへ string 値の書き込み</summary>
        bool WriteConfigString(string section, string key, string value);
        
        /// <summary>コンフィグから string 値の読み込み</summary>
        string ReadConfigString(string section, string key);
    }
}
