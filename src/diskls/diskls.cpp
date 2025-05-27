/*
 * diskls - Disk Information Utility
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
 #include <sys/statvfs.h>
 #include <sys/stat.h>
 
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
 
 struct DiskInfo {
     std::string device;
     std::string model;
     std::string vendor;
     std::string type; // HDD/SSD/NVMe
     unsigned long long size_bytes = 0;
     std::string size_human;
     bool removable = false;
     bool rotational = true;
     std::string scheduler;
     unsigned int queue_depth = 0;
     std::vector<std::string> partitions;
 };
 
 struct PartitionInfo {
     std::string device;
     std::string mountpoint;
     std::string filesystem;
     unsigned long long total_bytes = 0;
     unsigned long long used_bytes = 0;
     unsigned long long available_bytes = 0;
     double usage_percent = 0.0;
     std::string mount_options;
 };
 
 struct DiskStats {
     std::string device;
     unsigned long long reads_completed = 0;
     unsigned long long reads_merged = 0;
     unsigned long long sectors_read = 0;
     unsigned long long time_reading = 0;
     unsigned long long writes_completed = 0;
     unsigned long long writes_merged = 0;
     unsigned long long sectors_written = 0;
     unsigned long long time_writing = 0;
     unsigned long long io_in_progress = 0;
     unsigned long long time_io = 0;
     unsigned long long weighted_time_io = 0;
 };
 
 class DiskLsUtil {
 private:
     bool show_detailed = false;
     bool show_usage = false;
     bool show_mounts = false;
     bool show_types = false;
     bool use_colors = true;
 
     std::string colorize(const std::string& text, const std::string& color) {
         if (!use_colors) return text;
         return color + text + Colors::RESET;
     }
 
     std::string formatBytes(unsigned long long bytes) {
         if (bytes == 0) return "0 B";
         
         const char* units[] = {"B", "KB", "MB", "GB", "TB", "PB"};
         int unit = 0;
         double size = static_cast<double>(bytes);
         
         while (size >= 1024.0 && unit < 5) {
             size /= 1024.0;
             unit++;
         }
         
         std::ostringstream oss;
         if (unit == 0) {
             oss << static_cast<unsigned long long>(size) << " " << units[unit];
         } else {
             oss << std::fixed << std::setprecision(1) << size << " " << units[unit];
         }
         return oss.str();
     }
 
     std::vector<DiskInfo> getDiskInfo() {
         std::vector<DiskInfo> disks;
         
         try {
             // Read from /sys/block/
             for (const auto& entry : fs::directory_iterator("/sys/block/")) {
                 std::string device_name = entry.path().filename();
                 
                 // Skip loop devices and ram disks by default
                 if (device_name.find("loop") == 0 || device_name.find("ram") == 0) {
                     continue;
                 }
                 
                 DiskInfo disk;
                 disk.device = "/dev/" + device_name;
                 
                 // Read size
                 std::ifstream size_file(entry.path() / "size");
                 if (size_file.is_open()) {
                     unsigned long long sectors;
                     size_file >> sectors;
                     disk.size_bytes = sectors * 512; // Assume 512 byte sectors
                     disk.size_human = formatBytes(disk.size_bytes);
                 }
                 
                 // Read model
                 std::ifstream model_file(entry.path() / "device/model");
                 if (model_file.is_open()) {
                     std::getline(model_file, disk.model);
                     // Trim whitespace
                     disk.model.erase(disk.model.find_last_not_of(" \t\n\r") + 1);
                 }
                 
                 // Read vendor
                 std::ifstream vendor_file(entry.path() / "device/vendor");
                 if (vendor_file.is_open()) {
                     std::getline(vendor_file, disk.vendor);
                     disk.vendor.erase(disk.vendor.find_last_not_of(" \t\n\r") + 1);
                 }
                 
                 // Check if removable
                 std::ifstream removable_file(entry.path() / "removable");
                 if (removable_file.is_open()) {
                     int removable_flag;
                     removable_file >> removable_flag;
                     disk.removable = (removable_flag == 1);
                 }
                 
                 // Check if rotational (SSD vs HDD)
                 std::ifstream rotational_file(entry.path() / "queue/rotational");
                 if (rotational_file.is_open()) {
                     int rotational_flag;
                     rotational_file >> rotational_flag;
                     disk.rotational = (rotational_flag == 1);
                 }
                 
                 // Determine disk type
                 if (device_name.find("nvme") == 0) {
                     disk.type = "NVMe";
                 } else if (!disk.rotational) {
                     disk.type = "SSD";
                 } else {
                     disk.type = "HDD";
                 }
                 
                 // Read scheduler
                 std::ifstream scheduler_file(entry.path() / "queue/scheduler");
                 if (scheduler_file.is_open()) {
                     std::string scheduler_line;
                     std::getline(scheduler_file, scheduler_line);
                     
                     // Extract current scheduler (between square brackets)
                     size_t start = scheduler_line.find('[');
                     size_t end = scheduler_line.find(']');
                     if (start != std::string::npos && end != std::string::npos) {
                         disk.scheduler = scheduler_line.substr(start + 1, end - start - 1);
                     }
                 }
                 
                 // Read queue depth
                 std::ifstream queue_file(entry.path() / "queue/nr_requests");
                 if (queue_file.is_open()) {
                     queue_file >> disk.queue_depth;
                 }
                 
                 // Find partitions
                 try {
                     for (const auto& part_entry : fs::directory_iterator(entry.path())) {
                         std::string part_name = part_entry.path().filename();
                         if (part_name.find(device_name) == 0 && part_name != device_name) {
                             disk.partitions.push_back("/dev/" + part_name);
                         }
                     }
                 } catch (const std::exception& e) {
                     // Ignore partition enumeration errors
                 }
                 
                 disks.push_back(disk);
             }
         } catch (const std::exception& e) {
             std::cerr << colorize("Warning: Could not read all disk information", Colors::YELLOW) << std::endl;
         }
         
         return disks;
     }
 
     std::vector<PartitionInfo> getPartitionInfo() {
         std::vector<PartitionInfo> partitions;
         
         // Read from /proc/mounts
         std::ifstream mounts_file("/proc/mounts");
         std::string line;
         
         while (std::getline(mounts_file, line)) {
             std::istringstream iss(line);
             std::string device, mountpoint, filesystem, options;
             
             if (iss >> device >> mountpoint >> filesystem >> options) {
                 // Skip special filesystems
                 if (device.find("/dev/") != 0) continue;
                 if (filesystem == "proc" || filesystem == "sysfs" || 
                     filesystem == "devtmpfs" || filesystem == "tmpfs") continue;
                 
                 PartitionInfo part;
                 part.device = device;
                 part.mountpoint = mountpoint;
                 part.filesystem = filesystem;
                 part.mount_options = options;
                 
                 // Get space usage
                 struct statvfs stat;
                 if (statvfs(mountpoint.c_str(), &stat) == 0) {
                     part.total_bytes = static_cast<unsigned long long>(stat.f_blocks) * stat.f_frsize;
                     part.available_bytes = static_cast<unsigned long long>(stat.f_bavail) * stat.f_frsize;
                     part.used_bytes = (static_cast<unsigned long long>(stat.f_blocks) - 
                                      static_cast<unsigned long long>(stat.f_bfree)) * stat.f_frsize;
                     
                     if (part.total_bytes > 0) {
                         part.usage_percent = static_cast<double>(part.used_bytes) / part.total_bytes * 100.0;
                     }
                 }
                 
                 partitions.push_back(part);
             }
         }
         
         return partitions;
     }
 
     std::map<std::string, DiskStats> getDiskStats() {
         std::map<std::string, DiskStats> stats;
         
         std::ifstream diskstats_file("/proc/diskstats");
         std::string line;
         
         while (std::getline(diskstats_file, line)) {
             std::istringstream iss(line);
             unsigned int major, minor;
             std::string device;
             DiskStats stat;
             
             if (iss >> major >> minor >> device >> 
                 stat.reads_completed >> stat.reads_merged >> stat.sectors_read >> stat.time_reading >>
                 stat.writes_completed >> stat.writes_merged >> stat.sectors_written >> stat.time_writing >>
                 stat.io_in_progress >> stat.time_io >> stat.weighted_time_io) {
                 
                 stat.device = device;
                 stats[device] = stat;
             }
         }
         
         return stats;
     }
 
     void printSeparator(const std::string& title = "") {
         if (title.empty()) {
             std::cout << std::string(70, '-') << std::endl;
         } else {
             std::cout << colorize(title, Colors::BOLD) << std::endl;
             std::cout << std::string(title.length(), '=') << std::endl;
         }
     }
 
     void printDiskInfo() {
         auto disks = getDiskInfo();
         
         printSeparator("Disk Information");
         
         if (disks.empty()) {
             std::cout << "No disks found" << std::endl;
             return;
         }
         
         for (const auto& disk : disks) {
             std::cout << colorize(disk.device, Colors::BOLD) << std::endl;
             
             if (!disk.model.empty()) {
                 std::cout << "  " << std::left << std::setw(16) << "Model:" 
                           << disk.model << std::endl;
             }
             
             if (!disk.vendor.empty()) {
                 std::cout << "  " << std::left << std::setw(16) << "Vendor:" 
                           << disk.vendor << std::endl;
             }
             
             std::cout << "  " << std::left << std::setw(16) << "Type:" 
                       << disk.type << std::endl;
             
             std::cout << "  " << std::left << std::setw(16) << "Size:" 
                       << disk.size_human;
             
             if (show_detailed) {
                 std::cout << colorize(" (" + std::to_string(disk.size_bytes) + " bytes)", Colors::DIM);
             }
             std::cout << std::endl;
             
             if (disk.removable) {
                 std::cout << "  " << std::left << std::setw(16) << "Removable:" 
                           << "Yes" << std::endl;
             }
             
             if (show_detailed) {
                 if (!disk.scheduler.empty()) {
                     std::cout << "  " << std::left << std::setw(16) << "Scheduler:" 
                               << disk.scheduler << std::endl;
                 }
                 
                 if (disk.queue_depth > 0) {
                     std::cout << "  " << std::left << std::setw(16) << "Queue depth:" 
                               << disk.queue_depth << std::endl;
                 }
                 
                 if (!disk.partitions.empty()) {
                     std::cout << "  " << std::left << std::setw(16) << "Partitions:";
                     for (size_t i = 0; i < disk.partitions.size(); ++i) {
                         if (i > 0) std::cout << ", ";
                         std::cout << fs::path(disk.partitions[i]).filename().string();
                     }
                     std::cout << std::endl;
                 }
             }
             
             std::cout << std::endl;
         }
     }
 
     void printUsageInfo() {
         auto partitions = getPartitionInfo();
         
         std::cout << std::endl;
         printSeparator("Disk Usage");
         
         if (partitions.empty()) {
             std::cout << "No mounted partitions found" << std::endl;
             return;
         }
         
         std::cout << std::left 
                   << std::setw(20) << "DEVICE"
                   << std::setw(15) << "SIZE"
                   << std::setw(15) << "USED"
                   << std::setw(15) << "AVAILABLE"
                   << std::setw(8) << "USE%"
                   << "MOUNTED ON" << std::endl;
         printSeparator();
         
         for (const auto& part : partitions) {
             std::cout << std::left 
                       << std::setw(20) << part.device.substr(0, 19)
                       << std::setw(15) << formatBytes(part.total_bytes)
                       << std::setw(15) << formatBytes(part.used_bytes)
                       << std::setw(15) << formatBytes(part.available_bytes)
                       << std::setw(7) << std::fixed << std::setprecision(0) << part.usage_percent << "%"
                       << part.mountpoint << std::endl;
         }
     }
 
     void printMountInfo() {
         auto partitions = getPartitionInfo();
         
         std::cout << std::endl;
         printSeparator("Mount Information");
         
         for (const auto& part : partitions) {
             std::cout << colorize(part.device, Colors::BOLD) << std::endl;
             std::cout << "  " << std::left << std::setw(16) << "Mount point:" 
                       << part.mountpoint << std::endl;
             std::cout << "  " << std::left << std::setw(16) << "Filesystem:" 
                       << part.filesystem << std::endl;
             
             if (show_detailed) {
                 std::cout << "  " << std::left << std::setw(16) << "Mount options:" 
                           << part.mount_options << std::endl;
             }
             
             std::cout << std::endl;
         }
     }
 
     void printTypeInfo() {
         auto disks = getDiskInfo();
         auto partitions = getPartitionInfo();
         
         std::cout << std::endl;
         printSeparator("Disk Types and Filesystems");
         
         // Group disks by type
         std::map<std::string, std::vector<std::string>> disk_types;
         for (const auto& disk : disks) {
             disk_types[disk.type].push_back(fs::path(disk.device).filename().string());
         }
         
         std::cout << colorize("Disk Types:", Colors::BOLD) << std::endl;
         for (const auto& type : disk_types) {
             std::cout << "  " << std::left << std::setw(16) << (type.first + ":") << std::endl;
             std::cout << "    ";
             for (size_t i = 0; i < type.second.size(); ++i) {
                 if (i > 0) std::cout << ", ";
                 std::cout << type.second[i];
             }
             std::cout << std::endl;
         }
         
         std::cout << std::endl;
         
         // Group filesystems
         std::map<std::string, std::vector<std::string>> filesystems;
         for (const auto& part : partitions) {
             filesystems[part.filesystem].push_back(fs::path(part.device).filename().string());
         }
         
         std::cout << colorize("Filesystems:", Colors::BOLD) << std::endl;
         for (const auto& fs_type : filesystems) {
             std::cout << "  " << std::left << std::setw(16) << (fs_type.first + ":") << std::endl;
             std::cout << "    ";
             for (size_t i = 0; i < fs_type.second.size(); ++i) {
                 if (i > 0) std::cout << ", ";
                 std::cout << fs_type.second[i];
             }
             std::cout << std::endl;
         }
     }
 
 public:
     DiskLsUtil() {
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
             } else if (arg == "--usage" || arg == "-u") {
                 show_usage = true;
             } else if (arg == "--mounts" || arg == "-m") {
                 show_mounts = true;
             } else if (arg == "--types" || arg == "-t") {
                 show_types = true;
             } else if (arg == "--all" || arg == "-a") {
                 show_detailed = show_usage = show_mounts = show_types = true;
             } else if (arg == "--no-color") {
                 use_colors = false;
             } else {
                 std::cerr << colorize("diskls: invalid option -- '" + arg + "'", Colors::RED) << std::endl;
                 std::cerr << "Try 'diskls --help' for more information." << std::endl;
                 exit(1);
             }
         }
     }
 
     void printVersion() {
         std::cout << "diskls (QCO InfoUtils) 1.0" << std::endl;
         std::cout << "Copyright (C) 2025 AnmiTaliDev" << std::endl;
         std::cout << "License Apache 2.0: Apache License version 2.0" << std::endl;
         std::cout << "This is free software: you are free to change and redistribute it." << std::endl;
         std::cout << "There is NO WARRANTY, to the extent permitted by law." << std::endl;
     }
 
     void printHelp() {
         std::cout << "Usage: diskls [OPTION]..." << std::endl;
         std::cout << "Display information about system disks and storage." << std::endl;
         std::cout << std::endl;
         std::cout << "  -a, --all         display all available information" << std::endl;
         std::cout << "  -d, --detailed    show detailed disk information" << std::endl;
         std::cout << "  -h, --help        display this help and exit" << std::endl;
         std::cout << "  -m, --mounts      show mount point information" << std::endl;
         std::cout << "      --no-color    disable colored output" << std::endl;
         std::cout << "  -t, --types       show disk types and filesystems" << std::endl;
         std::cout << "  -u, --usage       show disk space usage" << std::endl;
         std::cout << "  -V, --version     output version information and exit" << std::endl;
         std::cout << std::endl;
         std::cout << "Examples:" << std::endl;
         std::cout << "  diskls            Show basic disk information" << std::endl;
         std::cout << "  diskls -a         Show comprehensive disk report" << std::endl;
         std::cout << "  diskls -u         Show disk usage information" << std::endl;
         std::cout << std::endl;
         std::cout << "QCO InfoUtils home page: <https://github.com/Qainar-Projects/infoutils>" << std::endl;
     }
 
     void run() {
         printDiskInfo();
         
         if (show_usage) {
             printUsageInfo();
         }
         
         if (show_mounts) {
             printMountInfo();
         }
         
         if (show_types) {
             printTypeInfo();
         }
     }
 };
 
 int main(int argc, char* argv[]) {
     try {
         DiskLsUtil util;
         util.parseArgs(argc, argv);
         util.run();
         return 0;
     } catch (const std::exception& e) {
         std::cerr << "diskls: " << e.what() << std::endl;
         return 1;
     }
 }