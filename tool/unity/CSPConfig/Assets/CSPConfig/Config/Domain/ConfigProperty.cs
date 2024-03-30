public struct ConfigProperty {
    public ConfigPropertyDirection Direction { get; }
    public string Section { get; }
    public string Key { get; }
    public object Value { get; }

    public ConfigProperty(ConfigPropertyDirection direction, string section, string key, object value) {
        Direction = direction;
        Section = section;
        Key = key;
        Value = value;
    }

    /// <summary>コンフィグ読み込み</summary>
    public static ConfigProperty ReadConfigProperty(string section, string key) {
        return new ConfigProperty(ConfigPropertyDirection.Load, section, key, null);
    }
    
    /// <summary>コンフィグ書き込み</summary>
    public static ConfigProperty WriteConfigProperty(string section, string key, object value) {
        return new ConfigProperty(ConfigPropertyDirection.Store, section, key, value);
    }
}
