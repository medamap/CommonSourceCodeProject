using System;
using CSPConfig.CSPConfig.Domain;
using UnityEngine;
using VContainer.Unity;

namespace CSPConfig.CSPConfig.Application
{
    /// <summary>Emulator Configure サービス</summary>
    // ReSharper disable once ClassNeverInstantiated.Global
    public class CspConfigService : IInitializable, IDisposable
    {
        /// <summary>Emulator Config ステータスプロバイダ</summary>
        private readonly ICspConfigTaskProvider _cspConfigTaskProvider;

        /// <summary>依存注入</summary>
        public CspConfigService(ICspConfigTaskProvider cspConfigTaskProvider)
        {
            Debug.Log("CSPConfigService::CSPConfigService()");
            _cspConfigTaskProvider = cspConfigTaskProvider;
        }
        
        /// <summary>サービス初期化</summary>
        public void Initialize()
        {
            Debug.Log("CSPConfigService::Initialize()");
            
            // Emulator Configure タスク初期化
            _cspConfigTaskProvider.Initialize();
        }

        /// <summary>リソース解放</summary>
        public void Dispose()
        {
            Debug.Log("CSPConfigService::Dispose()");
        }
    }
}
