/*
 * osinfo - Operating System Information Utility
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
 #include <sys/utsname.h>
 #include <sys/sysinfo.h>
 #include <pwd.h>
 #include <grp.h>
 
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
     unsigned long uptime_seconds = 0;
     std::string boot_time;
     std::string timezone;
 };
 
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
 };
 
 struct UserInfo {
     std::string current_user;
     std::string current_group;
     std::string home_directory;
     std::string shell;
     int user_count = 0;
     int group_count = 0;
     std::vector<std::string> logged_users;
 };
 
 struct EnvironmentInfo {
     std::string path;
     std::string lang;
     std::string editor;
     std::string pager;
     std::string browser;
     std::string desktop_session;
     std::string display_manager;
     std::string window_manager;
 };
 
 class OsInfoUtil {
 private:
     bool show_detailed = false;
     bool show_distro = false;
     bool show_users = false;
     bool show_environment = false;
     bool use_colors = true;
 
     std::string colorize(const std::string& text, const std::string& color) {
         if (!use_colors) return text;
         return color + text + Colors::RESET;
     }
 
     std::string formatUptime(unsigned long seconds) {
         unsigned long days = seconds / 86400;
         unsigned long hours = (seconds % 86400) / 3600;
         unsigned long minutes = (seconds % 3600) / 60;
         unsigned long secs = seconds % 60;
         
         std::ostringstream oss;
         if (days > 0) {
             oss << days << " day" << (days != 1 ? "s" : "") << ", ";
         }
         if (hours > 0 || days > 0) {
             oss << hours << " hour" << (hours != 1 ? "s" : "") << ", ";
         }
         if (minutes > 0 || hours > 0 || days > 0) {
             oss << minutes << " minute" << (minutes != 1 ? "s" : "") << ", ";
         }
         oss << secs << " second" << (secs != 1 ? "s" : "");
         
         return oss.str();
     }
 
     SystemInfo getSystemInfo() {
         SystemInfo info;
         
         // Get uname information
         struct utsname uts;
         if (uname(&uts) == 0) {
             info.kernel_name = uts.sysname;
             info.kernel_release = uts.release;
             info.kernel_version = uts.version;
             info.architecture = uts.machine;
             info.hostname = uts.nodename;
         }
         
         // Get domain name
         char domainname[256];
         if (getdomainname(domainname, sizeof(domainname)) == 0) {
             info.domain_name = domainname;
         }
         
         // Get uptime
         struct sysinfo si;
         if (sysinfo(&si) == 0) {
             info.uptime_seconds = si.uptime;
         }
         
         // Try to get timezone
         std::ifstream timezone_file("/etc/timezone");
         if (timezone_file.is_open()) {
             std::getline(timezone_file, info.timezone);
         } else {
             // Fallback to TZ environment variable
             const char* tz = getenv("TZ");
             if (tz) {
                 info.timezone = tz;
             }
         }
         
         return info;
     }
 
     DistroInfo getDistroInfo() {
         DistroInfo info;
         
         // Read /etc/os-release
         std::ifstream os_release("/etc/os-release");
         std::string line;
         
         while (std::getline(os_release, line)) {
             size_t equals = line.find('=');
             if (equals == std::string::npos) continue;
             
             std::string key = line.substr(0, equals);
             std::string value = line.substr(equals + 1);
             
             // Remove quotes
             if (value.front() == '"' && value.back() == '"') {
                 value = value.substr(1, value.length() - 2);
             }
             
             if (key == "NAME") info.name = value;
             else if (key == "VERSION") info.version = value;
             else if (key == "ID") info.id = value;
             else if (key == "ID_LIKE") info.id_like = value;
             else if (key == "VERSION_CODENAME") info.version_codename = value;
             else if (key == "VERSION_ID") info.version_id = value;
             else if (key == "PRETTY_NAME") info.pretty_name = value;
             else if (key == "HOME_URL") info.home_url = value;
             else if (key == "SUPPORT_URL") info.support_url = value;
             else if (key == "BUG_REPORT_URL") info.bug_report_url = value;
         }
         
         return info;
     }
 
     UserInfo getUserInfo() {
         UserInfo info;
         
         // Current user information
         uid_t uid = getuid();
         gid_t gid = getgid();
         
         struct passwd* pw = getpwuid(uid);
         if (pw) {
             info.current_user = pw->pw_name;
             info.home_directory = pw->pw_dir;
             info.shell = pw->pw_shell;
         }
         
         struct group* gr = getgrgid(gid);
         if (gr) {
             info.current_group = gr->gr_name;
         }
         
         // Count users and groups
         setpwent();
         while (getpwent() != nullptr) {
             info.user_count++;
         }
         endpwent();
         
         setgrent();
         while (getgrent() != nullptr) {
             info.group_count++;
         }
         endgrent();
         
         return info;
     }
 
     EnvironmentInfo getEnvironmentInfo() {
         EnvironmentInfo info;
         
         const char* env_vars[] = {
             "PATH", "LANG", "EDITOR", "PAGER", "BROWSER",
             "DESKTOP_SESSION", "XDG_CURRENT_DESKTOP", "WINDOWMANAGER"
         };
         
         for (const char* var : env_vars) {
             const char* value = getenv(var);
             if (value) {
                 std::string var_name = var;
                 if (var_name == "PATH") info.path = value;
                 else if (var_name == "LANG") info.lang = value;
                 else if (var_name == "EDITOR") info.editor = value;
                 else if (var_name == "PAGER") info.pager = value;
                 else if (var_name == "BROWSER") info.browser = value;
                 else if (var_name == "DESKTOP_SESSION") info.desktop_session = value;
                 else if (var_name == "XDG_CURRENT_DESKTOP") info.window_manager = value;
                 else if (var_name == "WINDOWMANAGER") {
                     if (info.window_manager.empty()) info.window_manager = value;
                 }
             }
         }
         
         return info;
     }
 
     void printSeparator(const std::string& title = "") {
         if (title.empty()) {
             std::cout << std::string(70, '-') << std::endl;
         } else {
             std::cout << colorize(title, Colors::BOLD) << std::endl;
             std::cout << std::string(title.length(), '=') << std::endl;
         }
     }
 
     void printSystemInfo() {
         SystemInfo sys_info = getSystemInfo();
         DistroInfo distro_info = getDistroInfo();
         
         printSeparator("System Information");
         
         if (!distro_info.pretty_name.empty()) {
             std::cout << std::left << std::setw(18) << "Operating System:" 
                       << distro_info.pretty_name << std::endl;
         } else if (!distro_info.name.empty()) {
             std::cout << std::left << std::setw(18) << "Operating System:" 
                       << distro_info.name;
             if (!distro_info.version.empty()) {
                 std::cout << " " << distro_info.version;
             }
             std::cout << std::endl;
         }
         
         if (!sys_info.kernel_name.empty() && !sys_info.kernel_release.empty()) {
             std::cout << std::left << std::setw(18) << "Kernel:" 
                       << sys_info.kernel_name << " " << sys_info.kernel_release << std::endl;
         }
         
         if (!sys_info.architecture.empty()) {
             std::cout << std::left << std::setw(18) << "Architecture:" 
                       << sys_info.architecture << std::endl;
         }
         
         if (!sys_info.hostname.empty()) {
             std::cout << std::left << std::setw(18) << "Hostname:" 
                       << sys_info.hostname;
             if (!sys_info.domain_name.empty() && sys_info.domain_name != "(none)") {
                 std::cout << "." << sys_info.domain_name;
             }
             std::cout << std::endl;
         }
         
         if (sys_info.uptime_seconds > 0) {
             std::cout << std::left << std::setw(18) << "Uptime:" 
                       << formatUptime(sys_info.uptime_seconds) << std::endl;
         }
         
         if (show_detailed) {
             if (!sys_info.kernel_version.empty()) {
                 std::cout << std::left << std::setw(18) << "Kernel version:" 
                           << sys_info.kernel_version << std::endl;
             }
             
             if (!sys_info.timezone.empty()) {
                 std::cout << std::left << std::setw(18) << "Timezone:" 
                           << sys_info.timezone << std::endl;
             }
             
             // Read additional kernel information
             std::ifstream version_file("/proc/version");
             if (version_file.is_open()) {
                 std::string version_line;
                 std::getline(version_file, version_line);
                 if (version_line.length() > 80) {
                     version_line = version_line.substr(0, 77) + "...";
                 }
                 std::cout << std::left << std::setw(18) << "Kernel info:" 
                           << version_line << std::endl;
             }
         }
     }
 
     void printDistroInfo() {
         DistroInfo info = getDistroInfo();
         
         std::cout << std::endl;
         printSeparator("Distribution Information");
         
         if (!info.name.empty()) {
             std::cout << std::left << std::setw(18) << "Name:" 
                       << info.name << std::endl;
         }
         
         if (!info.version.empty()) {
             std::cout << std::left << std::setw(18) << "Version:" 
                       << info.version << std::endl;
         }
         
         if (!info.id.empty()) {
             std::cout << std::left << std::setw(18) << "ID:" 
                       << info.id << std::endl;
         }
         
         if (!info.version_codename.empty()) {
             std::cout << std::left << std::setw(18) << "Codename:" 
                       << info.version_codename << std::endl;
         }
         
         if (show_detailed) {
             if (!info.id_like.empty()) {
                 std::cout << std::left << std::setw(18) << "Based on:" 
                           << info.id_like << std::endl;
             }
             
             if (!info.version_id.empty()) {
                 std::cout << std::left << std::setw(18) << "Version ID:" 
                           << info.version_id << std::endl;
             }
             
             if (!info.home_url.empty()) {
                 std::cout << std::left << std::setw(18) << "Home URL:" 
                           << info.home_url << std::endl;
             }
             
             if (!info.support_url.empty()) {
                 std::cout << std::left << std::setw(18) << "Support URL:" 
                           << info.support_url << std::endl;
             }
         }
     }
 
     void printUserInfo() {
         UserInfo info = getUserInfo();
         
         std::cout << std::endl;
         printSeparator("User Information");
         
         if (!info.current_user.empty()) {
             std::cout << std::left << std::setw(18) << "Current user:" 
                       << info.current_user << std::endl;
         }
         
         if (!info.current_group.empty()) {
             std::cout << std::left << std::setw(18) << "Primary group:" 
                       << info.current_group << std::endl;
         }
         
         if (!info.home_directory.empty()) {
             std::cout << std::left << std::setw(18) << "Home directory:" 
                       << info.home_directory << std::endl;
         }
         
         if (!info.shell.empty()) {
             std::cout << std::left << std::setw(18) << "Shell:" 
                       << info.shell << std::endl;
         }
         
         if (info.user_count > 0) {
             std::cout << std::left << std::setw(18) << "Total users:" 
                       << info.user_count << std::endl;
         }
         
         if (info.group_count > 0) {
             std::cout << std::left << std::setw(18) << "Total groups:" 
                       << info.group_count << std::endl;
         }
     }
 
     void printEnvironmentInfo() {
         EnvironmentInfo info = getEnvironmentInfo();
         
         std::cout << std::endl;
         printSeparator("Environment Information");
         
         if (!info.lang.empty()) {
             std::cout << std::left << std::setw(18) << "Language:" 
                       << info.lang << std::endl;
         }
         
         if (!info.desktop_session.empty()) {
             std::cout << std::left << std::setw(18) << "Desktop session:" 
                       << info.desktop_session << std::endl;
         }
         
         if (!info.window_manager.empty()) {
             std::cout << std::left << std::setw(18) << "Desktop environment:" 
                       << info.window_manager << std::endl;
         }
         
         if (!info.editor.empty()) {
             std::cout << std::left << std::setw(18) << "Default editor:" 
                       << info.editor << std::endl;
         }
         
         const char* shell_env = getenv("SHELL");
         if (shell_env) {
             std::cout << std::left << std::setw(18) << "Default shell:" 
                       << shell_env << std::endl;
         }
         
         if (show_detailed) {
             if (!info.pager.empty()) {
                 std::cout << std::left << std::setw(18) << "Pager:" 
                           << info.pager << std::endl;
             }
             
             if (!info.browser.empty()) {
                 std::cout << std::left << std::setw(18) << "Browser:" 
                           << info.browser << std::endl;
             }
             
             if (!info.path.empty()) {
                 std::cout << std::left << std::setw(18) << "PATH:" << std::endl;
                 std::istringstream path_stream(info.path);
                 std::string path_component;
                 while (std::getline(path_stream, path_component, ':')) {
                     std::cout << "  " << path_component << std::endl;
                 }
             }
         }
     }
 
 public:
     OsInfoUtil() {
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
             } else if (arg == "--distro" || arg == "-r") {
                 show_distro = true;
             } else if (arg == "--users" || arg == "-u") {
                 show_users = true;
             } else if (arg == "--environment" || arg == "-e") {
                 show_environment = true;
             } else if (arg == "--all" || arg == "-a") {
                 show_detailed = show_distro = show_users = show_environment = true;
             } else if (arg == "--no-color") {
                 use_colors = false;
             } else {
                 std::cerr << colorize("osinfo: invalid option -- '" + arg + "'", Colors::RED) << std::endl;
                 std::cerr << "Try 'osinfo --help' for more information." << std::endl;
                 exit(1);
             }
         }
     }
 
     void printVersion() {
         std::cout << "osinfo (QCO InfoUtils) 1.0" << std::endl;
         std::cout << "Copyright (C) 2025 AnmiTaliDev" << std::endl;
         std::cout << "License Apache 2.0: Apache License version 2.0" << std::endl;
         std::cout << "This is free software: you are free to change and redistribute it." << std::endl;
         std::cout << "There is NO WARRANTY, to the extent permitted by law." << std::endl;
     }
 
     void printHelp() {
         std::cout << "Usage: osinfo [OPTION]..." << std::endl;
         std::cout << "Display information about the operating system." << std::endl;
         std::cout << std::endl;
         std::cout << "  -a, --all         display all available information" << std::endl;
         std::cout << "  -d, --detailed    show detailed system information" << std::endl;
         std::cout << "  -e, --environment show environment information" << std::endl;
         std::cout << "  -h, --help        display this help and exit" << std::endl;
         std::cout << "      --no-color    disable colored output" << std::endl;
         std::cout << "  -r, --distro      show distribution information" << std::endl;
         std::cout << "  -u, --users       show user information" << std::endl;
         std::cout << "  -V, --version     output version information and exit" << std::endl;
         std::cout << std::endl;
         std::cout << "Examples:" << std::endl;
         std::cout << "  osinfo            Show basic system information" << std::endl;
         std::cout << "  osinfo -a         Show comprehensive system report" << std::endl;
         std::cout << "  osinfo -r         Show distribution information" << std::endl;
         std::cout << std::endl;
         std::cout << "QCO InfoUtils home page: <https://github.com/Qainar-Projects/infoutils>" << std::endl;
     }
 
     void run() {
         printSystemInfo();
         
         if (show_distro) {
             printDistroInfo();
         }
         
         if (show_users) {
             printUserInfo();
         }
         
         if (show_environment) {
             printEnvironmentInfo();
         }
     }
 };
 
 int main(int argc, char* argv[]) {
     try {
         OsInfoUtil util;
         util.parseArgs(argc, argv);
         util.run();
         return 0;
     } catch (const std::exception& e) {
         std::cerr << "osinfo: " << e.what() << std::endl;
         return 1;
     }
 }