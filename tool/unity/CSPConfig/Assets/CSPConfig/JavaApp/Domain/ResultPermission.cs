namespace CSPConfig.JavaApp.Domain
{
    /// <summary>
    /// 権限チェックとリクエスト結果
    /// </summary>
    public struct ResultPermission {
        /// <summary>権限が付与されたか</summary>
        public bool IsGranted { get; }

        /// <summary>権限チェック時にエラーが発生したか</summary>
        public string ErrorMessage { get; }

        /// <summary>コンストラクタ</summary>
        private ResultPermission(bool isGranted, string errorMessage) {
            IsGranted = isGranted;
            ErrorMessage = errorMessage;
        }

        /// <summary>権限が付与されたか</summary>
        public static ResultPermission GetResultPermission(bool isGranted) {
            return new ResultPermission(isGranted, null);
        }
        
        /// <summary>権限チェック時にエラーが発生したか</summary>
        public static ResultPermission GetErrorResultPermission(string errorMessage) {
            return new ResultPermission(false, errorMessage);
        }
    }
}
