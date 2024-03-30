namespace CSPConfig.JavaApp.Domain
{
    /// <summary>権限プロバイダ</summary>
    public interface IPermissionProvider
    {
        /// <summary>権限チェックとリクエスト</summary>
        bool CheckAndRequestPermission();
    }
}
