using CSPConfig.Config.Domain;

namespace CSPConfig.Config.Infrastructure
{
    /// <summary>コンフィグ更新プロバイダ</summary>
    // ReSharper disable once ClassNeverInstantiated.Global
    public class UpdateConfigProvider : IUpdateConfigProvider
    {
        /// <summary>コンフィグプロバイダ</summary>
        private readonly IConfigProvider _configProvider;

        /// <summary>依存注入</summary>
        public UpdateConfigProvider(IConfigProvider configProvider)
        {
            _configProvider = configProvider;
        }
        
        /// <summary>プロバイダ初期化</summary>
        public void Initialize()
        {
        }

        /// <summary>リソース解放</summary>
        public void Dispose()
        {
            throw new System.NotImplementedException();
        }
    }
}