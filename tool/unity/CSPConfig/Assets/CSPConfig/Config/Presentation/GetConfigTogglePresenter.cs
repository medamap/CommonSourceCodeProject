using System;
using CSPConfig.Config.Application;
using CSPConfig.CSPConfig.Domain;
using MessagePipe;
using UnityEngine;
using UnityEngine.UI;
using VContainer;

namespace CSPConfig.Config.Presentation
{
    [RequireComponent(typeof(Toggle))]
    public class GetConfigTogglePresenter : MonoBehaviour
    {
        /// <summary>Emulator Configure ステータス購読</summary>
        [Inject] public ISubscriber<CspStatus> CspStatusSubscriber { get; set; }
        /// <summary>Emulator Configure ステータス購読破棄</summary>
        IDisposable _cspStatusSubscription;
        /// <summary>コンフィグサービス</summary>
        [Inject] public ConfigService ConfigService { get; set; }

        /// <summary>ステータス更新購読</summary>
        private void Start()
        {
            // トグルコンポーネント取得
            var toggle = GetComponent<Toggle>();
            // コンポーネント名をハイフンで区切り、１つ目をセクション名、２つ目をキー名、３つ目を値とする（区切れない場合はデフォルトとして空文字を設定）
            var names = gameObject.name.Split('-');
            var section = names.Length > 0 ? names[0] : "";
            var key = names.Length > 1 ? names[1] : "";
            var value = names.Length > 2 ? names[2] : "";
            
            Debug.Log($"Setup Toggle: section={section}, key={key}, value={value}");
            
            // Emulator Configure ステータス GetConfig 待機
            _cspStatusSubscription = CspStatusSubscriber
                .Subscribe(x =>
                    {
                        // コンフィグ取得
                        var getValue = ConfigService.ReadConfigString(section, key);
                        var isOn = !string.IsNullOrWhiteSpace(getValue) && value.Trim() == getValue.Trim();
                        Debug.Log($"GetConfigTogglePresenter::Subscribe(): section={section}, key={key}, value={value}, getValue={getValue}, isOn={isOn}");
                        // 取得値がnullまたは空白でない場合、コンポーネント名のvalueと一致するかチェックし、一致する場合はトグルをONにする
                        // また、コンポーネント名及びコンフィグの値はそれぞれ前後の余分な空白を除去する
                        toggle.isOn = isOn;
                    },
                    x => x == CspStatus.GetConfig
                );

        }
        
        /// <summary>ステータス更新購読破棄</summary>
        private void OnDestroy()
        {
            _cspStatusSubscription?.Dispose();
        }
    }
}