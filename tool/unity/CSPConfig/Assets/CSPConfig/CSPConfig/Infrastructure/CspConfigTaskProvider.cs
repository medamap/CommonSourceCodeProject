using System;
using System.Threading;
using CSPConfig.Config.Application;
using CSPConfig.CSPConfig.Domain;
using CSPConfig.Info.Domain;
using CSPConfig.JavaApp.Application;
using Cysharp.Threading.Tasks;
using MessagePipe;
using UnityEngine;

namespace CSPConfig.CSPConfig.Infrastructure
{
    /// <summary>Emulator Configure タスクプロバイダ</summary>
    // ReSharper disable once ClassNeverInstantiated.Global
    public class CspConfigTaskProvider : ICspConfigTaskProvider
    {
        /// <summary>メインサービス</summary>
        private readonly JavaAppService _javaAppService;
        /// <summary>コンフィグサービス</summary>
        private readonly ConfigService _configService;
        /// <summary>Emulator Configure ステータスのメッセージ送信</summary>
        private readonly IPublisher<CspStatus> _cspStatusPublisher;
        /// <summary>お知らせメッセージ送信</summary>
        private readonly IPublisher<InfoMessage> _infoMessagePublisher;

        /// <summary>Emulator Configure ステータス</summary>
        public CspStatus CspStatus { get; private set; }

        /// <summary>タスクのキャンセルトークンソース</summary>
        private CancellationTokenSource _cspConfigTaskCancellationTokenSource;
        
        /// <summary>依存注入</summary>
        protected CspConfigTaskProvider(
            JavaAppService javaAppService,
            ConfigService configService,
            IPublisher<CspStatus> cspStatusPublisher,
            IPublisher<InfoMessage> infoMessagePublisher
            )
        {
            // 依存注入
            _javaAppService = javaAppService;
            _configService = configService;
            _cspStatusPublisher = cspStatusPublisher;
            _infoMessagePublisher = infoMessagePublisher;
            
            // Emulator Configure ステータス初期化
            UpdateCspStatus(CspStatus.Boot);
        }

        /// <summary>プロバイダ初期化</summary>
        public void Initialize()
        {
            // キャンセルトークンソース生成
            _cspConfigTaskCancellationTokenSource = new CancellationTokenSource();
            // Emulator Configure タスク実行
            ExecuteCspConfigTaskAsync(_cspConfigTaskCancellationTokenSource.Token).Forget();
        }
        
        /// <summary>Emulator Configure タスク</summary>
        private async UniTask ExecuteCspConfigTaskAsync(CancellationToken cancellationToken)
        {
            // アクセス権限チェックとリクエスト
            UpdateCspStatus(CspStatus.PermissionRequest);

            // 権限チェックとリクエストを行い、拒否だった場合、1秒毎にサイド権限チェックとリクエストを行うよう繰り返す
            while (!_javaAppService.CheckAndRequestPermission())
            {
                UpdateCspStatus(CspStatus.PermissionDenied);
                await UniTask.Delay(TimeSpan.FromSeconds(1), cancellationToken: cancellationToken);
            }

            // 権限が許可された場合、Emulator Configure ステータスを更新
            UpdateCspStatus(CspStatus.PermissionGranted);
            // コンフィグファイル取得リクエスト
            UpdateCspStatus(CspStatus.RequestGetConfig);
            _infoMessagePublisher.Publish(InfoMessage.GetInfoMessage("コンフィグファイル取得リクエスト"));
            Debug.Log("コンフィグファイル取得リクエスト");
            
            // コンフィグファイルが存在しない場合、1秒毎にコンフィグファイルの存在チェックを行うよう繰り返す
            while (!_configService.IsExistsConfig())
            {
                _infoMessagePublisher.Publish(InfoMessage.GetInfoMessage($"コンフィグファイルが存在しません\n{_configService.ConfigPath}"));
                Debug.Log($"コンフィグファイルが存在しません\n{_configService.ConfigPath}");
                UpdateCspStatus(CspStatus.NoConfig);
                await UniTask.Delay(TimeSpan.FromSeconds(1), cancellationToken: cancellationToken);
            }

            // 1秒待機
            await UniTask.Delay(TimeSpan.FromSeconds(1), cancellationToken: cancellationToken);

            // コンフィグファイル取得完了
            UpdateCspStatus(CspStatus.GetConfig);
            _infoMessagePublisher.Publish(InfoMessage.GetInfoMessage($"コンフィグファイル取得完了\n{_configService.ConfigPath}"));
            Debug.Log($"コンフィグファイル取得完了\n{_configService.ConfigPath}");
            
            // 1秒待機
            await UniTask.Delay(TimeSpan.FromSeconds(1), cancellationToken: cancellationToken);
            
            // Emulator Configure 開始
            UpdateCspStatus(CspStatus.StartCsp);
            _infoMessagePublisher.Publish(InfoMessage.GetInfoMessage("Emulator Configure 開始"));
            Debug.Log("Emulator Configure 開始");
        }
        
        private void UpdateCspStatus(CspStatus cspStatus)
        {
            CspStatus = cspStatus;
            _cspStatusPublisher.Publish(cspStatus);
        }

        /// <summary>リソース解放</summary>
        public void Dispose()
        {
            // タスクの破棄
            _cspConfigTaskCancellationTokenSource?.Cancel();
        }
    }
}
