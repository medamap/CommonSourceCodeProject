# Build Process

The usual build flow is:

1. Clone this repository from GitHub.
2. Initialize the `oboe` submodule.
3. Set up Android Studio and the local Android SDK settings.
4. Run the build scripts in `androidstudio/buildbatch/`.

## Typical Steps

```bash
git clone <repo-url>
cd CommonSourceCodeProject
git submodule update --init --recursive oboe
```

Then open `androidstudio/` in Android Studio and make sure:

- Gradle uses JDK 17.
- `androidstudio/local.properties` points to your Android SDK.

After that, use the scripts in `androidstudio/buildbatch/`:

- `ReleaseBuild` for APK generation
- `DebugBuild` for debug APK generation
- `ReleaseBuildExecute` or `DebugBuildExecute` for build + install + start

## Notes

- The build scripts support both Windows batch files and shell scripts.
- For large test runs, `allexecute.sh` / `allexecute.bat` can iterate through the
  machine list CSV files.
- See the upstream wiki for machine-specific details.
