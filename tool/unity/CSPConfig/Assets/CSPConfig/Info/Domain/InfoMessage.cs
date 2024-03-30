using UnityEngine;

namespace CSPConfig.Info.Domain
{
    public struct InfoMessage
    {
        public string Message { get; }
        public InfoMessage(string message)
        {
            Message = message;
        }
        public static InfoMessage GetInfoMessage(string message) => new InfoMessage(message);
    }
}
