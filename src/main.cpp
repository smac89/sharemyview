#include "smv/winclient.hpp"
#include <chrono>
#include <condition_variable>
#include <csignal>
#include <mutex>
#include <spdlog/cfg/env.h>
#include <spdlog/spdlog.h>

static std::condition_variable cv;

extern "C" void sigHandler(int) {
  static std::once_flag sigHandled;
  std::call_once(sigHandled, &std::condition_variable::notify_one, &cv);
}

int main() {
  spdlog::cfg::load_env_levels();
  std::signal(SIGINT, sigHandler);
  std::signal(SIGTERM, sigHandler);

  static std::mutex m;
  spdlog::info("Starting...");
  smv::init();

  std::unique_lock<std::mutex> lk(m);
  cv.wait_for(lk, std::chrono::minutes(10));
  smv::deinit();

  spdlog::info("Finished");
  return 0;
}
