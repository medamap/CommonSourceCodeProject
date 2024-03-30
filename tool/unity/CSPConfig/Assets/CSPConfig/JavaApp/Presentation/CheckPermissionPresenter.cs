using MessagePipe;
using R3;
using UnityEngine;
using UnityEngine.UI;
using VContainer;

namespace CSPConfig.JavaApp.Presentation
{
    [RequireComponent(typeof(Button))]
    public class CheckPermissionPresenter : MonoBehaviour
    {
        /// <summary>権限チェックとリクエスト結果の購読</summary>
        [Inject] public IPublisher<Domain.RequestPermission> RequestPermissionPublisher { get; set; }

        /// <summary>権限リクエストボタン</summary>
        void Start()
        {
            var button = GetComponent<Button>();
            button.OnClickAsObservable()
                .Subscribe(_ => {
                    // アクセス権限リクエスト
                    Debug.Log("CheckPermissionPresenter::OnClick()");
                    RequestPermissionPublisher.Publish(Domain.RequestPermission.GetRequestPermission());
                })
                .AddTo(this);
        }
    }
}
