using System;
using VContainer.Unity;

namespace CSPConfig.CSPConfig.Domain
{
    /// <summary>Emulator Configure ステータスプロバイダ</summary>
    public interface ICspConfigTaskProvider : IInitializable, IDisposable
    {
        /// <summary>Emulator Configure ステータス</summary>
        CspStatus CspStatus { get; }
    }
}
