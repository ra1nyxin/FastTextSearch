#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <set>
#include <fstream>
#include <string_view>
#include <sstream>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>

std::mutex g_console_mutex;

#ifdef _WIN32
#include <Windows.h>
#include <fcntl.h>
#include <io.h>
#include <stringapiset.h>

HANDLE hConsole = NULL;
WORD default_attributes;

void set_console_color(WORD attributes) {
    SetConsoleTextAttribute(hConsole, attributes);
}

void reset_console_color() {
    SetConsoleTextAttribute(hConsole, default_attributes);
}

const WORD COLOR_DARK_AQUA = FOREGROUND_GREEN | FOREGROUND_BLUE;
const WORD COLOR_AQUA = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
const WORD COLOR_RED = FOREGROUND_RED;

#else
#define COLOR_DARK_AQUA "\x1b[36m"
#define COLOR_AQUA "\x1b[96m"
#define COLOR_RED "\x1b[31m"
#define COLOR_RESET "\x1b[0m"

void set_console_color(const char* color_code) {
    std::cout << color_code;
}

void reset_console_color() {
    std::cout << COLOR_RESET;
}
#endif

void print_colored(const std::string& text, const std::string& color_type, bool add_newline = false) {
    std::lock_guard<std::mutex> lock(g_console_mutex);
#ifdef _WIN32
    if (hConsole == NULL) {
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
        GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
        default_attributes = consoleInfo.wAttributes;
    }

    std::stringstream ss;
    if (color_type == "dark_aqua") {
        set_console_color(COLOR_DARK_AQUA);
    } else if (color_type == "aqua") {
        set_console_color(COLOR_AQUA);
    } else if (color_type == "red") {
        set_console_color(COLOR_RED);
    }
    ss << text;
    if (add_newline) {
        ss << std::endl;
    }
    std::cout << ss.str() << std::flush;
    reset_console_color();
#else
    std::stringstream ss;
    if (color_type == "dark_aqua") {
        ss << COLOR_DARK_AQUA;
    } else if (color_type == "aqua") {
        ss << COLOR_AQUA;
    } else if (color_type == "red") {
        ss << COLOR_RED;
    }
    ss << text;
    if (add_newline) {
        ss << std::endl;
    }
    ss << COLOR_RESET;
    std::cout << ss.str() << std::flush;
#endif
}

void println_colored(const std::string& text, const std::string& color_type) {
    print_colored(text, color_type, true);
}

bool shouldSkipPath(const std::filesystem::path& path) {
    std::string path_str = path.string();
    std::transform(path_str.begin(), path_str.end(), path_str.begin(), ::tolower);

    std::vector<std::string> skip_dirs = {
        "c:\\windows",
        "c:\\program files",
        "c:\\program files (x86)",
        "c:\\programdata",
        "c:\\users\\all users",
        "c:\\documents and settings",
        "/mnt/c/windows",
        "/mnt/c/program files",
        "/mnt/c/program files (x86)",
        "/mnt/c/programdata",
        "/proc",
        "/sys",
        "/dev",
        "/run",
        "/var/lib",
        "/var/cache",
        "/var/log",
        "/usr/bin",
        "/usr/lib",
        "/usr/local",
        "/opt",
        "/boot",
        "/etc"
    };

    for (const auto& skip_dir : skip_dirs) {
        if (path_str.find(skip_dir) != std::string::npos) {
            return true;
        }
    }
    return false;
}

bool isTextFileExtension(const std::filesystem::path& path) {
    static const std::set<std::string> text_extensions = {
        ".txt", ".log", ".ini", ".java", ".cs", ".json", ".cpp", ".hpp", ".h", ".c", ".md",
        ".xml", ".html", ".htm", ".css", ".js", ".ts", ".py", ".sh", ".bat", ".ps1",
        ".yml", ".yaml", ".toml", ".csv", ".tsv", ".sql", ".php", ".rb", ".go", ".rs",
        ".swift", ".kt", ".scala", ".groovy", ".pl", ".pm", ".r", ".vue", ".jsx", ".tsx",
        ".scss", ".less", ".sass", ".config", ".properties", ".gitignore", ".editorconfig",
        ".gitattributes", ".npmignore", ".project", ".classpath", ".settings", ".buildpath",
        ".dockerfile", ".env", ".example", ".template", ".conf", ".cfg", ".rc", ".manifest",
        ".plist", ".xaml", ".csproj", ".vbproj", ".fsproj", ".sln", ".vcxproj", ".filters",
        ".user", ".cmake", ".make", ".gradle", ".pom", ".lock", ".sum", ".mod", ".bak",
        ".tmp", ".old", ".orig", ".diff", ".patch", ".rej", ".swp", ".swo", ".swr", ".bak",
        ".bk", ".copy", ".orig", ".old", ".new", ".temp", ".test", ".debug", ".release"
    };

    std::string ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return text_extensions.count(ext);
}

// File discovery function
void discoverFiles(const std::filesystem::path& root_path, std::queue<std::filesystem::path>& file_queue, std::mutex& queue_mutex, std::atomic<bool>& discovery_finished, std::condition_variable& cv_worker) {
    println_colored("File discovery started from: " + root_path.u8string(), "dark_aqua");
    try {
        for (auto it = std::filesystem::recursive_directory_iterator(root_path); it != std::filesystem::recursive_directory_iterator(); ++it) {
            const auto& entry = *it;

            if (entry.is_directory()) {
                if (shouldSkipPath(entry.path())) {
                    println_colored("Skipping directory: " + entry.path().u8string(), "dark_aqua");
                    it.disable_recursion_pending();
                }
            } else if (entry.is_regular_file()) {
                if (isTextFileExtension(entry.path())) {
                    {
                        std::lock_guard<std::mutex> lock(queue_mutex);
                        file_queue.push(entry.path());
                    }
                    cv_worker.notify_one();
                }
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
#ifdef _WIN32
        std::string error_msg_ansi = e.what();
        int wchars_num = MultiByteToWideChar(CP_ACP, 0, error_msg_ansi.c_str(), -1, NULL, 0);
        std::wstring w_error_msg(wchars_num, 0);
        MultiByteToWideChar(CP_ACP, 0, error_msg_ansi.c_str(), -1, &w_error_msg[0], wchars_num);

        int utf8_num = WideCharToMultiByte(CP_UTF8, 0, w_error_msg.c_str(), -1, NULL, 0, NULL, NULL);
        std::string utf8_error_msg(utf8_num, 0);
        WideCharToMultiByte(CP_UTF8, 0, w_error_msg.c_str(), -1, &utf8_error_msg[0], utf8_num, NULL, NULL);

        println_colored("Filesystem error during discovery: " + utf8_error_msg, "red");
#else
        println_colored("Filesystem error during discovery: " + std::string(e.what()), "red");
#endif
    } catch (const std::exception& e) {
#ifdef _WIN32
        std::string error_msg_ansi = e.what();
        int wchars_num = MultiByteToWideChar(CP_ACP, 0, error_msg_ansi.c_str(), -1, NULL, 0);
        std::wstring w_error_msg(wchars_num, 0);
        MultiByteToWideChar(CP_ACP, 0, error_msg_ansi.c_str(), -1, &w_error_msg[0], wchars_num);

        int utf8_num = WideCharToMultiByte(CP_UTF8, 0, w_error_msg.c_str(), -1, NULL, 0, NULL, NULL);
        std::string utf8_error_msg(utf8_num, 0);
        WideCharToMultiByte(CP_UTF8, 0, w_error_msg.c_str(), -1, &utf8_error_msg[0], utf8_num, NULL, NULL);

        println_colored("Error during discovery: " + utf8_error_msg, "red");
#else
        println_colored("Error during discovery: " + std::string(e.what()), "red");
#endif
    }

    discovery_finished.store(true);
    cv_worker.notify_all();
    println_colored("File discovery finished.", "dark_aqua");
}

size_t boyerMooreSearch(const std::string_view& text, const std::string_view& pattern) {
    if (pattern.empty()) {
        return 0;
    }
    if (pattern.length() > text.length()) {
        return std::string_view::npos;
    }

    std::vector<size_t> bad_char_shift(256, pattern.length());
    for (size_t i = 0; i < pattern.length() - 1; ++i) {
        bad_char_shift[static_cast<unsigned char>(pattern[i])] = pattern.length() - 1 - i;
    }

    long long i = 0;
    while (i <= (long long)text.length() - (long long)pattern.length()) {
        long long j = pattern.length() - 1;
        while (j >= 0 && text[i + j] == pattern[j]) {
            j--;
        }
        if (j < 0) {
            return i;
        } else {
            i += bad_char_shift[static_cast<unsigned char>(text[i + pattern.length() - 1])];
        }
    }
    return std::string_view::npos;
}

struct WorkerCountGuard {
    std::atomic<int>& active_workers;
    std::condition_variable& cv_worker;

    WorkerCountGuard(std::atomic<int>& workers, std::condition_variable& cv)
        : active_workers(workers), cv_worker(cv) {
        active_workers++;
    }

    ~WorkerCountGuard() {
        active_workers--;
        cv_worker.notify_all();
    }
};

void searchWorker(const std::string& search_text, std::queue<std::filesystem::path>& file_queue, std::mutex& queue_mutex, std::atomic<bool>& discovery_finished, std::atomic<int>& active_workers, std::condition_variable& cv_worker) {
    WorkerCountGuard guard(active_workers, cv_worker);

    const size_t BUFFER_SIZE = 64 * 1024;
    const size_t OVERLAP_SIZE = search_text.length() > 0 ? search_text.length() - 1 : 0;
    std::vector<char> buffer(BUFFER_SIZE + OVERLAP_SIZE);

    while (true) {
        std::filesystem::path file_path;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            cv_worker.wait(lock, [&]{ return !file_queue.empty() || discovery_finished.load(); });

            if (file_queue.empty() && discovery_finished.load()) {
                break;
            }

            file_path = file_queue.front();
            file_queue.pop();
        }

        if (!file_path.empty()) {
            std::ifstream file(file_path, std::ios::binary);
            if (file.is_open()) {
                std::string overlap_buffer;

                while (true) {
                    if (!overlap_buffer.empty()) {
                        std::copy(overlap_buffer.begin(), overlap_buffer.end(), buffer.begin());
                    }

                    file.read(buffer.data() + OVERLAP_SIZE, BUFFER_SIZE);
                    size_t bytes_read = file.gcount();

                    size_t current_chunk_size = bytes_read + OVERLAP_SIZE;

                    if (current_chunk_size == 0) {
                        break;
                    }

                    std::string_view chunk(buffer.data(), current_chunk_size);

                    if (boyerMooreSearch(chunk, search_text) != std::string_view::npos) {
                        print_colored("Found \"", "dark_aqua");
                        print_colored(search_text, "aqua");
                        print_colored("\" in: ", "dark_aqua");
                        println_colored(file_path.u8string(), "aqua");
                        break;
                    }

                    if (bytes_read > 0) {
                        size_t copy_len = std::min(bytes_read, OVERLAP_SIZE);
                        overlap_buffer.assign(buffer.data() + OVERLAP_SIZE + bytes_read - copy_len, copy_len);
                    } else {
                        overlap_buffer.clear();
                    }

                    if (file.eof() && bytes_read == 0) {
                        break;
                    }
                }
                file.close();
            } else {
                println_colored("Error opening file: " + file_path.u8string(), "red");
            }
        }
    }
}


int main(int argc, char* argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    if (argc < 3) {
        println_colored("Usage: " + std::string(argv[0]) + " <search_text> <search_path>", "red");
        println_colored("Example: " + std::string(argv[0]) + " \"Hello World\" C:\\", "red");
        return 1;
    }

    std::string search_text = argv[1];
    std::filesystem::path search_path = argv[2];

    print_colored("Search Text: \"", "dark_aqua");
    print_colored(search_text, "aqua");
    println_colored("\"", "dark_aqua");

    print_colored("Search Path: \"", "dark_aqua");
    print_colored(search_path.u8string(), "aqua");
    println_colored("\"", "dark_aqua");

    std::queue<std::filesystem::path> file_queue;
    std::mutex queue_mutex;
    std::atomic<bool> discovery_finished(false);
    std::atomic<int> active_workers(0);
    std::condition_variable cv_worker;

    std::thread discovery_thread(discoverFiles, search_path, std::ref(file_queue), std::ref(queue_mutex), std::ref(discovery_finished), std::ref(cv_worker));

    std::vector<std::thread> worker_threads;
    unsigned int num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) num_threads = 4;
    print_colored("Starting ", "dark_aqua");
    print_colored(std::to_string(num_threads), "aqua");
    println_colored(" worker threads.", "dark_aqua");

    for (unsigned int i = 0; i < num_threads; ++i) {
        worker_threads.emplace_back(searchWorker, std::ref(search_text), std::ref(file_queue), std::ref(queue_mutex), std::ref(discovery_finished), std::ref(active_workers), std::ref(cv_worker));
    }

    discovery_thread.join();

    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        cv_worker.wait(lock, [&]{
            return file_queue.empty() && discovery_finished.load() && active_workers.load() == 0;
        });
    }

    for (std::thread& t : worker_threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    println_colored("Search completed.", "dark_aqua");

    return 0;
}
