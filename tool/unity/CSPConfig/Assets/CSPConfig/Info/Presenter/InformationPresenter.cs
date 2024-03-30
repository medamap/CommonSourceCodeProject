using System;
using MessagePipe;
using TMPro;
using UnityEngine;
using VContainer;

namespace CSPConfig.Info.Presenter
{
    /// <summary>お知らせプレゼンタ</summary>
    [RequireComponent(typeof(TextMeshProUGUI))]
    public class InformationPresenter : MonoBehaviour
    {
        /// <summary>お知らせ購読</summary>
        [Inject]
        public ISubscriber<Domain.InfoMessage> InfoMessageSubscriber { get; set; }

        private IDisposable _infoMessageDisposable;

        /// <summary>テキストラベル</summary>
        private TextMeshProUGUI _text;

        /// <summary>
        /// お知らせ購読
        /// </summary>
        private void Start()
        {
            // テキストラベル取得
            _text = GetComponent<TextMeshProUGUI>();

            // お知らせ購読
            _infoMessageDisposable = InfoMessageSubscriber.Subscribe(x => { _text.text = x.Message; });
        }

        /// <summary>リソースの解放</summary>
        private void OnDestroy()
        {
            _infoMessageDisposable?.Dispose();
        }
    }
}
