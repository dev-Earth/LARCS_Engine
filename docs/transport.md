# LARCS Transport Layer

## 概要

LARCS は通信トランスポート層として **Zenoh** を採用しています。Zenoh は高性能で ROS 非依存の Pub/Sub および RPC を提供し、自動ピア検出（auto discovery）も備えています。これにより、シミュレーション・実機・監視ツール（モニタリング）間をまたぐロボットアプリケーションに適した通信基盤になります。

## Zenoh とは？

[Zenoh](https://zenoh.io/) は、以下を目的として設計されたモダンな通信ミドルウェアです。

- **高性能**: ゼロコピー転送、低レイテンシ
- **自動検出（Auto Discovery）**: マルチキャストによるスカウティングでピアを自動発見
- **柔軟なデプロイ**: プロセス間・マシン間・ネットワーク越しでも動作
- **ROS 非依存**: ROS のインフラに依存しない
- **統一 API**: ローカルでもリモートでも同じコードで動作

従来のミドルウェアと異なり、Zenoh はリアルタイム・ロボティクス用途を前提に、最小オーバーヘッドで動作するよう設計されています。

## アーキテクチャ

### トランスポート抽象化

LARCS は Zenoh の上に、扱いやすい抽象化レイヤを提供します。

```cpp
// Initialize transport
auto transport = std::make_shared<larcs::runtime::ZenohTransport>();
transport->initialize();

// Create publisher
auto pub = std::make_shared<larcs::runtime::Publisher<larcs::msgs::Twist>>(
    transport, "/cmd_vel", QoSProfile::Control);

// Create subscriber
auto sub = std::make_shared<larcs::runtime::Subscriber<larcs::msgs::Twist>>(
    transport, "/cmd_vel", 
    [](const larcs::msgs::Twist& msg) {
        // Handle received message
    },
    QoSProfile::Control);

// Publish message
larcs::msgs::Twist twist;
twist.mutable_linear()->set_x(1.0);
pub->publish(twist);
```

### QoS プロファイル

LARCS は用途別に最適化した 3 つの Quality-of-Service (QoS) プロファイルを定義しています。

#### Control（高信頼）
- **用途**: 軌道コマンド、制御信号、緊急停止
- **混雑制御（Congestion Control）**: BLOCK（容量が空くまで待つ）
- **優先度**: REAL_TIME（最優先）
- **信頼性**: メッセージを落とさない

```cpp
QoSProfile::Control
```

#### Telemetry（最新値優先）
- **用途**: ロボット状態、オドメトリ、センサ値
- **混雑制御（Congestion Control）**: DROP（古いデータを捨てる）
- **優先度**: DATA（通常）
- **信頼性**: 最新値が最も重要

```cpp
QoSProfile::Telemetry
```

#### Perception（高スループット）
- **用途**: 点群、画像など大容量センサデータ
- **混雑制御（Congestion Control）**: DROP（混雑時に破棄）
- **優先度**: DATA_LOW（バックグラウンド）
- **信頼性**: ベストエフォート

```cpp
QoSProfile::Perception
```

## ネットワーク設定

### ピアモード（Peer Mode）

LARCS はデフォルトで Zenoh の **ピアモード** を使用します。

- すべてのノードが対等（中央ブローカー不要）
- マルチキャストによる自動ピア検出
- 1 台のマシンから分散システムまでスケール

### マルチキャスト・スカウティング

Zenoh のピアはローカルネットワーク上で UDP マルチキャストにより自動的に相互発見します。

- **マルチキャストアドレス**: 224.0.0.224:7447（デフォルト）
- **発見**: 自動・ゼロコンフィグ
- **動作範囲**: 同一マシン / 同一 LAN / 同一サブネット

## 使い方

### C++ API

#### Publisher 例

```cpp
#include "larcs/runtime/publisher.hpp"
#include "larcs/runtime/zenoh_transport.hpp"
#include "larcs/msgs/geometry.pb.h"

auto transport = std::make_shared<larcs::runtime::ZenohTransport>();
transport->initialize();

auto pub = std::make_shared<larcs::runtime::Publisher<larcs::msgs::Twist>>(
    transport, "/robot/cmd_vel", larcs::runtime::QoSProfile::Control);

larcs::msgs::Twist cmd;
cmd.mutable_linear()->set_x(1.5);
cmd.mutable_angular()->set_z(0.3);

pub->publish(cmd);
```

#### Subscriber 例

```cpp
#include "larcs/runtime/subscriber.hpp"
#include "larcs/runtime/zenoh_transport.hpp"
#include "larcs/msgs/geometry.pb.h"

auto transport = std::make_shared<larcs::runtime::ZenohTransport>();
transport->initialize();

auto sub = std::make_shared<larcs::runtime::Subscriber<larcs::msgs::Twist>>(
    transport, "/robot/cmd_vel",
    [](const larcs::msgs::Twist& msg) {
        std::cout << "Received: linear.x = " << msg.linear().x() << std::endl;
    },
    larcs::runtime::QoSProfile::Control);

// Keep running to receive messages
while (true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
```

### CLI ツール

LARCS はテスト・デバッグ用にコマンドラインツールも提供します。

#### larcs-pub

トピックへメッセージを publish します。

```bash
# Publish Twist message
./build/default/tools/larcs-pub /cmd_vel '{"linear":{"x":1.0},"angular":{"z":0.5}}' -t Twist

# Publish Pose message
./build/default/tools/larcs-pub /pose '{"position":{"x":1.0,"y":2.0,"z":0.0}}' -t Pose

# Publish Vector3 message
./build/default/tools/larcs-pub /point '{"x":1.0,"y":2.0,"z":3.0}' -t Vector3
```

#### larcs-sub

トピックからメッセージを subscribe します。

```bash
# Subscribe to Twist messages (unlimited)
./build/default/tools/larcs-sub /cmd_vel -t Twist

# Subscribe and exit after 10 messages
./build/default/tools/larcs-sub /cmd_vel -t Twist --count 10

# Subscribe to Pose messages with verbose output
./build/default/tools/larcs-sub /pose -t Pose -v
```

## 通信のテスト

### 単一マシンでのテスト

ターミナル1:
```bash
./build/default/tools/larcs-sub /test/twist -t Twist
```

ターミナル2:
```bash
./build/default/tools/larcs-pub /test/twist '{"linear":{"x":1.5},"angular":{"z":0.5}}' -t Twist
```

subscriber 側にメッセージが表示されれば OK です。

### マルチプロセスでのテスト

Zenoh の自動検出により、プロセスが別でも同じ手順でそのまま動作します。

### マルチマシンでのテスト

マシンA:
```bash
./build/default/tools/larcs-sub /robot/cmd_vel -t Twist
```

マシンB（同一ネットワーク）:
```bash
./build/default/tools/larcs-pub /robot/cmd_vel '{"linear":{"x":1.0}}' -t Twist
```

メッセージは自動的にルーティングされます。

## トラブルシューティング

### メッセージが受信できない

1. **ファイアウォール確認**: UDP マルチキャストを許可
   ```bash
   # Ubuntu/Debian
   sudo ufw allow proto udp to 224.0.0.0/4
   ```

2. **マルチキャストルート確認**: マルチキャストのルーティングが有効か確認
   ```bash
   ip route show | grep 224
   # Should show: 224.0.0.0/4 dev <interface> ...
   ```

3. **マルチキャストルート追加**（無い場合）:
   ```bash
   sudo ip route add 224.0.0.0/4 dev eth0  # or your network interface
   ```

### レイテンシが高い

- 時間クリティカルなメッセージは `QoSProfile::Control` を使う
- ネットワーク混雑を確認
- CPU リソースが足りているか確認

### メッセージがドロップされる

- `QoSProfile::Telemetry` / `Perception` が用途に合っているか確認
- 重要なメッセージは `QoSProfile::Control` を使う
- 帯域をモニタリング

### Discovery（自動検出）がうまくいかない

1. **同じサブネット**: 同一サブネット上か確認
2. **VPN の影響**: VPN によりマルチキャストがブロックされることがある
3. **Docker/VM**: ネットワーク設定が必要になる場合がある

### デバッグログを有効化

```cpp
#include <spdlog/spdlog.h>
spdlog::set_level(spdlog::level::debug);
```

あるいは CLI ツールの `-v` オプション:

```bash
./build/default/tools/larcs-sub /topic -t Twist -v
```

## パフォーマンス特性

### レイテンシ
- ローカル（同一プロセス）: < 1 μs
- プロセス間（同一マシン）: < 100 μs
- ネットワーク（LAN）: < 1 ms

### スループット
- 小さいメッセージ（< 1 KB）: > 100k msg/s
- 大きいメッセージ（> 1 MB）: ネットワーク帯域がボトルネック

### Discovery 時間
- ローカルネットワーク: < 100 ms
- サブネット越え: ルータ設定に依存

## LARCS との統合

### シミュレーションモード
トランスポートはシミュレーションでも実機でも同一に動作します。

### 実機ロボットモード
同じ API のまま、コード変更なしで動作します。

### 監視（Monitor）モード
CLI ツールや独自モニタにより、すべてのトラフィックを観測できます。

## 将来拡張

今後のフェーズで計画している内容:
- **Service/Client RPC**: リクエスト・レスポンス
- **Recorder/Replayer**: MCAP 形式との統合
- **ネットワーク統計**: レイテンシ、ロス、帯域監視
- **高度なルーティング**: カスタムルーティングポリシー

## 参考資料

- [Zenoh Documentation](https://zenoh.io/docs/)
- [Zenoh GitHub](https://github.com/eclipse-zenoh/zenoh)
- [LARCS Architecture](architecture.md)
- [Message Specifications](messaging.md)
