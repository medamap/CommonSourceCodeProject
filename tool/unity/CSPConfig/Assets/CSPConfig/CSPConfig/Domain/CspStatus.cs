namespace CSPConfig.CSPConfig.Domain
{
    /// <summary>Emulator Configure ステータス</summary>
    public enum CspStatus
    {
        Boot = 0,
        PermissionRequest = 1,
        PermissionDenied = 2,
        PermissionGranted = 3,
        RequestGetConfig = 4,
        NoConfig = 5,
        GetConfig = 6,
        StartCsp = 7,
    }
}
