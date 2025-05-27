/*
 * cpuinfo - CPU Information Utility
 * Part of QCO InfoUtils (Qainrat Code Organization InfoUtils)
 * Author: AnmiTaliDev
 * License: Apache 2.0
 */

 #ifndef CPUINFO_HPP
 #define CPUINFO_HPP
 
 #include <string>
 #include <vector>
 
 // Forward declarations
 struct CpuInfo;
 struct CpuLoad;
 struct CpuFrequency;
 
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
  * Structure to hold CPU information from /proc/cpuinfo
  */
 struct CpuInfo {
     std::string model_name;
     std::string vendor_id;
     std::string cpu_family;
     std::string model;
     std::string stepping;
     std::string microcode;
     std::string cache_size;
     std::vector<std::string> flags;
     double cpu_mhz;
     int physical_cores;
     int logical_cores;
     int siblings;
     int core_id;
     std::string architecture;
     std::string byte_order;
     std::string virtualization;
 
     CpuInfo() : cpu_mhz(0.0), physical_cores(0), logical_cores(0), 
                 siblings(0), core_id(0) {}
 };
 
 /**
  * Structure to hold CPU load information
  */
 struct CpuLoad {
     double load1;
     double load5;
     double load15;
     double cpu_usage;
     unsigned long long user;
     unsigned long long nice;
     unsigned long long system;
     unsigned long long idle;
     unsigned long long iowait;
     unsigned long long irq;
     unsigned long long softirq;
 
     CpuLoad() : load1(0.0), load5(0.0), load15(0.0), cpu_usage(0.0),
                 user(0), nice(0), system(0), idle(0), iowait(0), irq(0), softirq(0) {}
 };
 
 /**
  * Structure to hold CPU frequency information
  */
 struct CpuFrequency {
     double current_mhz;
     double min_mhz;
     double max_mhz;
     std::string governor;
     std::string driver;
 
     CpuFrequency() : current_mhz(0.0), min_mhz(0.0), max_mhz(0.0) {}
 };
 
 /**
  * Main utility class for CPU information display
  */
 class CpuInfoUtil {
 private:
     // Configuration flags
     bool show_detailed;
     bool show_frequencies;
     bool show_load;
     bool show_topology;
     bool use_colors;
 
     /**
      * Apply color formatting to text if colors are enabled
      * @param text Text to colorize
      * @param color ANSI color code
      * @return Colorized text or plain text if colors disabled
      */
     std::string colorize(const std::string& text, const std::string& color);
 
     /**
      * Format frequency with appropriate units (MHz/GHz)
      * @param mhz Frequency in MHz
      * @return Formatted frequency string
      */
     std::string formatFrequency(double mhz);
 
     /**
      * Read CPU information from /proc/cpuinfo
      * @return CpuInfo structure with CPU details
      */
     CpuInfo getCpuInfo();
 
     /**
      * Read CPU load information from /proc/loadavg and /proc/stat
      * @return CpuLoad structure with load statistics
      */
     CpuLoad getCpuLoad();
 
     /**
      * Read CPU frequency information from /sys/devices/system/cpu/
      * @return Vector of CpuFrequency structures for each CPU
      */
     std::vector<CpuFrequency> getCpuFrequencies();
 
     /**
      * Print section separator with optional title
      * @param title Optional section title
      */
     void printSeparator(const std::string& title = "");
 
     /**
      * Display general CPU information
      */
     void printGeneralInfo();
 
     /**
      * Display CPU load information
      */
     void printLoadInfo();
 
     /**
      * Display CPU frequency information
      */
     void printFrequencyInfo();
 
     /**
      * Display CPU topology information
      */
     void printTopologyInfo();
 
 public:
     /**
      * Constructor - initializes utility state
      */
     CpuInfoUtil();
 
     /**
      * Destructor
      */
     ~CpuInfoUtil() = default;
 
     // Disable copy constructor and assignment operator
     CpuInfoUtil(const CpuInfoUtil&) = delete;
     CpuInfoUtil& operator=(const CpuInfoUtil&) = delete;
 
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
     bool isShowFrequencies() const { return show_frequencies; }
     bool isShowLoad() const { return show_load; }
     bool isShowTopology() const { return show_topology; }
     bool isUseColors() const { return use_colors; }
 
     // Setter methods for testing purposes
     void setShowDetailed(bool value) { show_detailed = value; }
     void setShowFrequencies(bool value) { show_frequencies = value; }
     void setShowLoad(bool value) { show_load = value; }
     void setShowTopology(bool value) { show_topology = value; }
     void setUseColors(bool value) { use_colors = value; }
 };
 
 /**
  * Utility functions namespace
  */
 namespace CpuInfoUtils {
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
      * Parse a key-value pair from /proc/cpuinfo format
      * @param line Line to parse
      * @param key Reference to store the key
      * @param value Reference to store the value
      * @return true if parsing successful, false otherwise
      */
     bool parseCpuInfoLine(const std::string& line, std::string& key, std::string& value);
 
     /**
      * Convert MHz to GHz with appropriate formatting
      * @param mhz Frequency in MHz
      * @return Formatted frequency string
      */
     std::string formatMhzToGhz(double mhz);
 
     /**
      * Calculate CPU usage percentage from stat values
      * @param user User time
      * @param nice Nice time
      * @param system System time
      * @param idle Idle time
      * @param iowait I/O wait time
      * @param irq IRQ time
      * @param softirq Soft IRQ time
      * @return CPU usage percentage
      */
     double calculateCpuUsage(unsigned long long user, unsigned long long nice,
                            unsigned long long system, unsigned long long idle,
                            unsigned long long iowait, unsigned long long irq,
                            unsigned long long softirq);
 
     /**
      * Get number of online CPUs
      * @return Number of online CPUs
      */
     int getOnlineCpuCount();
 
     /**
      * Check if CPU supports a specific feature flag
      * @param flags Vector of CPU flags
      * @param feature Feature to check for
      * @return true if feature is supported, false otherwise
      */
     bool supportsCpuFeature(const std::vector<std::string>& flags, const std::string& feature);
 }
 
 // Version information
 namespace Version {
     constexpr const char* PROGRAM_NAME = "cpuinfo";
     constexpr const char* VERSION = "1.0";
     constexpr const char* AUTHOR = "AnmiTaliDev";
     constexpr const char* LICENSE = "Apache 2.0";
     constexpr const char* ORGANIZATION = "QCO InfoUtils";
     constexpr const char* SUPPORT_EMAIL = "support@qco-utils.org";
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
 
 #endif // CPUINFO_HPP