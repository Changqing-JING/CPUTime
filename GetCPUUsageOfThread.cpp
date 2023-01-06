#include <chrono>
#include <cstdio>
#include <ctime>
#include <stdexcept>

#include <sys/types.h>
#include <unistd.h>

#include <atomic>
#include <thread>

class CPUCostOfThread {
public:
  explicit CPUCostOfThread(pid_t tid) : tid_(tid) {}

  void refresh() {
    char fname[200];
    snprintf(fname, sizeof(fname), "/proc/self/task/%d/stat", tid_);
    FILE *fp = fopen(fname, "r");
    if (fp == nullptr) {
      throw std::runtime_error("open file failed\n");
    }
    int ucpu = 0, scpu = 0, tot_cpu = 0;
    if (fscanf(fp, "%*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %d %d",
               &ucpu, &scpu) == 2) {
      tot_cpu = ucpu + scpu;

      printf("total time is %d, user time %d, kernel time %d\n", tot_cpu, ucpu,
             scpu);
    } else {
      printf("read file failed\n");
    }
    fclose(fp);
  }

private:
  pid_t tid_;
};

int main() {

  std::atomic<pid_t> thread_pid{0};
  std::thread th([&thread_pid] {
    thread_pid = gettid();
    while (true) {
    }
  });

  while (thread_pid == 0) {
  }

  CPUCostOfThread cpuOfThread(thread_pid);
  while (true) {
    cpuOfThread.refresh();
    std::this_thread::sleep_for(std::chrono::seconds(3));
  }
  th.join();
  return 0;
}

