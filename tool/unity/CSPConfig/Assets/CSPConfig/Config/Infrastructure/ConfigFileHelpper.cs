using CSPConfig.Config.Domain;
using System;
using System.IO;
using System.Text;
using UnityEngine;

namespace CSPConfig.Config.Infrastructure
{
    public class ConfigFileHelper : IConfigFileHelper
    {
        /// <summary>プロファイル文字列書き込み</summary>
        public bool MyWritePrivateProfileString(string lpAppName, string lpKeyName, string lpString, string lpFileName)
        {
            bool result = false;
            string tempPath = lpFileName + ".$$$";
            StringBuilder newContents = new StringBuilder();

            try
            {
                bool inSection = false;
                bool foundKey = false;

                // Read the original file and modify the contents.
                if (File.Exists(lpFileName))
                {
                    foreach (var line in File.ReadAllLines(lpFileName))
                    {
                        if (line.StartsWith("[") && line.EndsWith("]"))
                        {
                            // Write the key-value pair before moving to the next section
                            if (!foundKey && inSection)
                            {
                                newContents.AppendLine($"{lpKeyName}={lpString}");
                                foundKey = true;
                            }

                            inSection = line.Equals($"[{lpAppName}]", StringComparison.OrdinalIgnoreCase);
                        }

                        if (inSection && line.Contains("="))
                        {
                            string[] keyValue = line.Split(new[] { '=' }, 2);
                            if (keyValue[0].Equals(lpKeyName, StringComparison.OrdinalIgnoreCase))
                            {
                                newContents.AppendLine($"{lpKeyName}={lpString}");
                                foundKey = true;
                                continue; // Skip the original line as it's replaced
                            }
                        }

                        newContents.AppendLine(line);
                    }
                }

                // If the section or key were not found, append them at the end.
                if (!foundKey)
                {
                    if (!inSection)
                    {
                        newContents.AppendLine($"[{lpAppName}]");
                    }
                    newContents.AppendLine($"{lpKeyName}={lpString}");
                }

                // Write the modified contents to a temporary file
                File.WriteAllText(tempPath, newContents.ToString());

                // Replace the original file with the updated one
                File.Delete(lpFileName);
                File.Move(tempPath, lpFileName);

                result = true;
            }
            catch (Exception ex)
            {
                Debug.Log($"Error writing to config file: {ex.Message}\n({lpAppName}, {lpKeyName}, {lpString}, {lpFileName})");
                result = false;
            }
            finally
            {
                if (File.Exists(tempPath))
                {
                    File.Delete(tempPath);
                }
            }

            return result;
        }
        
        /// <summary>プロファイル文字列読み込み</summary>
        public string MyGetPrivateProfileString(string lpAppName, string lpKeyName, string lpDefault, string lpFileName)
        {
            string result = lpDefault ?? string.Empty;

            if (!File.Exists(lpFileName))
            {
                return result;
            }

            bool inSection = false;
            string section = $"[{lpAppName}]";

            foreach (var line in File.ReadAllLines(lpFileName))
            {
                if (line.StartsWith("[") && line.EndsWith("]"))
                {
                    if (inSection) break;

                    inSection = line.Equals(section, StringComparison.OrdinalIgnoreCase);
                    continue;
                }

                if (inSection && line.Contains("="))
                {
                    var keyValue = line.Split(new[] { '=' }, 2);
                    if (keyValue[0].Trim().Equals(lpKeyName, StringComparison.OrdinalIgnoreCase))
                    {
                        result = keyValue.Length > 1 ? keyValue[1].Trim() : string.Empty;
                        break;
                    }
                }
            }
            return result;
        }

        /// <summary>プロファイル数値読み込み</summary>
        public int MyGetPrivateProfileInt(string lpAppName, string lpKeyName, int nDefault, string lpFileName)
        {
            string stringValue = MyGetPrivateProfileString(lpAppName, lpKeyName, nDefault.ToString(), lpFileName);
            if (int.TryParse(stringValue, out int result))
            {
                return result;
            }
            return nDefault;
        }
        
    }
}


