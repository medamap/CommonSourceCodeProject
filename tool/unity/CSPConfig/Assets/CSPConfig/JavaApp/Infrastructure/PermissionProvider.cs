using System;
using CSPConfig.Info.Domain;
using CSPConfig.JavaApp.Domain;
using MessagePipe;
using UnityEngine;

namespace CSPConfig.JavaApp.Infrastructure
{
    /// <summary>権限プロバイダ</summary>
    // ReSharper disable once ClassNeverInstantiated.Global
    public class PermissionProvider : IPermissionProvider, IDisposable {

        /// <summary>権限チェックとリクエスト結果メッセージ送信</summary>
        private readonly IPublisher<ResultPermission> _publisherResultPermission;
        /// <summary>権限リクエスト購読の解除</summary>
        private readonly IDisposable _requestPermissionDisposable;

        /// <summary>お知らせメッセージ送信</summary>
        private readonly IPublisher<InfoMessage> _infoMessagePublisher;

        /// <summary>依存注入とリクエスト購読</summary>
        public PermissionProvider(
            IPublisher<ResultPermission> publisherResultPermission,
            ISubscriber<RequestPermission> subscriberRequestPermission,
            IPublisher<InfoMessage> infoMessagePublisher
            ) {
            Debug.Log("PermissionProvider::PermissionProvider()");
            _publisherResultPermission = publisherResultPermission;
            _infoMessagePublisher = infoMessagePublisher;

            // 権限リクエストメッセージ購読
            _requestPermissionDisposable = subscriberRequestPermission.Subscribe(x =>
            {
                Debug.Log("PermissionProvider::RequestPermissionSubscriber::Subscribe()");
                _infoMessagePublisher.Publish(InfoMessage.GetInfoMessage("権限チェックとリクエスト"));

                // 権限チェックとリクエスト
                CheckAndRequestPermission();
            });
        }

        /// <summary>権限チェックとリクエスト</summary>
        public bool CheckAndRequestPermission() {
            Debug.Log("PermissionProvider::CheckAndRequestPermission()");
            try
            {
                var resultPermission = false;
                using (var javaUnityPlayer = new AndroidJavaClass("com.unity3d.player.UnityPlayer")) {
                    var currentActivity = javaUnityPlayer.GetStatic<AndroidJavaObject>("currentActivity");
                    using (var permissionManager = new AndroidJavaClass("jp.megamin.android.cspconfig.CspConfig")) {
                        resultPermission = permissionManager.CallStatic<bool>("checkAndRequestPermissions", currentActivity);
                    }
                }
                // 権限チェックとリクエスト結果メッセージ送信
                _publisherResultPermission.Publish(ResultPermission.GetResultPermission(resultPermission));
                // 権限チェックとリクエスト結果
                if (resultPermission)
                {
                    Debug.Log("Permission granted");
                    _infoMessagePublisher.Publish(InfoMessage.GetInfoMessage("権限が許可されました"));
                }
                else
                {
                    Debug.Log("Permission denied");
                    _infoMessagePublisher.Publish(InfoMessage.GetInfoMessage("権限が拒否されました"));
                }
                return resultPermission;
            }
            catch (Exception e)
            {
                Debug.LogError($"PermissionProvider::CheckAndRequestPermission()::catch -> {e.Message}");
                _infoMessagePublisher.Publish(InfoMessage.GetInfoMessage("権限チェックとリクエストエラー"));
                // 権限チェックとリクエスト結果メッセージ送信
                _publisherResultPermission.Publish(ResultPermission.GetErrorResultPermission(e.Message));
                return false;
            }
        }

        /// <summary>リソース解放</summary>
        public void Dispose()
        {
            _requestPermissionDisposable?.Dispose();
        }
    }
}
