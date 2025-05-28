/*
 * osinfo - Operating System Information Utility
 * Part of QCO InfoUtils (Qainrat Code Organization InfoUtils)
 * Author: AnmiTaliDev
 * License: Apache 2.0
 */

 #ifndef OSINFO_HPP
 #define OSINFO_HPP
 
 #include <string>
 #include <vector>
 #include <map>
 
 // Forward declarations
 struct SystemInfo;
 struct DistroInfo;
 struct UserInfo;
 struct EnvironmentInfo;
 
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
  * Structure to hold basic system information
  */
 struct SystemInfo {
     std::string os_name;
     std::string os_version;
     std::string os_id;
     std::string os_codename;
     std::string kernel_name;
     std::string kernel_release;
     std::string kernel_version;
     std::string architecture;
     std::string hostname;
     std::string domain_name;
     unsigned long uptime_seconds;
     std::string boot_time;
     std::string timezone;
 
     SystemInfo() : uptime_seconds(0) {}
 };
 
 /**
  * Structure to hold distribution-specific information
  */
 struct DistroInfo {
     std::string name;
     std::string version;
     std::string id;
     std::string id_like;
     std::string version_codename;
     std::string version_id;
     std::string pretty_name;
     std::string home_url;
     std::string support_url;
     std::string bug_report_url;
 
     DistroInfo() = default;
 };
 
 /**
  * Structure to hold user and login information
  */
 struct UserInfo {
     std::string current_user;
     std::string current_group;
     std::string home_directory;
     std::string shell;
     int user_count;
     int group_count;
     std::vector<std::string> logged_users;
 
     UserInfo() : user_count(0), group_count(0) {}
 };
 
 /**
  * Structure to hold environment information
  */
 struct EnvironmentInfo {
     std::string path;
     std::string lang;
     std::string editor;
     std::string pager;
     std::string browser;
     std::string desktop_session;
     std::string display_manager;
     std::string window_manager;
 
     EnvironmentInfo() = default;
 };
 
 /**
  * Main utility class for OS information display
  */
 class OsInfoUtil {
 private:
     // Configuration flags
     bool show_detailed;
     bool show_distro;
     bool show_users;
     bool show_environment;
     bool use_colors;
 
     /**
      * Apply color formatting to text if colors are enabled
      * @param text Text to colorize
      * @param color ANSI color code
      * @return Colorized text or plain text if colors disabled
      */
     std::string colorize(const std::string& text, const std::string& color);
 
     /**
      * Format uptime seconds into human-readable format
      * @param seconds Uptime in seconds
      * @return Formatted uptime string
      */
     std::string formatUptime(unsigned long seconds);
 
     /**
      * Read system information from uname and /proc
      * @return SystemInfo structure with basic system details
      */
     SystemInfo getSystemInfo();
 
     /**
      * Read distribution information from /etc/os-release
      * @return DistroInfo structure with distribution details
      */
     DistroInfo getDistroInfo();
 
     /**
      * Read user information from system calls and /etc files
      * @return UserInfo structure with user and login details
      */
     UserInfo getUserInfo();
 
     /**
      * Read environment information from environment variables
      * @return EnvironmentInfo structure with environment details
      */
     EnvironmentInfo getEnvironmentInfo();
 
     /**
      * Print section separator with optional title
      * @param title Optional section title
      */
     void printSeparator(const std::string& title = "");
 
     /**
      * Display general system information
      */
     void printSystemInfo();
 
     /**
      * Display distribution-specific information
      */
     void printDistroInfo();
 
     /**
      * Display user and login information
      */
     void printUserInfo();
 
     /**
      * Display environment information
      */
     void printEnvironmentInfo();
 
 public:
     /**
      * Constructor - initializes utility state
      */
     OsInfoUtil();
 
     /**
      * Destructor
      */
     ~OsInfoUtil() = default;
 
     // Disable copy constructor and assignment operator
     OsInfoUtil(const OsInfoUtil&) = delete;
     OsInfoUtil& operator=(const OsInfoUtil&) = delete;
 
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
     bool isShowDistro() const { return show_distro; }
     bool isShowUsers() const { return show_users; }
     bool isShowEnvironment() const { return show_environment; }
     bool isUseColors() const { return use_colors; }
 
     // Setter methods for testing purposes
     void setShowDetailed(bool value) { show_detailed = value; }
     void setShowDistro(bool value) { show_distro = value; }
     void setShowUsers(bool value) { show_users = value; }
     void setShowEnvironment(bool value) { show_environment = value; }
     void setUseColors(bool value) { use_colors = value; }
 };
 
 /**
  * Utility functions namespace
  */
 namespace OsInfoUtils {
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
      * Parse key-value pair from /etc/os-release format
      * @param line Line to parse
      * @param key Reference to store the key
      * @param value Reference to store the value
      * @return true if parsing successful, false otherwise
      */
     bool parseOsReleaseLine(const std::string& line, std::string& key, std::string& value);
 
     /**
      * Remove quotes from string value
      * @param value String that may contain quotes
      * @return String without surrounding quotes
      */
     std::string removeQuotes(const std::string& value);
 
     /**
      * Format time duration in human-readable format
      * @param seconds Duration in seconds
      * @param detailed Whether to include seconds in output
      * @return Formatted duration string
      */
     std::string formatDuration(unsigned long seconds, bool detailed = true);
 
     /**
      * Get environment variable value safely
      * @param var_name Environment variable name
      * @param default_value Default value if variable not set
      * @return Environment variable value or default
      */
     std::string getEnvVar(const std::string& var_name, const std::string& default_value = "");
 
     /**
      * Check if running in a container environment
      * @return true if running in container, false otherwise
      */
     bool isRunningInContainer();
 
     /**
      * Get system boot time from /proc/stat
      * @return Boot time as timestamp string
      */
     std::string getBootTime();
 
     /**
      * Count logged in users from utmp/wtmp
      * @return Number of currently logged in users
      */
     int getLoggedUserCount();
 
     /**
      * Get system load averages
      * @param load1 Reference to store 1-minute load
      * @param load5 Reference to store 5-minute load  
      * @param load15 Reference to store 15-minute load
      * @return true if successful, false otherwise
      */
     bool getLoadAverages(double& load1, double& load5, double& load15);
 
     /**
      * Detect desktop environment from environment variables
      * @return Desktop environment name or empty string
      */
     std::string detectDesktopEnvironment();
 
     /**
      * Get default shell for current user
      * @return Path to default shell
      */
     std::string getDefaultShell();
 }
 
 // Version information
 namespace Version {
     constexpr const char* PROGRAM_NAME = "osinfo";
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
 
 #endif // OSINFO_HPP