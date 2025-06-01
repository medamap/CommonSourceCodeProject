#!/bin/bash
#
# build_machine.sh
# CommonSourceCodeProject 機種別ビルドスクリプト
#
# Author: Medamap and Claude  
# Date: 2025.01.29
#

set -e

# 色付き出力
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# ヘルプ表示
show_help() {
    echo "CommonSourceCodeProject 機種別ビルドスクリプト"
    echo ""
    echo "使用方法:"
    echo "  $0 <機種名> [オプション]"
    echo ""
    echo "機種名:"
    echo "  x1turbo    - X1 turbo"
    echo "  x1         - X1"
    echo "  pc8801     - PC-8801"
    echo "  pc9801     - PC-9801"
    echo "  msx1       - MSX1"
    echo "  msx2       - MSX2"
    echo "  fm7        - FM-7"
    echo "  fm77       - FM-77"
    echo "  mz80k      - MZ-80K"
    echo "  mz80a      - MZ-80A"
    echo "  mz80b      - MZ-80B"
    echo "  mz700      - MZ-700"
    echo "  mz800      - MZ-800"
    echo "  mz1500     - MZ-1500"
    echo "  mz2200     - MZ-2200"
    echo "  mz2500     - MZ-2500"
    echo "  colecovision - ColecoVision"
    echo ""
    echo "オプション:"
    echo "  -c, --clean     ビルドディレクトリをクリーンアップ"
    echo "  -r, --release   リリースビルド（デフォルト: Debug）"
    echo "  -j N            並列ビルド（N個のジョブ）"
    echo "  -v, --verbose   詳細出力"
    echo "  -h, --help      このヘルプを表示"
    echo ""
    echo "例:"
    echo "  $0 x1turbo                    # X1 turbo をデバッグビルド"
    echo "  $0 pc8801 --release          # PC-8801 をリリースビルド"
    echo "  $0 msx1 --clean --release -j4 # MSX1 をクリーン後リリースビルド（4並列）"
}

# ログ出力
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1" >&2
}

# デフォルト値
MACHINE=""
BUILD_TYPE="Debug"
CLEAN=false
JOBS=4
VERBOSE=false
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# 引数解析
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -c|--clean)
            CLEAN=true
            shift
            ;;
        -r|--release)
            BUILD_TYPE="Release"
            shift
            ;;
        -j)
            JOBS="$2"
            shift 2
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -*)
            log_error "不明なオプション: $1"
            show_help
            exit 1
            ;;
        *)
            if [[ -z "$MACHINE" ]]; then
                MACHINE="$1"
            else
                log_error "複数の機種が指定されました: $MACHINE, $1"
                exit 1
            fi
            shift
            ;;
    esac
done

# 機種名チェック
if [[ -z "$MACHINE" ]]; then
    log_error "機種名が指定されていません"
    show_help
    exit 1
fi

# 利用可能な機種リスト
AVAILABLE_MACHINES=(
    "x1turbo" "x1" "pc8801" "pc9801" "msx1" "msx2" 
    "fm7" "fm77" "mz80k" "mz80a" "mz80b" "mz700" 
    "mz800" "mz1500" "mz2200" "mz2500" "pc6001" "colecovision"
)

# 機種名を小文字に変換
MACHINE=$(echo "$MACHINE" | tr '[:upper:]' '[:lower:]')

# 機種の有効性チェック
MACHINE_VALID=false
for available in "${AVAILABLE_MACHINES[@]}"; do
    if [[ "$MACHINE" == "$available" ]]; then
        MACHINE_VALID=true
        break
    fi
done

if [[ "$MACHINE_VALID" == false ]]; then
    log_error "サポートされていない機種: $MACHINE"
    echo "利用可能な機種:"
    for machine in "${AVAILABLE_MACHINES[@]}"; do
        echo "  - $machine"
    done
    exit 1
fi

# 設定ファイルの存在確認
MACHINE_UPPER=$(echo "$MACHINE" | tr '[:lower:]' '[:upper:]')
MACHINE_CONFIG="$SCRIPT_DIR/Machines/_${MACHINE_UPPER}.txt"

if [[ ! -f "$MACHINE_CONFIG" ]]; then
    log_error "機種設定ファイルが見つかりません: $MACHINE_CONFIG"
    log_info "設定ファイルを生成してください: python3 generate_machine_configs.py $MACHINE"
    exit 1
fi

# ビルドディレクトリ
BUILD_DIR="$SCRIPT_DIR/build_${MACHINE}_$(echo "$BUILD_TYPE" | tr '[:upper:]' '[:lower:]')"

# 情報表示
log_info "CommonSourceCodeProject ビルド設定"
echo "機種: $MACHINE ($MACHINE_UPPER)"
echo "ビルドタイプ: $BUILD_TYPE"
echo "ビルドディレクトリ: $BUILD_DIR"
echo "並列ジョブ数: $JOBS"
echo "詳細出力: $VERBOSE"

# クリーンアップ
if [[ "$CLEAN" == true ]]; then
    log_info "ビルドディレクトリをクリーンアップ中..."
    if [[ -d "$BUILD_DIR" ]]; then
        rm -rf "$BUILD_DIR"
        log_success "クリーンアップ完了"
    else
        log_warning "ビルドディレクトリが存在しません: $BUILD_DIR"
    fi
fi

# ビルドディレクトリ作成
log_info "ビルドディレクトリを作成中..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# CMake設定
log_info "CMake設定を実行中..."

CMAKE_ARGS=(
    "-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
    "-DMACHINE=$MACHINE"
    "-DCMAKE_OSX_DEPLOYMENT_TARGET=10.13"
    "$SCRIPT_DIR"
)

if [[ "$VERBOSE" == true ]]; then
    CMAKE_ARGS+=("-DCMAKE_VERBOSE_MAKEFILE=ON")
fi

if ! cmake "${CMAKE_ARGS[@]}"; then
    log_error "CMake設定に失敗しました"
    exit 1
fi

log_success "CMake設定完了"

# ビルド実行
log_info "ビルドを開始中..."

MAKE_ARGS=()
if [[ "$JOBS" -gt 1 ]]; then
    MAKE_ARGS+=("-j$JOBS")
fi

if [[ "$VERBOSE" == true ]]; then
    MAKE_ARGS+=("VERBOSE=1")
fi

if ! make "${MAKE_ARGS[@]}"; then
    log_error "ビルドに失敗しました"
    exit 1
fi

log_success "ビルド完了"

# 実行ファイルの確認
EXECUTABLE="$BUILD_DIR/bin/$MACHINE"
if [[ -f "$EXECUTABLE" ]]; then
    log_success "実行ファイルが生成されました: $EXECUTABLE"
    
    # ファイル情報表示
    log_info "実行ファイル情報:"
    ls -lh "$EXECUTABLE"
    
    if command -v file >/dev/null 2>&1; then
        file "$EXECUTABLE"
    fi
else
    log_warning "実行ファイルが見つかりません: $EXECUTABLE"
fi

# 成功メッセージ
echo ""
log_success "機種 $MACHINE のビルドが完了しました！"
echo ""
echo "実行方法:"
echo "  $EXECUTABLE"
echo ""
echo "デバッグ情報:"
echo "  lldb $EXECUTABLE"