namespace CSPConfig.JavaApp.Domain
{
    /// <summary>権限取得リクエスト</summary>
    public struct RequestPermission
    {
        public int Dummy { get; }
        public RequestPermission(int dummy)
        {
            Dummy = dummy;
        }
        public static RequestPermission GetRequestPermission() => new RequestPermission(0);
    }
}
