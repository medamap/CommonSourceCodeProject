namespace CSPConfig.Config.Domain
{
    public interface IConfigFileHelper
    {
        /// <summary>プロファイル文字列書き込み</summary>
        bool MyWritePrivateProfileString(string lpAppName, string lpKeyName, string lpString, string lpFileName);

        /// <summary>プロファイル文字列読み込み</summary>
        string MyGetPrivateProfileString(string lpAppName, string lpKeyName, string lpDefault, string lpFileName);

        /// <summary>プロファイル数値読み込み</summary>
        int MyGetPrivateProfileInt(string lpAppName, string lpKeyName, int nDefault, string lpFileName);
    }
}
