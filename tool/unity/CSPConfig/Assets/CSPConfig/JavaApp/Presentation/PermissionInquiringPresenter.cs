using System;
using CSPConfig.JavaApp.Domain;
using MessagePipe;
using TMPro;
using UnityEngine;
using VContainer;

namespace CSPConfig.JavaApp.Presentation
{
    [RequireComponent(typeof(TextMeshProUGUI))]
    public class PermissionInquiringPresenter : MonoBehaviour
    {
        /// <summary>権限チェックとリクエスト結果の購読</summary>
        [Inject] public ISubscriber<ResultPermission> ResultPermissionSubscriber { get; set; }
        private IDisposable _requestPermissionDisposable;

        /// <summary>テキストラベル</summary>
        private TextMeshProUGUI _text;
        
        /// <summary>権限チェックとリクエスト結果の購読</summary>
        private void Start()
        {
            Debug.Log("PermissionInquiringPresenter::Start()");
            // テキストラベル取得
            _text = GetComponent<TextMeshProUGUI>();
            // 権限チェックとリクエスト結果の購読
            _requestPermissionDisposable = ResultPermissionSubscriber.Subscribe(x =>
            {
                if (x.ErrorMessage != null)
                {
                    _text.text = x.ErrorMessage;
                    return;
                }
                _text.text = x.IsGranted ? "Granted" : "Denied";
            });
        }
        
        /// <summary>リソースの解放</summary>
        private void OnDestroy()
        {
            Debug.Log("PermissionInquiringPresenter::OnDestroy()");
            _requestPermissionDisposable?.Dispose();
        }
    }
}
