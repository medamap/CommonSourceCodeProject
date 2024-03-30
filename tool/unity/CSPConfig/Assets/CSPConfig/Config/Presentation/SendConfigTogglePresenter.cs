using System.Threading;
using CSPConfig.Config.Application;
using CSPConfig.CSPConfig.Domain;
using Cysharp.Threading.Tasks;
using MessagePipe;
using R3;
using UnityEngine;
using UnityEngine.UI;
using VContainer;

namespace CSPConfig.Config.Presentation
{
    [RequireComponent(typeof(Toggle))]
    public class SendConfigTogglePresenter : MonoBehaviour
    {
        /// <summary>Emulator Configure ステータス購読</summary>
        [Inject] public ISubscriber<CspStatus> CspStatusSubscriber { get; set; }
        /// <summary>コンフィグサービス</summary>
        [Inject] public ConfigService ConfigService { get; set; }

        /// <summary>コンフィグ送信設定</summary>
        private void Start()
        {
            // コンフィグ送信設定
            SendConfigSetUpAsync(destroyCancellationToken).Forget();
        }

        /// <summary>コンフィグ送信設定</summary>
        private async UniTask SendConfigSetUpAsync(CancellationToken cancellationToken)
        {
            // Emulator Configure ステータス CspStatus.StartCsp 待機
            await CspStatusSubscriber.FirstAsync(cancellationToken, x => x == CspStatus.StartCsp);

            // トグルコンポーネント取得
            var toggle = GetComponent<Toggle>();
            // コンポーネント名をハイフンで区切り、１つ目をセクション名、２つ目をキー名、３つ目を値とする（区切れない場合はデフォルトとして空文字を設定）
            var names = gameObject.name.Split('-');
            var section = names.Length > 0 ? names[0] : "";
            var key = names.Length > 1 ? names[1] : "";
            var value = names.Length > 2 ? names[2] : "";

            // トグル変更時の処理
            toggle.OnValueChangedAsObservable()
                .Subscribe(x =>
                    {
                        if (!x) return;
                        // 値を try で数値に変換する
                        // 数値以外の場合、コンフィグへ 0 の書き込み
                        // 数値の場合、コンフィグへ int 値の書き込み
                        ConfigService.WriteConfigInt(section, key, int.TryParse(value, out _) ? int.Parse(value) : 0);
                    }
                )
                .AddTo(this);
        }
    }
}
