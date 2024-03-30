using System;
using CSPConfig.Info.Domain;
using CSPConfig.JavaApp.Domain;
using MessagePipe;
using UnityEngine;
using VContainer.Unity;

namespace CSPConfig.JavaApp.Application
{
    /// <summary>メインサービス</summary>
    // ReSharper disable once ClassNeverInstantiated.Global
    public class JavaAppService : IInitializable, IDisposable {
        private bool _disposedValue;

        /// <summary>権限プロバイダ</summary>
        private readonly IPermissionProvider _permissionProvider;
        
        /// <summary>お知らせメッセージ送信</summary>
        private readonly IPublisher<InfoMessage> _infoMessagePublisher;

        /// <summary>依存注入</summary>
        public JavaAppService(
            IPermissionProvider permissionProvider,
            IPublisher<InfoMessage> infoMessagePublisher
            ) {
            Debug.Log("JavaAppService::JavaAppService()");
            _permissionProvider = permissionProvider;
            _infoMessagePublisher = infoMessagePublisher;
        }

        /// <summary>サ－ビス初期化</summary>
        public void Initialize()
        {
            Debug.Log("JavaAppService::Initialize()");
            _infoMessagePublisher.Publish(InfoMessage.GetInfoMessage("メインサービス初期化"));
        }

        /// <summary>権限チェックとリクエスト</summary>
        public bool CheckAndRequestPermission() {
            Debug.Log("JavaAppService::CheckAndRequestPermission()");
            var result = _permissionProvider.CheckAndRequestPermission();
            return result;
        }

        /// <summary>リソースの解放</summary>
        public void Dispose() {
            Debug.Log("JavaAppService::Dispose()");
        }
    }
}
