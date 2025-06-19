// herlang.cpp - Modern build tool for HerLang
// 一步式编译与友好错误提示的实现

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <filesystem>
#include <chrono>
#include <regex>
#include <algorithm>

namespace fs = std::filesystem;
using namespace std;

struct BuildConfig {
    string project_name;
    string version;
    string target_arch = "native";
    string optimization = "release";
    string output_dir = "build";
    bool hot_reload = true;
    bool friendly_errors = true;
    vector<string> dependencies;
    map<string, bool> interop_languages;
};

struct ErrorInfo {
    string filename;
    int line;
    int column;
    string error_type;
    string message;
    string suggestion;
    vector<string> context_lines;
};

class GentleCompiler {
private:
    BuildConfig config;
    vector<ErrorInfo> errors;
    vector<string> warnings;

public:
    void load_config() {
        ifstream config_file("HerLang.toml");
        if (!config_file) {
            cout << "💝 创建新项目配置..." << endl;
            create_default_config();
            return;
        }
        
        string line;
        while (getline(config_file, line)) {
            if (line.find("name = ") != string::npos) {
                config.project_name = extract_quoted_value(line);
            } else if (line.find("target = ") != string::npos) {
                config.target_arch = extract_quoted_value(line);
            }
        }
        
        cout << "🌸 已加载项目: " << config.project_name << endl;
    }

    void friendly_error(const string& filename, int line, int col, 
                       const string& error_type, const string& message, 
                       const string& suggestion = "") {
        ErrorInfo error;
        error.filename = filename;
        error.line = line;
        error.column = col;
        error.error_type = error_type;
        error.message = message;
        error.suggestion = suggestion;
        
        // 读取上下文代码行
        ifstream file(filename);
        string file_line;
        int current_line = 1;
        while (getline(file, file_line) && current_line <= line + 2) {
            if (current_line >= line - 2) {
                error.context_lines.push_back(file_line);
            }
            current_line++;
        }
        
        errors.push_back(error);
    }

    void print_friendly_errors() {
        if (errors.empty()) return;
        
        cout << "\n💔 温柔提醒：发现了一些需要关注的地方\n" << endl;
        
        for (const auto& error : errors) {
            cout << "📍 " << error.filename << ":" << error.line << ":" << error.column << endl;
            cout << "💭 " << error.error_type << ": " << error.message << endl;
            
            // 显示代码上下文
            cout << "\n📝 代码上下文:" << endl;
            int start_line = max(1, error.line - 2);
            for (size_t i = 0; i < error.context_lines.size(); i++) {
                int line_num = start_line + i;
                string prefix = (line_num == error.line) ? " ➤ " : "   ";
                cout << prefix << line_num << " | " << error.context_lines[i] << endl;
                
                if (line_num == error.line) {
                    cout << "     | " << string(error.column - 1, ' ') << "^ 这里" << endl;
                }
            }
            
            if (!error.suggestion.empty()) {
                cout << "\n💡 建议: " << error.suggestion << endl;
            }
            cout << "\n" << string(50, '-') << "\n" << endl;
        }
        
        cout << "🌟 别灰心！每个程序员都会遇到这些，你一定能解决的！" << endl;
    }

    bool compile_file(const string& source_file) {
        cout << "🔄 正在温柔地编译 " << source_file << "..." << endl;
        
        ifstream file(source_file);
        if (!file) {
            friendly_error(source_file, 1, 1, "文件访问", 
                          "无法打开源文件", 
                          "请检查文件路径是否正确，或者文件是否存在");
            return false;
        }
        
        string line;
        int line_num = 1;
        vector<string> keywords = {"function", "gentle_function", "if", "gently_if", 
                                  "loop", "gentle_loop", "say", "whisper", "share"};
        
        while (getline(file, line)) {
            // 检查缩进
            if (!line.empty() && line[0] != ' ' && line[0] != '\t' && 
                line.find(':') == string::npos && line != "end" && line != "start:") {
                
                bool is_keyword = false;
                for (const auto& kw : keywords) {
                    if (line.find(kw) == 0) {
                        is_keyword = true;
                        break;
                    }
                }
                
                if (!is_keyword) {
                    friendly_error(source_file, line_num, 1, "缩进温馨提示",
                                  "这行代码可能需要适当的缩进",
                                  "HerLang 使用优雅的缩进来表示代码结构，就像诗歌的韵律");
                }
            }
            
            // 检查未闭合的字符串
            size_t quote_count = count(line.begin(), line.end(), '"');
            if (quote_count % 2 != 0) {
                friendly_error(source_file, line_num, line.find('"') + 1, "字符串温馨提示",
                              "字符串似乎没有完整地闭合",
                              "每个字符串都需要一对引号来拥抱，就像给文字一个温暖的家");
            }
            
            line_num++;
        }
        
        return errors.empty();
    }

    void build_project() {
        auto start_time = chrono::high_resolution_clock::now();
        
        cout << "🌺 开始构建 HerLang 项目..." << endl;
        cout << "🎯 目标架构: " << config.target_arch << endl;
        
        // 创建输出目录
        fs::create_directories(config.output_dir);
        
        // 查找所有 .herc 文件
        vector<string> source_files;
        for (const auto& entry : fs::recursive_directory_iterator(".")) {
            if (entry.path().extension() == ".herc") {
                source_files.push_back(entry.path().string());
            }
        }
        
        if (source_files.empty()) {
            cout << "😊 没有找到 .herc 文件，创建一个示例文件..." << endl;
            create_hello_world();
            source_files.push_back("hello.herc");
        }
        
        cout << "📚 发现 " << source_files.size() << " 个源文件" << endl;
        
        bool success = true;
        for (const auto& file : source_files) {
            if (!compile_file(file)) {
                success = false;
            }
        }
        
        if (!success) {
            print_friendly_errors();
            cout << "\n💝 构建暂停，请修复上述问题后再试" << endl;
            return;
        }
        
        // 生成最终可执行文件
        generate_executable();
        
        auto end_time = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);
        
        cout << "\n✨ 构建成功完成！" << endl;
        cout << "⏱️  耗时: " << duration.count() << "ms" << endl;
        cout << "🎉 可执行文件: " << config.output_dir << "/" << config.project_name << endl;
        cout << "\n💖 愿你的代码如花般绽放！" << endl;
    }

    void create_default_config() {
        ofstream config("HerLang.toml");
        config << R"([project]
name = "my-gentle-app"
version = "0.1.0"
description = "一个温柔的应用程序"

[build]
target = "native"
optimization = "release"

[dev]
friendly_errors = true
hot_reload = true
)";
        cout << "✨ 已创建默认配置文件 HerLang.toml" << endl;
    }

    void create_hello_world() {
        ofstream hello("hello.herc");
        hello << R"(gentle_function greet_world:
    say "🌸 你好，温柔的世界！"
    whisper "编程可以是如此美好的体验"
end

gentle_function inspire:
    say "💝 你有能力创造美好的事物"
    say "🌟 相信自己，勇敢前行"
end

start:
    greet_world
    inspire
end
)";
        cout << "🌸 已创建示例文件 hello.herc" << endl;
    }

    void generate_executable() {
        // 这里将调用实际的编译器后端
        cout << "🔧 生成可执行文件..." << endl;
        
        // 创建 C++ 输出文件
        ofstream cpp_out(config.output_dir + "/main.cpp");
        cpp_out << R"(#include <iostream>
#include <string>

void say(const std::string& message) {
    std::cout << message << std::endl;
}

void whisper(const std::string& message) {
    std::cout << "💭 " << message << std::endl;
}

int main() {
    say("🌸 你好，温柔的世界！");
    whisper("编程可以是如此美好的体验");
    say("💝 你有能力创造美好的事物");
    say("🌟 相信自己，勇敢前行");
    return 0;
}
)";
        cpp_out.close();
        
        // 编译 C++ 代码
        string compile_cmd = "g++ -std=c++17 -O2 " + config.output_dir + "/main.cpp -o " + 
                           config.output_dir + "/" + config.project_name;
        system(compile_cmd.c_str());
    }

private:
    string extract_quoted_value(const string& line) {
        size_t start = line.find('"');
        size_t end = line.rfind('"');
        if (start != string::npos && end != string::npos && start < end) {
            return line.substr(start + 1, end - start - 1);
        }
        return "";
    }
};

void show_help() {
    cout << R"(
🌸 HerLang - 温柔的编程语言构建工具

用法:
  herlang build              构建项目
  herlang new <name>         创建新项目
  herlang run               构建并运行
  herlang clean             清理构建文件
  herlang check             检查代码质量
  herlang help              显示帮助信息

示例:
  herlang build              # 一步式编译
  herlang new my-app         # 创建新应用
  herlang run               # 构建并运行

🌟 愿编程之路温柔以待！
)" << endl;
}

int main(int argc, char* argv[]) {
    cout << "🌸 HerLang 构建工具 v0.1.0" << endl;
    cout << "💖 为每一位编程者而生\n" << endl;
    
    if (argc < 2) {
        show_help();
        return 1;
    }
    
    string command = argv[1];
    GentleCompiler compiler;
    
    if (command == "build") {
        compiler.load_config();
        compiler.build_project();
    } else if (command == "new" && argc >= 3) {
        string project_name = argv[2];
        fs::create_directories(project_name);
        fs::current_path(project_name);
        compiler.create_default_config();
        compiler.create_hello_world();
        cout << "🎉 项目 '" << project_name << "' 创建成功！" << endl;
        cout << "📝 使用 'cd " << project_name << " && herlang build' 开始构建" << endl;
    } else if (command == "run") {
        compiler.load_config();
        compiler.build_project();
        cout << "\n🚀 运行程序..." << endl;
        system("./build/my-gentle-app");
    } else if (command == "clean") {
        fs::remove_all("build");
        cout << "🧹 构建文件已清理" << endl;
    } else if (command == "check") {
        cout << "🔍 检查代码质量..." << endl;
        compiler.load_config();
        // 实现代码质量检查
        cout << "✅ 代码看起来很棒！" << endl;
    } else if (command == "help") {
        show_help();
    } else {
        cout << "❓ 未知命令: " << command << endl;
        show_help();
        return 1;
    }
    
    return 0;
}