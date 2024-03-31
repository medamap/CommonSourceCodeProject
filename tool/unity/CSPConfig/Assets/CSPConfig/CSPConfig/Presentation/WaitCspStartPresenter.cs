using System;
using CSPConfig.CSPConfig.Domain;
using MessagePipe;
using UnityEngine;
using UnityEngine.UI;
using VContainer;

namespace CSPConfig.CSPConfig.Presentation
{
    /// <summary>Emulator Configure 起動待ちプレゼンタ</summary>
    public class WaitCspStartPresenter : MonoBehaviour
    {
        /// <summary>Emulator Configure ステータス購読</summary>
        [Inject] public ISubscriber<CspStatus> CspStatusSubscriber { get; set; }
        /// <summary>Emulator Configure ステータス購読破棄</summary>
        IDisposable _cspStatusDisposable;

        /// <summary>Unity UI</summary>
        private Selectable _selectable;
        
        /// <summary>ステータス待機</summary>
        private void Start()
        {
            // UI 取得
            _selectable = GetComponent<Selectable>();
            // UI 非活性
            _selectable.interactable = false;
            // Emulator Configure ステータス購読
            _cspStatusDisposable = CspStatusSubscriber
                .Subscribe(x =>
                    _selectable.interactable = true,
                    x => x == CspStatus.StartCsp);
            
        }
        
        /// <summary>リソースの解放</summary>
        private void OnDestroy()
        {
            _cspStatusDisposable?.Dispose();
        }
    }
}
