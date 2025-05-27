/*
 * meminfo - Memory Information Utility
 * Part of QCO InfoUtils (Qainrat Code Organization InfoUtils)
 * Author: AnmiTaliDev
 * License: Apache 2.0
 */

 #ifndef MEMINFO_HPP
 #define MEMINFO_HPP
 
 #include <string>
 #include <vector>
 
 // Forward declarations
 struct MemoryInfo;
 struct ProcessInfo;
 
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
  * Structure to hold memory information from /proc/meminfo
  */
 struct MemoryInfo {
     unsigned long total_kb;
     unsigned long available_kb;
     unsigned long free_kb;
     unsigned long buffers_kb;
     unsigned long cached_kb;
     unsigned long swap_total_kb;
     unsigned long swap_free_kb;
     unsigned long swap_cached_kb;
     unsigned long shmem_kb;
     unsigned long sreclaimable_kb;
     unsigned long sunreclaim_kb;
 
     MemoryInfo() : total_kb(0), available_kb(0), free_kb(0), buffers_kb(0),
                    cached_kb(0), swap_total_kb(0), swap_free_kb(0), 
                    swap_cached_kb(0), shmem_kb(0), sreclaimable_kb(0),
                    sunreclaim_kb(0) {}
 };
 
 /**
  * Structure to hold process memory information
  */
 struct ProcessInfo {
     int pid;
     std::string name;
     unsigned long memory_kb;
     std::string cmd;
 
     ProcessInfo(int p, const std::string& n, unsigned long m, const std::string& c)
         : pid(p), name(n), memory_kb(m), cmd(c) {}
 };
 
 /**
  * Main utility class for memory information display
  */
 class MemInfoUtil {
 private:
     // Configuration flags
     bool show_processes;
     bool show_detailed;
     bool show_swap;
     bool use_colors;
 
     /**
      * Format bytes with human-readable units (B, KB, MB, GB, TB)
      * @param kb Size in kilobytes
      * @return Formatted string with appropriate unit
      */
     std::string formatBytes(unsigned long kb);
 
     /**
      * Apply color formatting to text if colors are enabled
      * @param text Text to colorize
      * @param color ANSI color code
      * @return Colorized text or plain text if colors disabled
      */
     std::string colorize(const std::string& text, const std::string& color);
 
     /**
      * Read memory information from /proc/meminfo
      * @return MemoryInfo structure with current memory stats
      */
     MemoryInfo getMemoryInfo();
 
     /**
      * Get list of processes sorted by memory usage
      * @param limit Maximum number of processes to return
      * @return Vector of ProcessInfo structures
      */
     std::vector<ProcessInfo> getTopProcesses(int limit = 15);
 
     /**
      * Print section separator with optional title
      * @param title Optional section title
      */
     void printSeparator(const std::string& title = "");
 
     /**
      * Display general memory information
      */
     void printGeneralInfo();
 
     /**
      * Display top memory consuming processes
      */
     void printProcesses();
 
 public:
     /**
      * Constructor - initializes utility state
      */
     MemInfoUtil();
 
     /**
      * Destructor
      */
     ~MemInfoUtil() = default;
 
     // Disable copy constructor and assignment operator
     MemInfoUtil(const MemInfoUtil&) = delete;
     MemInfoUtil& operator=(const MemInfoUtil&) = delete;
 
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
     bool isShowProcesses() const { return show_processes; }
     bool isShowDetailed() const { return show_detailed; }
     bool isShowSwap() const { return show_swap; }
     bool isUseColors() const { return use_colors; }
 
     // Setter methods for testing purposes
     void setShowProcesses(bool value) { show_processes = value; }
     void setShowDetailed(bool value) { show_detailed = value; }
     void setShowSwap(bool value) { show_swap = value; }
     void setUseColors(bool value) { use_colors = value; }
 };
 
 /**
  * Utility functions namespace
  */
 namespace MemInfoUtils {
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
      * Parse a key-value pair from /proc/meminfo format
      * @param line Line to parse
      * @param key Reference to store the key
      * @param value Reference to store the value
      * @return true if parsing successful, false otherwise
      */
     bool parseMemInfoLine(const std::string& line, std::string& key, unsigned long& value);
 
     /**
      * Get system page size in bytes
      * @return Page size in bytes
      */
     long getPageSize();
 
     /**
      * Convert pages to kilobytes
      * @param pages Number of pages
      * @return Size in kilobytes
      */
     unsigned long pagesToKilobytes(unsigned long pages);
 }
 
 // Version information
 namespace Version {
     constexpr const char* PROGRAM_NAME = "meminfo";
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
 
 #endif // MEMINFO_HPP