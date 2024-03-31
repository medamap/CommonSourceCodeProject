using CSPConfig.Config.Application;
using CSPConfig.Config.Domain;
using CSPConfig.Config.Infrastructure;
using CSPConfig.CSPConfig.Application;
using CSPConfig.CSPConfig.Domain;
using CSPConfig.CSPConfig.Infrastructure;
using CSPConfig.Info.Domain;
using CSPConfig.JavaApp.Application;
using CSPConfig.JavaApp.Domain;
using CSPConfig.JavaApp.Infrastructure;
using MessagePipe;
using UnityEngine;
using VContainer;
using VContainer.Unity;

namespace CSPConfig.LifetimeScope
{
    // ReSharper disable once InconsistentNaming
    public class CSPConfigLifetimeScope : VContainer.Unity.LifetimeScope
    {
        protected override void Configure(IContainerBuilder builder) {
            Debug.Log($"CSPConfigLifetimeScope.Configure()");

            // MessagePipeでメッセージを送受信するためのサービスを登録
            var options = builder.RegisterMessagePipe();

            //// Message /////////////////////////////////////////////////

            // ResultPermission をメッセージ登録
            builder.RegisterMessageBroker<ResultPermission>(options);
            builder.RegisterMessageBroker<RequestPermission>(options);
            builder.RegisterMessageBroker<InfoMessage>(options);
            builder.RegisterMessageBroker<CspStatus>(options);
            builder.RegisterMessageBroker<ConfigProperty>(options);

            //// Config /////////////////////////////////////////////////

            builder.Register<ConfigService>(Lifetime.Scoped);
            builder.Register<IConfigFileHelper, ConfigFileHelper>(Lifetime.Scoped);
            builder.Register<IConfigProvider, ConfigProvider>(Lifetime.Scoped);
            builder.Register<IUpdateConfigProvider, UpdateConfigProvider>(Lifetime.Scoped);

            //// CspConfig /////////////////////////////////////////////////

            builder.Register<CspConfigService>(Lifetime.Scoped);
            builder.Register<ICspConfigTaskProvider, CspConfigTaskProvider>(Lifetime.Scoped);

            //// JavaApp /////////////////////////////////////////////////

            builder.Register<JavaAppService>(Lifetime.Scoped);
            builder.Register<IPermissionProvider, PermissionProvider>(Lifetime.Scoped);
            
            //// EntryPoint /////////////////////////////////////////////////

            builder.RegisterEntryPoint<ConfigService>();
            builder.RegisterEntryPoint<CspConfigService>();
            builder.RegisterEntryPoint<JavaAppService>();

        }
    }
}
