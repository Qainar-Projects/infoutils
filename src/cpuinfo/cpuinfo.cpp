/*
 * cpuinfo - CPU Information Utility
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
 
 struct CpuInfo {
     std::string model_name;
     std::string vendor_id;
     std::string cpu_family;
     std::string model;
     std::string stepping;
     std::string microcode;
     std::string cache_size;
     std::vector<std::string> flags;
     double cpu_mhz = 0.0;
     int physical_cores = 0;
     int logical_cores = 0;
     int siblings = 0;
     int core_id = 0;
     std::string architecture;
     std::string byte_order;
     std::string virtualization;
 };
 
 struct CpuLoad {
     double load1 = 0.0;
     double load5 = 0.0;
     double load15 = 0.0;
     double cpu_usage = 0.0;
     unsigned long long user = 0;
     unsigned long long nice = 0;
     unsigned long long system = 0;
     unsigned long long idle = 0;
     unsigned long long iowait = 0;
     unsigned long long irq = 0;
     unsigned long long softirq = 0;
 };
 
 struct CpuFrequency {
     double current_mhz = 0.0;
     double min_mhz = 0.0;
     double max_mhz = 0.0;
     std::string governor;
     std::string driver;
 };
 
 class CpuInfoUtil {
 private:
     bool show_detailed = false;
     bool show_frequencies = false;
     bool show_load = false;
     bool show_topology = false;
     bool use_colors = true;
 
     std::string colorize(const std::string& text, const std::string& color) {
         if (!use_colors) return text;
         return color + text + Colors::RESET;
     }
 
     std::string formatFrequency(double mhz) {
         if (mhz >= 1000.0) {
             return std::to_string(static_cast<int>(mhz / 1000.0)) + "." + 
                    std::to_string(static_cast<int>(static_cast<int>(mhz) % 1000 / 100)) + " GHz";
         } else {
             return std::to_string(static_cast<int>(mhz)) + " MHz";
         }
     }
 
     CpuInfo getCpuInfo() {
         CpuInfo info;
         std::ifstream file("/proc/cpuinfo");
         std::string line;
         std::map<std::string, int> core_count;
         
         while (std::getline(file, line)) {
             size_t colon = line.find(':');
             if (colon == std::string::npos) continue;
             
             std::string key = line.substr(0, colon);
             std::string value = line.substr(colon + 1);
             
             // Trim whitespace
             key.erase(key.find_last_not_of(" \t") + 1);
             value.erase(0, value.find_first_not_of(" \t"));
             
             if (key == "model name" && info.model_name.empty()) {
                 info.model_name = value;
             } else if (key == "vendor_id" && info.vendor_id.empty()) {
                 info.vendor_id = value;
             } else if (key == "cpu family" && info.cpu_family.empty()) {
                 info.cpu_family = value;
             } else if (key == "model" && info.model.empty()) {
                 info.model = value;
             } else if (key == "stepping" && info.stepping.empty()) {
                 info.stepping = value;
             } else if (key == "microcode" && info.microcode.empty()) {
                 info.microcode = value;
             } else if (key == "cache size" && info.cache_size.empty()) {
                 info.cache_size = value;
             } else if (key == "cpu MHz" && info.cpu_mhz == 0.0) {
                 info.cpu_mhz = std::stod(value);
             } else if (key == "siblings") {
                 info.siblings = std::stoi(value);
             } else if (key == "core id") {
                 core_count[value]++;
             } else if (key == "processor") {
                 info.logical_cores++;
             } else if (key == "flags" && info.flags.empty()) {
                 std::istringstream iss(value);
                 std::string flag;
                 while (iss >> flag) {
                     info.flags.push_back(flag);
                 }
             }
         }
         
         info.physical_cores = core_count.size();
         if (info.physical_cores == 0) {
             info.physical_cores = info.logical_cores;
         }
         
         return info;
     }
 
     CpuLoad getCpuLoad() {
         CpuLoad load;
         
         // Get load average
         std::ifstream loadavg("/proc/loadavg");
         if (loadavg.is_open()) {
             loadavg >> load.load1 >> load.load5 >> load.load15;
         }
         
         // Get CPU statistics
         std::ifstream stat("/proc/stat");
         std::string line;
         if (stat.is_open() && std::getline(stat, line)) {
             std::istringstream iss(line);
             std::string cpu;
             iss >> cpu >> load.user >> load.nice >> load.system >> load.idle 
                 >> load.iowait >> load.irq >> load.softirq;
             
             unsigned long long total = load.user + load.nice + load.system + 
                                      load.idle + load.iowait + load.irq + load.softirq;
             unsigned long long used = total - load.idle - load.iowait;
             
             if (total > 0) {
                 load.cpu_usage = (double)used / total * 100.0;
             }
         }
         
         return load;
     }
 
     std::vector<CpuFrequency> getCpuFrequencies() {
         std::vector<CpuFrequency> frequencies;
         
         try {
             for (const auto& entry : fs::directory_iterator("/sys/devices/system/cpu/")) {
                 std::string dirname = entry.path().filename();
                 if (dirname.find("cpu") != 0 || !std::isdigit(dirname[3])) continue;
                 
                 std::string cpufreq_path = entry.path() / "cpufreq";
                 if (!fs::exists(cpufreq_path)) continue;
                 
                 CpuFrequency freq;
                 
                 // Current frequency
                 std::ifstream cur_freq(cpufreq_path + "/scaling_cur_freq");
                 if (cur_freq.is_open()) {
                     unsigned long khz;
                     cur_freq >> khz;
                     freq.current_mhz = khz / 1000.0;
                 }
                 
                 // Min frequency
                 std::ifstream min_freq(cpufreq_path + "/scaling_min_freq");
                 if (min_freq.is_open()) {
                     unsigned long khz;
                     min_freq >> khz;
                     freq.min_mhz = khz / 1000.0;
                 }
                 
                 // Max frequency
                 std::ifstream max_freq(cpufreq_path + "/scaling_max_freq");
                 if (max_freq.is_open()) {
                     unsigned long khz;
                     max_freq >> khz;
                     freq.max_mhz = khz / 1000.0;
                 }
                 
                 // Governor
                 std::ifstream gov(cpufreq_path + "/scaling_governor");
                 if (gov.is_open()) {
                     std::getline(gov, freq.governor);
                 }
                 
                 // Driver
                 std::ifstream drv(cpufreq_path + "/scaling_driver");
                 if (drv.is_open()) {
                     std::getline(drv, freq.driver);
                 }
                 
                 frequencies.push_back(freq);
                 break; // Just get the first one for summary
             }
         } catch (const std::exception& e) {
             // Ignore errors
         }
         
         return frequencies;
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
         CpuInfo info = getCpuInfo();
         
         printSeparator("CPU Information");
         
         if (!info.model_name.empty()) {
             std::cout << std::left << std::setw(18) << "Model:" 
                       << info.model_name << std::endl;
         }
         
         if (!info.vendor_id.empty()) {
             std::cout << std::left << std::setw(18) << "Vendor:" 
                       << info.vendor_id << std::endl;
         }
         
         if (info.logical_cores > 0) {
             std::cout << std::left << std::setw(18) << "Logical cores:" 
                       << info.logical_cores << std::endl;
         }
         
         if (info.physical_cores > 0 && info.physical_cores != info.logical_cores) {
             std::cout << std::left << std::setw(18) << "Physical cores:" 
                       << info.physical_cores << std::endl;
         }
         
         if (info.cpu_mhz > 0) {
             std::cout << std::left << std::setw(18) << "Base frequency:" 
                       << formatFrequency(info.cpu_mhz) << std::endl;
         }
         
         if (!info.cache_size.empty()) {
             std::cout << std::left << std::setw(18) << "Cache size:" 
                       << info.cache_size << std::endl;
         }
         
         if (show_detailed) {
             if (!info.cpu_family.empty()) {
                 std::cout << std::left << std::setw(18) << "CPU family:" 
                           << info.cpu_family << std::endl;
             }
             
             if (!info.model.empty()) {
                 std::cout << std::left << std::setw(18) << "Model:" 
                           << info.model << std::endl;
             }
             
             if (!info.stepping.empty()) {
                 std::cout << std::left << std::setw(18) << "Stepping:" 
                           << info.stepping << std::endl;
             }
             
             if (!info.microcode.empty()) {
                 std::cout << std::left << std::setw(18) << "Microcode:" 
                           << info.microcode << std::endl;
             }
             
             if (!info.flags.empty()) {
                 std::cout << std::left << std::setw(18) << "Features:" << std::endl;
                 
                 // Print flags in columns
                 const int cols = 4;
                 const int col_width = 15;
                 for (size_t i = 0; i < info.flags.size(); i += cols) {
                     std::cout << "  ";
                     for (int j = 0; j < cols && i + j < info.flags.size(); ++j) {
                         std::cout << std::left << std::setw(col_width) << info.flags[i + j];
                     }
                     std::cout << std::endl;
                 }
             }
         }
     }
 
     void printLoadInfo() {
         CpuLoad load = getCpuLoad();
         
         std::cout << std::endl;
         printSeparator("CPU Load");
         
         std::cout << std::left << std::setw(18) << "Load average:" 
                   << std::fixed << std::setprecision(2)
                   << load.load1 << ", " << load.load5 << ", " << load.load15 << std::endl;
         
         std::cout << std::left << std::setw(18) << "CPU usage:" 
                   << std::fixed << std::setprecision(1) << load.cpu_usage << "%" << std::endl;
         
         if (show_detailed) {
             std::cout << std::left << std::setw(18) << "User time:" 
                       << load.user << " jiffies" << std::endl;
             std::cout << std::left << std::setw(18) << "System time:" 
                       << load.system << " jiffies" << std::endl;
             std::cout << std::left << std::setw(18) << "Idle time:" 
                       << load.idle << " jiffies" << std::endl;
             std::cout << std::left << std::setw(18) << "I/O wait time:" 
                       << load.iowait << " jiffies" << std::endl;
         }
     }
 
     void printFrequencyInfo() {
         auto frequencies = getCpuFrequencies();
         
         std::cout << std::endl;
         printSeparator("CPU Frequency");
         
         if (frequencies.empty()) {
             std::cout << colorize("Warning: CPU frequency information not available", Colors::YELLOW) << std::endl;
             std::cout << "This may require cpufreq driver support or root privileges" << std::endl;
             return;
         }
         
         const auto& freq = frequencies[0];
         
         if (freq.current_mhz > 0) {
             std::cout << std::left << std::setw(18) << "Current:" 
                       << formatFrequency(freq.current_mhz) << std::endl;
         }
         
         if (freq.min_mhz > 0) {
             std::cout << std::left << std::setw(18) << "Minimum:" 
                       << formatFrequency(freq.min_mhz) << std::endl;
         }
         
         if (freq.max_mhz > 0) {
             std::cout << std::left << std::setw(18) << "Maximum:" 
                       << formatFrequency(freq.max_mhz) << std::endl;
         }
         
         if (!freq.governor.empty()) {
             std::cout << std::left << std::setw(18) << "Governor:" 
                       << freq.governor << std::endl;
         }
         
         if (!freq.driver.empty()) {
             std::cout << std::left << std::setw(18) << "Driver:" 
                       << freq.driver << std::endl;
         }
     }
 
     void printTopologyInfo() {
         std::cout << std::endl;
         printSeparator("CPU Topology");
         
         try {
             std::map<int, std::vector<int>> sockets;
             std::map<int, std::vector<int>> cores;
             
             for (const auto& entry : fs::directory_iterator("/sys/devices/system/cpu/")) {
                 std::string dirname = entry.path().filename();
                 if (dirname.find("cpu") != 0 || !std::isdigit(dirname[3])) continue;
                 
                 int cpu_num = std::stoi(dirname.substr(3));
                 
                 // Physical package ID (socket)
                 std::ifstream pkg_file(entry.path() / "topology/physical_package_id");
                 if (pkg_file.is_open()) {
                     int pkg_id;
                     pkg_file >> pkg_id;
                     sockets[pkg_id].push_back(cpu_num);
                 }
                 
                 // Core ID
                 std::ifstream core_file(entry.path() / "topology/core_id");
                 if (core_file.is_open()) {
                     int core_id;
                     core_file >> core_id;
                     cores[core_id].push_back(cpu_num);
                 }
             }
             
             std::cout << std::left << std::setw(18) << "Sockets:" 
                       << sockets.size() << std::endl;
             
             std::cout << std::left << std::setw(18) << "Cores per socket:" 
                       << cores.size() / (sockets.empty() ? 1 : sockets.size()) << std::endl;
             
             if (show_detailed) {
                 for (const auto& socket : sockets) {
                     std::cout << "Socket " << socket.first << ": CPUs ";
                     for (size_t i = 0; i < socket.second.size(); ++i) {
                         if (i > 0) std::cout << ", ";
                         std::cout << socket.second[i];
                     }
                     std::cout << std::endl;
                 }
             }
             
         } catch (const std::exception& e) {
             std::cout << colorize("Warning: Could not read topology information", Colors::YELLOW) << std::endl;
         }
     }
 
 public:
     CpuInfoUtil() {
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
             } else if (arg == "--detailed" || arg == "-d") {
                 show_detailed = true;
             } else if (arg == "--frequencies" || arg == "-f") {
                 show_frequencies = true;
             } else if (arg == "--load" || arg == "-l") {
                 show_load = true;
             } else if (arg == "--topology" || arg == "-t") {
                 show_topology = true;
             } else if (arg == "--all" || arg == "-a") {
                 show_detailed = show_frequencies = show_load = show_topology = true;
             } else if (arg == "--no-color") {
                 use_colors = false;
             } else {
                 std::cerr << colorize("cpuinfo: invalid option -- '" + arg + "'", Colors::RED) << std::endl;
                 std::cerr << "Try 'cpuinfo --help' for more information." << std::endl;
                 exit(1);
             }
         }
     }
 
     void printVersion() {
         std::cout << "cpuinfo (QCO InfoUtils) 1.0" << std::endl;
         std::cout << "Copyright (C) 2025 AnmiTaliDev" << std::endl;
         std::cout << "License Apache 2.0: Apache License version 2.0" << std::endl;
         std::cout << "This is free software: you are free to change and redistribute it." << std::endl;
         std::cout << "There is NO WARRANTY, to the extent permitted by law." << std::endl;
     }
 
     void printHelp() {
         std::cout << "Usage: cpuinfo [OPTION]..." << std::endl;
         std::cout << "Display information about system CPU." << std::endl;
         std::cout << std::endl;
         std::cout << "  -a, --all         display all available information" << std::endl;
         std::cout << "  -d, --detailed    show detailed CPU information" << std::endl;
         std::cout << "  -f, --frequencies show CPU frequency information" << std::endl;
         std::cout << "  -h, --help        display this help and exit" << std::endl;
         std::cout << "  -l, --load        show CPU load information" << std::endl;
         std::cout << "      --no-color    disable colored output" << std::endl;
         std::cout << "  -t, --topology    show CPU topology information" << std::endl;
         std::cout << "  -V, --version     output version information and exit" << std::endl;
         std::cout << std::endl;
         std::cout << "Examples:" << std::endl;
         std::cout << "  cpuinfo           Show basic CPU information" << std::endl;
         std::cout << "  cpuinfo -a        Show comprehensive CPU report" << std::endl;
         std::cout << "  cpuinfo -l        Show CPU information with load" << std::endl;
         std::cout << std::endl;
         std::cout << "QCO InfoUtils home page: <https://github.com/Qainar-Projects/infoutils>" << std::endl;
     }
 
     void run() {
         printGeneralInfo();
         
         if (show_load) {
             printLoadInfo();
         }
         
         if (show_frequencies) {
             printFrequencyInfo();
         }
         
         if (show_topology) {
             printTopologyInfo();
         }
     }
 };
 
 int main(int argc, char* argv[]) {
     try {
         CpuInfoUtil util;
         util.parseArgs(argc, argv);
         util.run();
         return 0;
     } catch (const std::exception& e) {
         std::cerr << "cpuinfo: " << e.what() << std::endl;
         return 1;
     }
 }