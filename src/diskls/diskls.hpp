/*
 * diskls - Disk Information Utility
 * Part of QCO InfoUtils (Qainrat Code Organization InfoUtils)
 * Author: AnmiTaliDev
 * License: Apache 2.0
 */

 #ifndef DISKLS_HPP
 #define DISKLS_HPP
 
 #include <string>
 #include <vector>
 #include <map>
 
 // Forward declarations
 struct DiskInfo;
 struct PartitionInfo;
 struct DiskStats;
 
 // ANSI Color codes namespace
 namespace Colors {
     extern const std::string RESET;
     extern const std::string BOLD;
     extern const std::string RED;
     extern const std::string GREEN;
     extern const std::string YELLOW;
     extern const std::string BLUE;
     extern const std::string MAGENTA;
     extern const std::string CYAN;
     extern const std::string WHITE;
     extern const std::string DIM;
 }
 
 /**
  * Structure to hold disk information from /sys/block/
  */
 struct DiskInfo {
     std::string device;
     std::string model;
     std::string vendor;
     std::string type; // HDD/SSD/NVMe
     unsigned long long size_bytes;
     std::string size_human;
     bool removable;
     bool rotational;
     std::string scheduler;
     unsigned int queue_depth;
     std::vector<std::string> partitions;
 
     DiskInfo() : size_bytes(0), removable(false), rotational(true), queue_depth(0) {}
 };
 
 /**
  * Structure to hold partition information from /proc/mounts
  */
 struct PartitionInfo {
     std::string device;
     std::string mountpoint;
     std::string filesystem;
     unsigned long long total_bytes;
     unsigned long long used_bytes;
     unsigned long long available_bytes;
     double usage_percent;
     std::string mount_options;
 
     PartitionInfo() : total_bytes(0), used_bytes(0), available_bytes(0), usage_percent(0.0) {}
 };
 
 /**
  * Structure to hold disk statistics from /proc/diskstats
  */
 struct DiskStats {
     std::string device;
     unsigned long long reads_completed;
     unsigned long long reads_merged;
     unsigned long long sectors_read;
     unsigned long long time_reading;
     unsigned long long writes_completed;
     unsigned long long writes_merged;
     unsigned long long sectors_written;
     unsigned long long time_writing;
     unsigned long long io_in_progress;
     unsigned long long time_io;
     unsigned long long weighted_time_io;
 
     DiskStats() : reads_completed(0), reads_merged(0), sectors_read(0), time_reading(0),
                   writes_completed(0), writes_merged(0), sectors_written(0), time_writing(0),
                   io_in_progress(0), time_io(0), weighted_time_io(0) {}
 };
 
 /**
  * Main utility class for disk information display
  */
 class DiskLsUtil {
 private:
     // Configuration flags
     bool show_detailed;
     bool show_usage;
     bool show_mounts;
     bool show_types;
     bool use_colors;
 
     /**
      * Apply color formatting to text if colors are enabled
      * @param text Text to colorize
      * @param color ANSI color code
      * @return Colorized text or plain text if colors disabled
      */
     std::string colorize(const std::string& text, const std::string& color);
 
     /**
      * Format bytes with human-readable units (B, KB, MB, GB, TB, PB)
      * @param bytes Size in bytes
      * @return Formatted string with appropriate unit
      */
     std::string formatBytes(unsigned long long bytes);
 
     /**
      * Read disk information from /sys/block/
      * @return Vector of DiskInfo structures with disk details
      */
     std::vector<DiskInfo> getDiskInfo();
 
     /**
      * Read partition information from /proc/mounts
      * @return Vector of PartitionInfo structures with mount information
      */
     std::vector<PartitionInfo> getPartitionInfo();
 
     /**
      * Read disk statistics from /proc/diskstats
      * @return Map of device names to DiskStats structures
      */
     std::map<std::string, DiskStats> getDiskStats();
 
     /**
      * Print section separator with optional title
      * @param title Optional section title
      */
     void printSeparator(const std::string& title = "");
 
     /**
      * Display general disk information
      */
     void printDiskInfo();
 
     /**
      * Display disk usage information (similar to df command)
      */
     void printUsageInfo();
 
     /**
      * Display mount point information
      */
     void printMountInfo();
 
     /**
      * Display disk types and filesystem information
      */
     void printTypeInfo();
 
 public:
     /**
      * Constructor - initializes utility state
      */
     DiskLsUtil();
 
     /**
      * Destructor
      */
     ~DiskLsUtil() = default;
 
     // Disable copy constructor and assignment operator
     DiskLsUtil(const DiskLsUtil&) = delete;
     DiskLsUtil& operator=(const DiskLsUtil&) = delete;
 
     /**
      * Parse command line arguments
      * @param argc Argument count
      * @param argv Argument values
      */
     void parseArgs(int argc, char* argv[]);
 
     /**
      * Print version information
      */
     void printVersion();
 
     /**
      * Print help message
      */
     void printHelp();
 
     /**
      * Run the main program logic
      */
     void run();
 
     // Accessor methods for configuration flags
     bool isShowDetailed() const { return show_detailed; }
     bool isShowUsage() const { return show_usage; }
     bool isShowMounts() const { return show_mounts; }
     bool isShowTypes() const { return show_types; }
     bool isUseColors() const { return use_colors; }
 
     // Setter methods for testing purposes
     void setShowDetailed(bool value) { show_detailed = value; }
     void setShowUsage(bool value) { show_usage = value; }
     void setShowMounts(bool value) { show_mounts = value; }
     void setShowTypes(bool value) { show_types = value; }
     void setUseColors(bool value) { use_colors = value; }
 };
 
 /**
  * Utility functions namespace
  */
 namespace DiskLsUtils {
     /**
      * Check if output terminal supports colors
      * @return true if colors are supported, false otherwise
      */
     bool supportsColors();
 
     /**
      * Read a single line from a file
      * @param filepath Path to the file
      * @param line Reference to store the read line
      * @return true if successful, false otherwise
      */
     bool readFileLine(const std::string& filepath, std::string& line);
 
     /**
      * Parse device information from /sys/block/ path
      * @param device_path Path to device in /sys/block/
      * @param disk_info Reference to DiskInfo structure to fill
      * @return true if parsing successful, false otherwise
      */
     bool parseDeviceInfo(const std::string& device_path, DiskInfo& disk_info);
 
     /**
      * Convert bytes to human-readable format
      * @param bytes Size in bytes
      * @param binary Use binary (1024) or decimal (1000) base
      * @return Formatted size string
      */
     std::string bytesToHuman(unsigned long long bytes, bool binary = true);
 
     /**
      * Determine disk type from device name and characteristics
      * @param device_name Device name (e.g., "sda", "nvme0n1")
      * @param rotational Whether disk is rotational
      * @return Disk type string (HDD/SSD/NVMe)
      */
     std::string determineDiskType(const std::string& device_name, bool rotational);
 
     /**
      * Check if device is a physical disk (not partition or virtual device)
      * @param device_name Device name
      * @return true if it's a physical disk, false otherwise
      */
     bool isPhysicalDisk(const std::string& device_name);
 
     /**
      * Get disk usage percentage with color coding
      * @param usage_percent Usage percentage
      * @param use_colors Whether to apply colors
      * @return Formatted usage string with optional colors
      */
     std::string formatUsagePercent(double usage_percent, bool use_colors = true);
 
     /**
      * Parse mount options string into readable format
      * @param options Mount options string
      * @return Formatted options string
      */
     std::string formatMountOptions(const std::string& options);
 }
 
 // Version information
 namespace Version {
     constexpr const char* PROGRAM_NAME = "diskls";
     constexpr const char* VERSION = "1.0";
     constexpr const char* AUTHOR = "AnmiTaliDev";
     constexpr const char* LICENSE = "Apache 2.0";
     constexpr const char* ORGANIZATION = "QCO InfoUtils";
     constexpr const char* HOMEPAGE = "https://github.com/Qainar-Projects/infoutils";
 }
 
 // Program exit codes
 namespace ExitCodes {
     constexpr int SUCCESS = 0;
     constexpr int INVALID_OPTION = 1;
     constexpr int PERMISSION_DENIED = 2;
     constexpr int FILE_NOT_FOUND = 3;
     constexpr int RUNTIME_ERROR = 4;
 }
 
 #endif // DISKLS_HPP