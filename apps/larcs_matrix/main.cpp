#include <CLI/CLI.hpp>
#include <chrono>
#include <csignal>
#include <memory>
#include <thread>

#include "larcs/runtime/zenoh_transport.hpp"
#include "larcs/sim/robot/robot_assembly.hpp"
#include "larcs/sim/world.hpp"

// Ctrl+C ハンドラ
std::atomic<bool> running{true};
void signal_handler(int) { running = false; }

int main(int argc, char** argv) {
  CLI::App app{"LARCS Matrix - Physics Simulation Engine"};

  std::string config_file = "configs/sim_default.yaml";
  double time_scale = 1.0;
  bool headless = true;

  app.add_option("-c,--config", config_file, "Configuration file");
  app.add_option("-s,--speed", time_scale,
                 "Time scale (1.0=realtime, 10.0=10x)");
  app.add_flag("-h,--headless", headless, "Run without visualization");

  CLI11_PARSE(app, argc, argv);

  // シグナルハンドラ
  std::signal(SIGINT, signal_handler);

  // Zenoh初期化
  auto transport = std::make_shared<larcs::runtime::ZenohTransport>();
  if (!transport->initialize()) {
    spdlog::error("Failed to initialize Zenoh transport");
    return 1;
  }

  // World初期化
  larcs::sim::World world;
  if (!world.initialize()) {
    spdlog::error("Failed to initialize PhysX world");
    return 1;
  }
  world.set_time_scale(time_scale);

  // ロボット読み込み
  larcs::sim::RobotAssembly robot("larcs_robot");
  if (!robot.load_from_yaml(config_file)) {
    spdlog::error("Failed to load robot from config: {}", config_file);
    return 1;
  }

  // ロボットスポーン
  Eigen::Vector3d spawn_pos(0, 0, 0.1);
  if (!robot.spawn(&world, spawn_pos)) {
    spdlog::error("Failed to spawn robot");
    return 1;
  }

  // TODO: cmd_vel購読
  // auto cmd_vel_sub =
  // std::make_shared<larcs::runtime::Subscriber<larcs::msgs::Twist>>(
  //   transport, "/robot/cmd_vel",
  //   [&robot](const larcs::msgs::Twist& msg) {
  //     Eigen::Vector2d linear(msg.linear().x(), msg.linear().y());
  //     robot.set_velocity(linear, msg.angular().z());
  //   }
  // );

  // メインループ
  const double dt = 1.0 / 240.0;  // 240Hz固定ステップ
  auto last_time = std::chrono::steady_clock::now();

  spdlog::info("LARCS Matrix started (time_scale={}x)", time_scale);
  spdlog::info("Press Ctrl+C to stop");

  while (running) {
    // シミュレーションステップ
    world.step(dt);

    // ロボット更新
    robot.update(world.get_sim_time());

    // 状態配信
    robot.publish_state(transport);

    // リアルタイム同期（time_scale考慮）
    if (time_scale <= 1.0) {
      auto now = std::chrono::steady_clock::now();
      auto elapsed = std::chrono::duration<double>(now - last_time).count();
      double sleep_time = (dt / time_scale) - elapsed;
      if (sleep_time > 0) {
        std::this_thread::sleep_for(std::chrono::duration<double>(sleep_time));
      }
      last_time = std::chrono::steady_clock::now();
    } else {
      // 高速シミュレーションの場合は定期的にyield
      static int step_count = 0;
      if (++step_count % 100 == 0) {
        std::this_thread::yield();
      }
    }
  }

  spdlog::info("Shutting down LARCS Matrix");
  world.shutdown();
  transport->shutdown();

  return 0;
}
