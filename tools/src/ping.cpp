#include <arpa/inet.h>
#include <fmt/core.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <CLI/CLI.hpp>
#include <chrono>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>

int main(int argc, char** argv) {
  CLI::App app{"LARCS Network Ping Tool - UDP connectivity test"};

  std::string host = "127.0.0.1";
  int port = 8888;
  int count = 4;
  int timeout_ms = 1000;

  app.add_option("--host", host, "Target host address")->default_val("127.0.0.1");
  app.add_option("-p,--port", port, "Target port")->default_val(8888);
  app.add_option("-c,--count", count, "Number of ping attempts")->default_val(4);
  app.add_option("-t,--timeout", timeout_ms, "Timeout in milliseconds")->default_val(1000);

  CLI11_PARSE(app, argc, argv);

  fmt::print("LARCS PING {} port {} with {} attempts\n", host, port, count);

  // Create UDP socket
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    fmt::print(stderr, "Error creating socket: {}\n", strerror(errno));
    return 1;
  }

  // Set socket timeout
  struct timeval tv;
  tv.tv_sec = timeout_ms / 1000;
  tv.tv_usec = (timeout_ms % 1000) * 1000;
  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

  // Setup server address
  struct sockaddr_in server_addr;
  std::memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);

  if (inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr) <= 0) {
    fmt::print(stderr, "Invalid address: {}\n", host);
    close(sockfd);
    return 1;
  }

  int successful = 0;
  int failed = 0;

  for (int i = 0; i < count; ++i) {
    std::string message = fmt::format("PING {}", i);

    auto start = std::chrono::steady_clock::now();

    // Send ping
    ssize_t sent = sendto(sockfd, message.c_str(), message.length(), 0,
                          (struct sockaddr*)&server_addr, sizeof(server_addr));

    if (sent < 0) {
      fmt::print("Attempt {}: Send failed - {}\n", i + 1, strerror(errno));
      failed++;
      continue;
    }

    // Try to receive response
    char buffer[1024];
    struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr);

    ssize_t received =
        recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr*)&from_addr, &from_len);

    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    if (received > 0) {
      buffer[received] = '\0';
      fmt::print("Attempt {}: Response received in {} ms\n", i + 1, duration.count());
      successful++;
    } else {
      fmt::print("Attempt {}: Timeout (no response)\n", i + 1);
      failed++;
    }

    // Wait between attempts
    if (i < count - 1) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }

  close(sockfd);

  fmt::print("\n--- Summary ---\n");
  fmt::print("Sent: {}, Received: {}, Failed: {}\n", count, successful, failed);
  fmt::print("Success rate: {:.1f}%\n", (100.0 * successful) / static_cast<double>(count));

  return (successful > 0) ? 0 : 1;
}
