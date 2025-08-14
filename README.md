# FastTextSearch <img src="https://github.com/images/mona-happy.gif" width="40px">

## 简介 (Introduction)
FastTextSearch 是一个高性能、多线程的 C++ 应用程序，旨在对指定目录下的文本文件内容进行递归搜索。通过字符串匹配算法和并发处理技术实现对大型文件系统的高效、快速搜索。
FastTextSearch is a high-performance, multi-threaded C++ application designed for recursively searching text content within files across specified directories. It leverages advanced string matching algorithms and concurrent processing techniques to achieve efficient and rapid searches across large file systems.

## 特性 (Features)
*   **多线程文件发现与搜索 (Multi-threaded File Discovery and Search):** 利用多核处理器并行处理文件发现和内容搜索任务，显著提高效率。
    *   Leverages multi-core processors to parallelize file discovery and content search tasks, significantly improving efficiency.
*   **Boyer-Moore-Horspool 字符串匹配算法 (Boyer-Moore-Horspool String Matching Algorithm):** 采用优化的 Boyer-Moore-Horspool 算法进行快速子字符串匹配，减少不必要的字符比较。
    *   Employs the optimized Boyer-Moore-Horspool algorithm for fast substring matching, reducing unnecessary character comparisons.
*   **可配置的系统目录跳过 (Configurable System Directory Skipping):** 自动跳过常见的系统目录（如 Windows 和 Linux 系统路径），避免不必要的扫描和潜在的权限问题。
    *   Automatically skips common system directories (e.g., Windows and Linux system paths) to avoid unnecessary scanning and potential permission issues.
*   **广泛的文本文件类型支持 (Broad Text File Type Support):** 识别并处理多种文本文件扩展名，确保广泛的适用性。
    *   Identifies and processes a wide range of text file extensions, ensuring broad applicability.
*   **跨平台兼容性 (Cross-Platform Compatibility):** 支持 Windows、Linux 和 macOS 操作系统。
    *   Supports Windows, Linux, and macOS operating systems.
*   **彩色控制台输出 (Colored Console Output):** 提供清晰、易读的彩色输出，增强用户体验。
    *   Provides clear, readable colored output for an enhanced user experience.

## 技术栈 (Technology Stack)
*   **C++17:** 利用 C++17 标准库特性，包括 `std::filesystem` 进行文件系统操作和 `std::thread` 进行并发编程。
    *   Leverages C++17 standard library features, including `std::filesystem` for file system operations and `std::thread` for concurrent programming.
*   **`std::mutex` & `std::condition_variable`:** 用于线程同步和协调。
    *   Used for thread synchronization and coordination.
*   **Boyer-Moore-Horspool Algorithm:** 核心字符串搜索逻辑。
    *   Core string search logic.

## 使用方法 (Usage)

### 命令行参数 (Command Line Arguments)
```bash
Usage: FastTextSearch.exe <search_text> <search_path>
Example: FastTextSearch.exe "Hello World" C:\
```

*   `<search_text>`: 要搜索的文本字符串。
    *   The text string to search for.
*   `<search_path>`: 开始搜索的根目录路径。
    *   The root directory path to start the search from.

### 示例 (Example)
在 `C:\Users\YourUser` 目录下搜索 "important_data"：
To search for "important_data" in the `C:\Users\YourUser` directory:
```bash
FastTextSearch.exe "important_data" "C:\Users\YourUser"
```

## 构建指南 (Building from Source)

### 先决条件 (Prerequisites)
*   C++17 兼容的编译器 (如 GCC 7+, Clang 5+, MSVC 19.14+)
    *   A C++17 compatible compiler (e.g., GCC 7+, Clang 5+, MSVC 19.14+)
*   CMake (推荐用于构建系统)
    *   CMake (recommended for build system)

### 步骤 (Steps)
1.  **克隆仓库 (Clone the repository):**
    ```bash
    git clone https://github.com/ra1nyxin/FastTextSearch.git
    cd FastTextSearch
    ```
2.  **创建构建目录并配置 (Create build directory and configure):**
    ```bash
    mkdir build
    cd build
    cmake ..
    ```
3.  **编译项目 (Build the project):**
    ```bash
    cmake --build .
    ```
    在 Windows 上，这会生成一个 `.exe` 文件（例如 `FastTextSearch.exe`）。
    On Windows, this will generate an `.exe` file (e.g., `FastTextSearch.exe`).
    在 Linux/macOS 上，这会生成一个可执行文件（例如 `FastTextSearch`）。
    On Linux/macOS, this will generate an executable (e.g., `FastTextSearch`).

## 贡献 (Contributing)
欢迎贡献！如果您有任何改进建议、错误报告或新功能请求，请随时提交 Issue 或 Pull Request。
Contributions are welcome! If you have any suggestions for improvements, bug reports, or feature requests, feel free to open an issue or submit a pull request.

## 许可证 (License)
本项目采用 MIT 许可证。详情请参阅 [LICENSE](LICENSE) 文件。
This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
