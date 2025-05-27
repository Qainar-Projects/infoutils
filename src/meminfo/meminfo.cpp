/*
 * meminfo - Memory Information Utility
 * Part of QCO InfoUtils (Qainrat Code Organization InfoUtils)
 * Author: AnmiTaliDev
 * License: Apache 2.0
 */

 #include <iostream>
 #include <fstream>
 #include <sstream>
 #include <string>
 #include <vector>
 #include <map>
 #include <algorithm>
 #include <iomanip>
 #include <filesystem>
 #include <cstring>
 #include <unistd.h>
 #include <sys/sysinfo.h>
 
 namespace fs = std::filesystem;
 
 // ANSI Color codes
 namespace Colors {
     const std::string RESET = "\033[0m";
     const std::string BOLD = "\033[1m";
     const std::string RED = "\033[31m";
     const std::string GREEN = "\033[32m";
     const std::string YELLOW = "\033[33m";
     const std::string BLUE = "\033[34m";
     const std::string MAGENTA = "\033[35m";
     const std::string CYAN = "\033[36m";
     const std::string WHITE = "\033[37m";
     const std::string DIM = "\033[2m";
 }
 
 struct MemoryInfo {
     unsigned long total_kb = 0;
     unsigned long available_kb = 0;
     unsigned long free_kb = 0;
     unsigned long buffers_kb = 0;
     unsigned long cached_kb = 0;
     unsigned long swap_total_kb = 0;
     unsigned long swap_free_kb = 0;
     unsigned long swap_cached_kb = 0;
     unsigned long shmem_kb = 0;
     unsigned long sreclaimable_kb = 0;
     unsigned long sunreclaim_kb = 0;
 };
 
 struct ProcessInfo {
     int pid;
     std::string name;
     unsigned long memory_kb;
     std::string cmd;
 };
 
 class MemInfoUtil {
 private:
     bool show_processes = false;
     bool show_detailed = false;
     bool show_swap = false;
     bool use_colors = true;
 
     std::string formatBytes(unsigned long kb) {
         if (kb == 0) return "0";
         
         double bytes = kb * 1024.0;
         const char* units[] = {"B", "KB", "MB", "GB", "TB"};
         int unit = 0;
         
         while (bytes >= 1024.0 && unit < 4) {
             bytes /= 1024.0;
             unit++;
         }
         
         std::ostringstream oss;
         if (unit == 0) {
             oss << static_cast<unsigned long>(bytes) << " " << units[unit];
         } else {
             oss << std::fixed << std::setprecision(1) << bytes << " " << units[unit];
         }
         return oss.str();
     }
 
     std::string colorize(const std::string& text, const std::string& color) {
         if (!use_colors) return text;
         return color + text + Colors::RESET;
     }
 
     MemoryInfo getMemoryInfo() {
         MemoryInfo info;
         std::ifstream file("/proc/meminfo");
         std::string line;
         
         while (std::getline(file, line)) {
             std::istringstream iss(line);
             std::string key;
             unsigned long value;
             
             if (iss >> key >> value) {
                 if (key == "MemTotal:") info.total_kb = value;
                 else if (key == "MemAvailable:") info.available_kb = value;
                 else if (key == "MemFree:") info.free_kb = value;
                 else if (key == "Buffers:") info.buffers_kb = value;
                 else if (key == "Cached:") info.cached_kb = value;
                 else if (key == "SwapTotal:") info.swap_total_kb = value;
                 else if (key == "SwapFree:") info.swap_free_kb = value;
                 else if (key == "SwapCached:") info.swap_cached_kb = value;
                 else if (key == "Shmem:") info.shmem_kb = value;
                 else if (key == "SReclaimable:") info.sreclaimable_kb = value;
                 else if (key == "SUnreclaim:") info.sunreclaim_kb = value;
             }
         }
         
         return info;
     }
 
     std::vector<ProcessInfo> getTopProcesses(int limit = 15) {
         std::vector<ProcessInfo> processes;
         
         try {
             for (const auto& entry : fs::directory_iterator("/proc")) {
                 if (!entry.is_directory()) continue;
                 
                 std::string dirname = entry.path().filename();
                 if (!std::all_of(dirname.begin(), dirname.end(), ::isdigit)) continue;
                 
                 int pid = std::stoi(dirname);
                 std::string status_path = "/proc/" + dirname + "/status";
                 std::string cmdline_path = "/proc/" + dirname + "/cmdline";
                 
                 std::ifstream status_file(status_path);
                 if (!status_file.is_open()) continue;
                 
                 std::string name;
                 unsigned long memory_kb = 0;
                 std::string line;
                 
                 while (std::getline(status_file, line)) {
                     if (line.substr(0, 5) == "Name:") {
                         name = line.substr(6);
                     } else if (line.substr(0, 6) == "VmRSS:") {
                         std::istringstream iss(line);
                         std::string key;
                         if (iss >> key >> memory_kb) {
                             break;
                         }
                     }
                 }
                 
                 // Get command line
                 std::string cmd;
                 std::ifstream cmdline_file(cmdline_path);
                 if (cmdline_file.is_open()) {
                     std::getline(cmdline_file, cmd);
                     // Replace null bytes with spaces
                     std::replace(cmd.begin(), cmd.end(), '\0', ' ');
                     if (cmd.length() > 40) {
                         cmd = cmd.substr(0, 37) + "...";
                     }
                 }
                 
                 if (!name.empty() && memory_kb > 0) {
                     processes.push_back({pid, name, memory_kb, cmd});
                 }
             }
         } catch (const std::exception& e) {
             std::cerr << colorize("Warning: Could not read all process information", Colors::YELLOW) << std::endl;
         }
         
         std::sort(processes.begin(), processes.end(), 
                   [](const ProcessInfo& a, const ProcessInfo& b) {
                       return a.memory_kb > b.memory_kb;
                   });
         
         if (processes.size() > limit) {
             processes.resize(limit);
         }
         
         return processes;
     }
 
     void printSeparator(const std::string& title = "") {
         if (title.empty()) {
             std::cout << std::string(70, '-') << std::endl;
         } else {
             std::cout << colorize(title, Colors::BOLD) << std::endl;
             std::cout << std::string(title.length(), '=') << std::endl;
         }
     }
 
     void printGeneralInfo() {
         MemoryInfo info = getMemoryInfo();
         
         unsigned long used_kb = info.total_kb - info.available_kb;
         double used_percentage = (double)used_kb / info.total_kb * 100.0;
         
         printSeparator("Memory Information");
         
         std::cout << std::left << std::setw(18) << "Total:" 
                   << std::setw(12) << formatBytes(info.total_kb) 
                   << colorize("(" + std::to_string(info.total_kb) + " kB)", Colors::DIM) << std::endl;
         
         std::cout << std::left << std::setw(18) << "Available:" 
                   << std::setw(12) << formatBytes(info.available_kb)
                   << colorize("(" + std::to_string(info.available_kb) + " kB)", Colors::DIM) << std::endl;
         
         std::cout << std::left << std::setw(18) << "Used:" 
                   << std::setw(12) << formatBytes(used_kb)
                   << colorize("(" + std::to_string(used_kb) + " kB, " + 
                              std::to_string(static_cast<int>(used_percentage)) + "%)", Colors::DIM) << std::endl;
         
         std::cout << std::left << std::setw(18) << "Free:" 
                   << std::setw(12) << formatBytes(info.free_kb)
                   << colorize("(" + std::to_string(info.free_kb) + " kB)", Colors::DIM) << std::endl;
         
         if (show_detailed) {
             std::cout << std::left << std::setw(18) << "Buffers:" 
                       << std::setw(12) << formatBytes(info.buffers_kb)
                       << colorize("(" + std::to_string(info.buffers_kb) + " kB)", Colors::DIM) << std::endl;
             
             std::cout << std::left << std::setw(18) << "Cached:" 
                       << std::setw(12) << formatBytes(info.cached_kb)
                       << colorize("(" + std::to_string(info.cached_kb) + " kB)", Colors::DIM) << std::endl;
             
             if (info.shmem_kb > 0) {
                 std::cout << std::left << std::setw(18) << "Shared:" 
                           << std::setw(12) << formatBytes(info.shmem_kb)
                           << colorize("(" + std::to_string(info.shmem_kb) + " kB)", Colors::DIM) << std::endl;
             }
             
             if (info.sreclaimable_kb > 0 || info.sunreclaim_kb > 0) {
                 std::cout << std::left << std::setw(18) << "Slab reclaimable:" 
                           << std::setw(12) << formatBytes(info.sreclaimable_kb)
                           << colorize("(" + std::to_string(info.sreclaimable_kb) + " kB)", Colors::DIM) << std::endl;
                 
                 std::cout << std::left << std::setw(18) << "Slab unreclaimable:" 
                           << std::setw(12) << formatBytes(info.sunreclaim_kb)
                           << colorize("(" + std::to_string(info.sunreclaim_kb) + " kB)", Colors::DIM) << std::endl;
             }
         }
         
         if (info.swap_total_kb > 0 || show_swap) {
             std::cout << std::endl;
             printSeparator("Swap Information");
             
             if (info.swap_total_kb > 0) {
                 unsigned long swap_used_kb = info.swap_total_kb - info.swap_free_kb;
                 double swap_used_percentage = (double)swap_used_kb / info.swap_total_kb * 100.0;
                 
                 std::cout << std::left << std::setw(18) << "Total:" 
                           << std::setw(12) << formatBytes(info.swap_total_kb)
                           << colorize("(" + std::to_string(info.swap_total_kb) + " kB)", Colors::DIM) << std::endl;
                 
                 std::cout << std::left << std::setw(18) << "Free:" 
                           << std::setw(12) << formatBytes(info.swap_free_kb)
                           << colorize("(" + std::to_string(info.swap_free_kb) + " kB)", Colors::DIM) << std::endl;
                 
                 std::cout << std::left << std::setw(18) << "Used:" 
                           << std::setw(12) << formatBytes(swap_used_kb)
                           << colorize("(" + std::to_string(swap_used_kb) + " kB, " + 
                                      std::to_string(static_cast<int>(swap_used_percentage)) + "%)", Colors::DIM) << std::endl;
                 
                 if (info.swap_cached_kb > 0) {
                     std::cout << std::left << std::setw(18) << "Cached:" 
                               << std::setw(12) << formatBytes(info.swap_cached_kb)
                               << colorize("(" + std::to_string(info.swap_cached_kb) + " kB)", Colors::DIM) << std::endl;
                 }
             } else {
                 std::cout << "No swap space configured" << std::endl;
             }
         }
     }
 
     void printProcesses() {
         std::cout << std::endl;
         printSeparator("Top Memory Consumers");
         
         auto processes = getTopProcesses(15);
         
         std::cout << std::left 
                   << std::setw(8) << "PID"
                   << std::setw(16) << "COMMAND"
                   << std::setw(12) << "MEMORY"
                   << "CMDLINE" << std::endl;
         printSeparator();
         
         for (const auto& proc : processes) {
             std::cout << std::left 
                       << std::setw(8) << proc.pid
                       << std::setw(16) << proc.name.substr(0, 15)
                       << std::setw(12) << formatBytes(proc.memory_kb)
                       << proc.cmd << std::endl;
         }
     }
 
 public:
     MemInfoUtil() {
         // Check if output is terminal for color support
         use_colors = isatty(STDOUT_FILENO);
     }
 
     void parseArgs(int argc, char* argv[]) {
         for (int i = 1; i < argc; ++i) {
             std::string arg = argv[i];
             
             if (arg == "--help" || arg == "-h") {
                 printHelp();
                 exit(0);
             } else if (arg == "--version" || arg == "-V") {
                 printVersion();
                 exit(0);
             } else if (arg == "--processes" || arg == "-p") {
                 show_processes = true;
             } else if (arg == "--detailed" || arg == "-d") {
                 show_detailed = true;
             } else if (arg == "--swap" || arg == "-s") {
                 show_swap = true;
             } else if (arg == "--all" || arg == "-a") {
                 show_processes = show_detailed = show_swap = true;
             } else if (arg == "--no-color") {
                 use_colors = false;
             } else {
                 std::cerr << colorize("meminfo: invalid option -- '" + arg + "'", Colors::RED) << std::endl;
                 std::cerr << "Try 'meminfo --help' for more information." << std::endl;
                 exit(1);
             }
         }
     }
 
     void printVersion() {
         std::cout << "meminfo (QCO InfoUtils) 1.0" << std::endl;
         std::cout << "Copyright (C) 2025 AnmiTaliDev" << std::endl;
         std::cout << "License Apache 2.0: Apache License version 2.0" << std::endl;
         std::cout << "This is free software: you are free to change and redistribute it." << std::endl;
         std::cout << "There is NO WARRANTY, to the extent permitted by law." << std::endl;
     }
 
     void printHelp() {
         std::cout << "Usage: meminfo [OPTION]..." << std::endl;
         std::cout << "Display information about system memory usage." << std::endl;
         std::cout << std::endl;
         std::cout << "  -a, --all         display all available information" << std::endl;
         std::cout << "  -d, --detailed    show detailed memory breakdown" << std::endl;
         std::cout << "  -h, --help        display this help and exit" << std::endl;
         std::cout << "      --no-color    disable colored output" << std::endl;
         std::cout << "  -p, --processes   show top memory consuming processes" << std::endl;
         std::cout << "  -s, --swap        show swap space information" << std::endl;
         std::cout << "  -V, --version     output version information and exit" << std::endl;
         std::cout << std::endl;
         std::cout << "Examples:" << std::endl;
         std::cout << "  meminfo           Show basic memory information" << std::endl;
         std::cout << "  meminfo -a        Show comprehensive memory report" << std::endl;
         std::cout << "  meminfo -p        Show memory usage with top processes" << std::endl;
         std::cout << std::endl;
         std::cout << "QCO InfoUtils home page: <https://github.com/Qainar-Projects/infoutils>" << std::endl;
     }
 
     void run() {
         printGeneralInfo();
         
         if (show_processes) {
             printProcesses();
         }
     }
 };
 
 int main(int argc, char* argv[]) {
     try {
         MemInfoUtil util;
         util.parseArgs(argc, argv);
         util.run();
         return 0;
     } catch (const std::exception& e) {
         std::cerr << "meminfo: " << e.what() << std::endl;
         return 1;
     }
 }