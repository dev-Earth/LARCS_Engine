# LARCS メッセージングシステム

## 概要

LARCS はメッセージ定義に Protocol Buffers（protobuf）を使用します。これにより以下が得られます。

- 言語非依存のシリアライズ
- バージョン互換性
- コンパクトなバイナリ形式
- 強い型付け
- 自動生成コード

## メッセージ分類

### 1. 共通メッセージ（`common.proto`）

#### Time
ナノ秒精度で時刻（ある時点）を表します。
```protobuf
message Time {
  int64 sec = 1;        // Seconds since epoch
  uint32 nanosec = 2;   // Nanoseconds component [0, 999999999]
}
```

**用途**: タイムスタンプ、経過時間（duration）計測、時刻同期

#### Header
多くのメッセージに含める標準ヘッダです。
```protobuf
message Header {
  Time timestamp = 1;    // When message was created
  string frame_id = 2;   // Reference frame (e.g., "base_link", "odom")
  uint64 seq = 3;        // Sequence number for ordering
}
```

**用途**: 時刻付きメッセージ全般（センサデータ、コマンド、テレメトリ）

### 2. 幾何メッセージ（`geometry.proto`）

#### Vector3
位置・速度・力などに使う 3 次元ベクトルです。
```protobuf
message Vector3 {
  double x = 1;
  double y = 2;
  double z = 3;
}
```

**単位**: 文脈依存（一般に m, m/s, N など）

#### Quaternion
姿勢を表すクォータニオンです（正規化が前提）。
```protobuf
message Quaternion {
  double x = 1;
  double y = 2;
  double z = 3;
  double w = 4;
}
```

**制約**: 正規化されていること（x² + y² + z² + w² = 1）

#### Pose
位置 + 姿勢をまとめて表します。
```protobuf
message Pose {
  Vector3 position = 1;
  Quaternion orientation = 2;
}
```

**用途**: ロボット姿勢、目標ウェイポイント、物体の位置

#### Twist
並進速度と角速度を表します。
```protobuf
message Twist {
  Vector3 linear = 1;    // Linear velocity [m/s]
  Vector3 angular = 2;   // Angular velocity [rad/s]
}
```

**用途**: 速度コマンド、オドメトリ

### 3. 制御メッセージ（`control.proto`）

#### TrajectoryPoint
軌道上の 1 点です。
```protobuf
message TrajectoryPoint {
  double t = 1;           // Time from start [seconds]
  Pose pose = 2;          // Target pose at this time
  Twist velocity = 3;     // Target velocity at this time
}
```

#### TrajectoryPlan
ロボットが実行する軌道全体です。
```protobuf
message TrajectoryPlan {
  Header header = 1;
  string plan_id = 2;                     // Unique identifier
  repeated TrajectoryPoint points = 3;    // Trajectory waypoints
  double v_max = 4;                       // Max linear velocity [m/s]
  double a_max = 5;                       // Max linear accel [m/s²]
  double omega_max = 6;                   // Max angular velocity [rad/s]
}
```

**用途**: プランナからコントローラへの高レベル運動指令

#### WheelState
駆動輪の現在状態です。
```protobuf
message WheelState {
  Header header = 1;
  repeated double rpm = 2;           // RPM for each wheel
  repeated int64 encoder_ticks = 3;  // Raw encoder values
}
```

**用途**: オドメトリ、モータ制御フィードバック、診断

### 4. ヘルス（状態監視）メッセージ（`health.proto`）

#### ComponentStatus
システム内の 1 コンポーネントの状態です。
```protobuf
message ComponentStatus {
  enum State {
    UNKNOWN = 0;
    OK = 1;
    WARNING = 2;
    ERROR = 3;
    FATAL = 4;
  }
  
  string name = 1;      // Component identifier
  State state = 2;      // Current health state
  string message = 3;   // Human-readable status
  double value = 4;     // Optional metric (temp, load, etc.)
}
```

#### SystemHealth
システム全体のヘルスレポートです。
```protobuf
message SystemHealth {
  Header header = 1;
  repeated ComponentStatus components = 2;
}
```

**用途**: 監視、診断、自動故障検知

## 設計原則

### 1. 拡張性
- 既存フィールドを壊さずに新しいフィールドを追加できる
- 後方互換のためオプショナルなフィールドを活用
- 廃止フィールド番号は予約（reserved）する

### 2. 一貫性
- 時刻付きメッセージは `Header` を含める
- タイムスタンプは `Time` 型を使用する
- 姿勢と位置は `Pose`（position + orientation）を用いる

### 3. 効率
- コンパクトなバイナリ形式
- ゼロコピーのデシリアライズが可能
- 配列は repeated フィールドで表現

### 4. 相互運用性
- 言語非依存の定義
- ブリッジにより ROS と相互運用可能
- Python/Java などへのコード生成が容易

## メッセージ利用パターン

### Publish パターン
```cpp
// Create message
TrajectoryPlan plan;
plan.mutable_header()->set_timestamp(...);
plan.set_plan_id("traj_001");
// ... fill in details

// Publish
publisher.Publish(plan);
```

### Subscribe パターン
```cpp
void OnTrajectory(const TrajectoryPlan& plan) {
  // Process received plan
  for (const auto& point : plan.points()) {
    // Execute trajectory point
  }
}

Subscriber<TrajectoryPlan> sub("trajectory", OnTrajectory);
```

## 拡張ガイドライン

### 新しいメッセージタイプの追加
1. 適切な `.proto` ファイルを選ぶ（または新規作成）
2. 分かりやすいフィールド名を付けて message を定義
3. 目的や単位（units）をコメントで明記
4. 新しいファイルの場合は `msgs/CMakeLists.txt` に追加
5. このドキュメントも更新

### バージョニング
- proto のパッケージはセマンティックバージョニングを想定
- 既存フィールドを変更せず、新フィールド追加で対応
- 削除したフィールド番号は reserved で確保
- 破壊的変更は必ず文書化

### ベストプラクティス
- メッセージは目的を絞り、凝集度を高く保つ
- 複雑な構造はネスト message を活用
- 単位（m, rad, s など）をコメントに書く
- 固定セットは enum で表現
- 時刻が重要なデータは Header を含める
