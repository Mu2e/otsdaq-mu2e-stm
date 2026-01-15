#ifndef SYS_UTILS_HH
#define SYS_UTILS_HH

#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <pthread.h>
#include <sched.h>
#include <numeric>
#include <sstream>
#include <algorithm>

#include <vector>
#include <set>
#include <thread>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <sched.h>
#include <string>
#include <limits.h>
#include <chrono>
#include "tcp_utils.hh"

// System variables
int PORT = 5050; // TCP socket port
std::string IP = "127.0.0.1";

std::string getCurrentDateTime() {
    // Get current time
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    // Convert to local time safely
    std::tm local_time{};
#if defined(_WIN32)
    localtime_s(&local_time, &now_time);
#else
    localtime_r(&now_time, &local_time);
#endif
    // Format the time as a string
    std::ostringstream oss;
    oss << std::put_time(&local_time, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

// Create Sender TCP socket and connect to receiver
inline int setup_sender_socket(const std::string& server_ip, int port, int rcvbuf_bytes) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        return -1;
    }

    if (connect(sockfd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sockfd);
        return -1;
    }

    // Increase TCP send buffer
    int bufsize = rcvbuf_bytes;
    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &bufsize, sizeof(bufsize)) < 0) {
        perror("setsockopt(SO_SNDBUF)");
    } else {
        std::cout << "TCP send buffer set to "
                  << bufsize / (1024 * 1024) << " MB" << std::endl;
    }

    std::cout << getCurrentDateTime() << " | Sender connected to receiver at " << server_ip << ":" << port << std::endl;
    return sockfd;
}

// Create Receiver TCP socket, bind it, and start listening
// NOTE: accept() is intentionally NOT called here
inline int setup_tcp_socket(const std::string& server_ip, int port, int rcvbuf_bytes)
{
    TLOG_DEBUG(1) << "1 setup_tcp_socket host=" << server_ip << " port=" << port;

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return -1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);

    // Convert input IP string to binary form
    if (inet_pton(AF_INET, server_ip.c_str(), &addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(server_fd);
        return -1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, 1) < 0) {
        perror("listen");
        close(server_fd);
        return -1;
    }

    std::cout << getCurrentDateTime()
              << " | STMTCPReceiver listening on "
              << server_ip << ":" << port << std::endl;

    TLOG_DEBUG(1) << "Listening socket established (fd=" << server_fd << ")";

    // IMPORTANT:
    //  - Do NOT accept() here
    //  - accept() is performed in receiverThread_()
    return server_fd;
}

struct CPUUsage {
    uint64_t user = 0, nice = 0, system = 0, idle = 0, iowait = 0, irq = 0, softirq = 0;
    uint64_t total() const { return user + nice + system + idle + iowait + irq + softirq; }
    uint64_t active() const { return total() - idle - iowait; }
};

// Track pinned cores
std::set<int> pinned_cores;

// Read CPU usage from /proc/stat
std::vector<CPUUsage> get_cpu_usage() {
    std::ifstream file("/proc/stat");
    std::string line;
    std::vector<CPUUsage> usage;

    while (std::getline(file, line)) {
        if (line.substr(0, 3) != "cpu") break; // skip non-cpu lines
        if (line.substr(0, 4) == "cpu ") continue; // skip aggregate cpu line

        std::istringstream ss(line);
        std::string cpu_label;
        CPUUsage u;
        ss >> cpu_label >> u.user >> u.nice >> u.system >> u.idle >> u.iowait >> u.irq >> u.softirq;
        usage.push_back(u);
    }
    return usage;
}

// Calculate % CPU usage per core
std::vector<int> calc_core_usage(const std::vector<CPUUsage>& prev, const std::vector<CPUUsage>& curr) {
    std::vector<int> usage;
    for (size_t i = 0; i < curr.size(); ++i) {
        uint64_t prev_active = prev[i].active();
        uint64_t prev_total = prev[i].total();
        uint64_t curr_active = curr[i].active();
        uint64_t curr_total = curr[i].total();

        double percent = 100.0 * (curr_active - prev_active) / double(curr_total - prev_total);
        usage.push_back(static_cast<int>(percent));
    }
    return usage;
}

int find_least_busy_core(const std::vector<int>& usage) {
    int min_usage = INT_MAX;
    int best_core = -1;
    for (size_t i = 0; i < usage.size(); ++i) {
        if (pinned_cores.count(i)) continue;
        if (usage[i] < min_usage) {
            min_usage = usage[i];
            best_core = i;
        }
    }
    if (best_core != -1) pinned_cores.insert(best_core);
    return best_core;
}

void pin_thread_to_least_busy_core(std::thread& t, std::string thrd_str) {
    // Measure CPU usage twice to get delta
    auto prev = get_cpu_usage();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto curr = get_cpu_usage();

    auto usage = calc_core_usage(prev, curr);
    int core = find_least_busy_core(usage);

    if (core < 0) {
        std::cerr << thrd_str << " | [WARNING] No available cores, using default\n";
        return;
    }

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core, &cpuset);

    int rc = pthread_setaffinity_np(t.native_handle(), sizeof(cpu_set_t), &cpuset);
    if (rc != 0) {
        std::cerr << thrd_str << " | [ERROR] Failed to set thread affinity: " << rc << "\n";
    } else {
        if(verbose) std::cout << thrd_str << " | [INFO] Thread pinned to core " << core << "\n";
    }
}

#endif // SYS_UTILS_HH
