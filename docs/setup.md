# LARCS 開発環境セットアップ

このガイドでは、Ubuntu 24.04 LTS 上で LARCS Engine の開発環境をセットアップする手順を説明します。

## 前提条件

### システム要件
- **OS**: Ubuntu 24.04 LTS
- **RAM**: 4GB 以上（推奨 8GB）
- **Disk**: 空き 10GB 以上
- **Compiler**: GCC 13+ または Clang 17+

## インストール手順

### 1. システムパッケージ

システムを更新し、ビルドに必要なツールをインストールします。

```bash
sudo apt update
sudo apt upgrade -y
sudo apt install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    curl \
    zip \
    unzip \
    tar \
    pkg-config \
    clang-format \
    clang-tidy
```

### 2. vcpkg のインストール

依存関係管理のために vcpkg をインストールします。

```bash
# Clone vcpkg
cd ~
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg

# Bootstrap vcpkg
./bootstrap-vcpkg.sh

# Add to PATH (add to ~/.bashrc for persistence)
export VCPKG_ROOT=~/vcpkg
export PATH=$VCPKG_ROOT:$PATH

# Optional: Add to .bashrc
echo 'export VCPKG_ROOT=~/vcpkg' >> ~/.bashrc
echo 'export PATH=$VCPKG_ROOT:$PATH' >> ~/.bashrc
```

### 3. LARCS リポジトリの取得

```bash
cd ~/projects  # or your preferred location
git clone https://github.com/dev-Earth/LARCS_Engine.git
cd LARCS_Engine
```

### 4. ビルド

#### CMake Presets を使う（推奨）

```bash
# Configure with default preset
cmake --preset default

# Build
cmake --build build/default

# Run tests
cd build/default
ctest --output-on-failure
```

#### Debug Preset を使う

```bash
# Configure debug build
cmake --preset debug

# Build
cmake --build build/debug

# Run tests with verbose output
cd build/debug
ctest --output-on-failure -V
```

### 5. インストール確認

```bash
# Test tools
./build/default/tools/larcs-ping --help
./build/default/tools/larcs-record --help
./build/default/tools/larcs-replay --help

# Run tests
cd build/default
ctest
```

期待される出力例:
```
Test project .../LARCS_Engine/build/default
    Start 1: time_test
1/1 Test #1: time_test ........................   Passed    0.01 sec

100% tests passed, 0 tests failed out of 1
```

## CLion IDE セットアップ

### 1. CLion のインストール

[JetBrains のサイト](https://www.jetbrains.com/clion/) から CLion をダウンロードしてインストールします。

### 2. プロジェクトを開く

1. CLion を起動
2. "Open" を選び、`LARCS_Engine` ディレクトリを指定
3. CLion が `CMakeLists.txt` を自動検出します

### 3. CLion で CMake を設定

CLion は `CMakePresets.json` に定義された CMake Preset を自動検出します。

1. **File → Settings → Build, Execution, Deployment → CMake** を開く
2. 以下のプロファイルが見えるはずです:
   - `clion`（Release）
   - `clion-debug`（Debug）
3. 目的に応じてプロファイルを選択（両方使っても OK）

### 4. 環境変数の設定

CLion が vcpkg を見つけられない場合:

1. **File → Settings → Build, Execution, Deployment → CMake**
2. 対象プロファイルの "Environment" を開く
3. `VCPKG_ROOT=/home/your_username/vcpkg` を追加

### 5. CLion でビルド

- **Build → Build Project** または `Ctrl+F9`
- ツールバーのドロップダウンからターゲットを選択できます

### 6. CLion でテスト実行

1. "Run" のドロップダウンを開く
2. 任意のテスト（例: `time_test`）を選択
3. 実行ボタン、または `Shift+F10`

### 7. コードフォーマット

CLion は `.clang-format` を自動で使います。

- ファイルを整形: `Ctrl+Alt+L`
- 保存時に整形: **Settings → Tools → Actions on Save → Reformat code**

## 開発ワークフロー

### ビルド

```bash
# Full rebuild
cmake --build build/default --clean-first

# Parallel build (faster)
cmake --build build/default -j$(nproc)

# Build specific target
cmake --build build/default --target larcs-ping
```

### テスト

```bash
# Run all tests
cd build/default
ctest

# Run specific test
./runtime/time_test

# Verbose test output
ctest -V

# Run tests matching pattern
ctest -R time
```

### コード品質

```bash
# Format all code
find . -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i

# Run clang-tidy (from build directory)
cd build/default
run-clang-tidy
```

## トラブルシューティング

### vcpkg 関連

**問題**: CMake が vcpkg を見つけられない
```bash
# Ensure VCPKG_ROOT is set
echo $VCPKG_ROOT

# Should output: /home/your_username/vcpkg
# If not, export it:
export VCPKG_ROOT=~/vcpkg
```

**問題**: vcpkg のパッケージインストールが失敗する
```bash
# Clear vcpkg cache
rm -rf $VCPKG_ROOT/buildtrees
rm -rf $VCPKG_ROOT/downloads

# Try installing manually
cd $VCPKG_ROOT
./vcpkg install fmt spdlog protobuf cli11 gtest
```

### ビルド関連

**問題**: Protobuf のコード生成が失敗する
```bash
# Ensure protobuf is installed
vcpkg list | grep protobuf

# Manually install if needed
vcpkg install protobuf
```

**問題**: GTest が見つからない
```bash
# Install GTest via vcpkg
vcpkg install gtest
```

### CLion 関連

**問題**: CLion に "CMake Error" が出る
- CLion 設定で `VCPKG_ROOT` がセットされているか確認
- "File → Reload CMake Project" を試す
- キャッシュを無効化: "File → Invalidate Caches → Invalidate and Restart"

**問題**: コード補完が動かない
- 右下の "Rescan files" が完了しているか確認
- "Tools → CMake → Reset Cache and Reload Project" で再生成

## 次に読むもの

- システム概要: [architecture.md](architecture.md)
- メッセージ定義: [messaging.md](messaging.md)
- 例コード: `runtime/tests/` 配下
- 必要なロボット制御ロジックを書き始めましょう

## 参考資料

- [CMake Documentation](https://cmake.org/documentation/)
- [vcpkg Documentation](https://vcpkg.io/)
- [Protocol Buffers Guide](https://protobuf.dev/)
- [CLion Documentation](https://www.jetbrains.com/clion/learn/)
