// gentle_ui.hpp - HerLang 温柔的声明式UI框架
// 跨平台、响应式、支持热重载的现代UI系统
// 设计理念：让界面开发变得温柔而直观

#pragma once

#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <map>
#include <variant>
#include <optional>
#include <chrono>
#include <thread>
#include <atomic>
#include <fstream>
#include <filesystem>

namespace herlang::ui {

// === 颜色系统 - 温柔的调色板 ===
struct GentleColor {
    float r, g, b, a;
    
    GentleColor(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}
    
    // 预定义的温柔色彩
    static const GentleColor GENTLE_PINK;      // 温柔粉色
    static const GentleColor SOFT_LAVENDER;    // 柔和薰衣草
    static const GentleColor WARM_CREAM;       // 温暖奶油色
    static const GentleColor PEACEFUL_BLUE;    // 宁静蓝色
    static const GentleColor NATURE_GREEN;     // 自然绿色
    static const GentleColor SUNSET_ORANGE;    // 日落橙色
    static const GentleColor MOONLIGHT_SILVER; // 月光银色
    static const GentleColor PURE_WHITE;       // 纯净白色
    static const GentleColor GENTLE_BLACK;     // 温柔黑色
};

// 定义温柔色彩常量
const GentleColor GentleColor::GENTLE_PINK{1.0f, 0.8f, 0.9f, 1.0f};
const GentleColor GentleColor::SOFT_LAVENDER{0.9f, 0.8f, 1.0f, 1.0f};
const GentleColor GentleColor::WARM_CREAM{1.0f, 0.98f, 0.9f, 1.0f};
const GentleColor GentleColor::PEACEFUL_BLUE{0.8f, 0.9f, 1.0f, 1.0f};
const GentleColor GentleColor::NATURE_GREEN{0.8f, 1.0f, 0.8f, 1.0f};
const GentleColor GentleColor::SUNSET_ORANGE{1.0f, 0.9f, 0.7f, 1.0f};
const GentleColor GentleColor::MOONLIGHT_SILVER{0.9f, 0.9f, 0.95f, 1.0f};
const GentleColor GentleColor::PURE_WHITE{1.0f, 1.0f, 1.0f, 1.0f};
const GentleColor GentleColor::GENTLE_BLACK{0.1f, 0.1f, 0.1f, 1.0f};

// === 主题系统 ===
struct GentleTheme {
    std::string name;
    GentleColor primary;
    GentleColor secondary;
    GentleColor background;
    GentleColor surface;
    GentleColor text;
    GentleColor accent;
    
    // 预定义主题
    static GentleTheme create_light_theme() {
        return {
            "light-gentle",
            GentleColor::GENTLE_PINK,
            GentleColor::SOFT_LAVENDER,
            GentleColor::PURE_WHITE,
            GentleColor::WARM_CREAM,
            GentleColor::GENTLE_BLACK,
            GentleColor::PEACEFUL_BLUE
        };
    }
    
    static GentleTheme create_dark_theme() {
        return {
            "dark-gentle",
            GentleColor::SOFT_LAVENDER,
            GentleColor::GENTLE_PINK,
            GentleColor{0.05f, 0.05f, 0.1f, 1.0f},
            GentleColor{0.1f, 0.1f, 0.15f, 1.0f},
            GentleColor{0.9f, 0.9f, 0.95f, 1.0f},
            GentleColor::NATURE_GREEN
        };
    }
    
    static GentleTheme create_high_contrast_theme() {
        return {
            "high-contrast",
            GentleColor{1.0f, 1.0f, 0.0f, 1.0f}, // 黄色
            GentleColor{0.0f, 1.0f, 1.0f, 1.0f}, // 青色
            GentleColor{0.0f, 0.0f, 0.0f, 1.0f}, // 黑色背景
            GentleColor{0.2f, 0.2f, 0.2f, 1.0f}, // 深灰表面
            GentleColor{1.0f, 1.0f, 1.0f, 1.0f}, // 白色文字
            GentleColor{1.0f, 0.5f, 0.0f, 1.0f}  // 橙色强调
        };
    }
};

// === 响应式状态管理 ===
template<typename T>
class GentleState {
private:
    T value_;
    std::vector<std::function<void(const T&)>> observers_;
    
public:
    explicit GentleState(T initial_value) : value_(std::move(initial_value)) {}
    
    const T& get() const { return value_; }
    
    void set(T new_value) {
        if (value_ != new_value) {
            value_ = std::move(new_value);
            notify_observers();
        }
    }
    
    void observe(std::function<void(const T&)> observer) {
        observers_.push_back(std::move(observer));
    }
    
private:
    void notify_observers() {
        for (const auto& observer : observers_) {
            observer(value_);
        }
    }
};

// === 事件系统 ===
struct GentleEvent {
    enum Type {
        CLICK, HOVER, FOCUS, BLUR, CHANGE, SUBMIT, 
        KEY_PRESS, KEY_RELEASE, TOUCH_START, TOUCH_END
    } type;
    
    std::map<std::string, std::variant<std::string, int, float, bool>> data;
    
    template<typename T>
    std::optional<T> get_data(const std::string& key) const {
        auto it = data.find(key);
        if (it != data.end()) {
            if (auto value = std::get_if<T>(&it->second)) {
                return *value;
            }
        }
        return std::nullopt;
    }
};

using EventHandler = std::function<void(const GentleEvent&)>;

// === 布局系统 ===
struct GentleLayout {
    enum Type { VERTICAL, HORIZONTAL, GRID, FLEX, ABSOLUTE } type;
    
    struct {
        float top = 0, right = 0, bottom = 0, left = 0;
    } padding;
    
    struct {
        float top = 0, right = 0, bottom = 0, left = 0;
    } margin;
    
    struct {
        std::optional<float> width, height;
        std::optional<float> min_width, min_height;
        std::optional<float> max_width, max_height;
    } constraints;
    
    struct {
        enum { START, CENTER, END, STRETCH, SPACE_BETWEEN, SPACE_AROUND } main_axis = START;
        enum { START, CENTER, END, STRETCH } cross_axis = START;
    } alignment;
    
    float gap = 0.0f;
    int flex_grow = 0;
    int flex_shrink = 1;
};

// === UI组件基类 ===
class GentleComponent {
protected:
    std::string id_;
    GentleLayout layout_;
    GentleColor background_color_;
    std::vector<std::unique_ptr<GentleComponent>> children_;
    std::map<GentleEvent::Type, EventHandler> event_handlers_;
    bool visible_ = true;
    bool enabled_ = true;
    
public:
    explicit GentleComponent(const std::string& id = "") 
        : id_(id), background_color_(GentleColor::PURE_WHITE) {}
    
    virtual ~GentleComponent() = default;
    
    // 设置属性的流式接口
    GentleComponent& with_id(const std::string& id) {
        id_ = id;
        return *this;
    }
    
    GentleComponent& with_layout(const GentleLayout& layout) {
        layout_ = layout;
        return *this;
    }
    
    GentleComponent& with_background(const GentleColor& color) {
        background_color_ = color;
        return *this;
    }
    
    GentleComponent& with_visibility(bool visible) {
        visible_ = visible;
        return *this;
    }
    
    GentleComponent& on_event(GentleEvent::Type type, EventHandler handler) {
        event_handlers_[type] = std::move(handler);
        return *this;
    }
    
    // 添加子组件
    template<typename ComponentType, typename... Args>
    ComponentType& add_child(Args&&... args) {
        auto child = std::make_unique<ComponentType>(std::forward<Args>(args)...);
        ComponentType* ptr = child.get();
        children_.push_back(std::move(child));
        return *ptr;
    }
    
    // 纯虚函数，子类必须实现
    virtual void render(const GentleTheme& theme) = 0;
    virtual void update(float delta_time) {}
    
    // 事件处理
    virtual void handle_event(const GentleEvent& event) {
        auto it = event_handlers_.find(event.type);
        if (it != event_handlers_.end()) {
            it->second(event);
        }
        
        // 传递给子组件
        for (auto& child : children_) {
            child->handle_event(event);
        }
    }
    
    const std::string& id() const { return id_; }
    bool is_visible() const { return visible_; }
    bool is_enabled() const { return enabled_; }
};

// === 文本组件 ===
class GentleText : public GentleComponent {
private:
    std::string text_;
    float font_size_ = 16.0f;
    GentleColor text_color_;
    enum class Alignment { LEFT, CENTER, RIGHT, JUSTIFY } alignment_ = Alignment::LEFT;
    bool is_bold_ = false;
    bool is_italic_ = false;
    
public:
    explicit GentleText(const std::string& text = "") 
        : text_(text), text_color_(GentleColor::GENTLE_BLACK) {}
    
    GentleText& with_text(const std::string& text) {
        text_ = text;
        return *this;
    }
    
    GentleText& with_font_size(float size) {
        font_size_ = size;
        return *this;
    }
    
    GentleText& with_color(const GentleColor& color) {
        text_color_ = color;
        return *this;
    }
    
    GentleText& with_alignment(Alignment align) {
        alignment_ = align;
        return *this;
    }
    
    GentleText& bold(bool bold = true) {
        is_bold_ = bold;
        return *this;
    }
    
    GentleText& italic(bool italic = true) {
        is_italic_ = italic;
        return *this;
    }
    
    void render(const GentleTheme& theme) override {
        // 渲染文本的平台特定实现
        std::cout << "🌸 渲染文本: \"" << text_ << "\" (大小: " << font_size_ << ")" << std::endl;
    }
};

// === 按钮组件 ===
class GentleButton : public GentleComponent {
private:
    std::string label_;
    GentleColor button_color_;
    GentleColor text_color_;
    float border_radius_ = 8.0f;
    bool is_pressed_ = false;
    bool is_hovered_ = false;
    
public:
    explicit GentleButton(const std::string& label = "") 
        : label_(label), button_color_(GentleColor::GENTLE_PINK), text_color_(GentleColor::GENTLE_BLACK) {}
    
    GentleButton& with_label(const std::string& label) {
        label_ = label;
        return *this;
    }
    
    GentleButton& with_button_color(const GentleColor& color) {
        button_color_ = color;
        return *this;
    }
    
    GentleButton& with_text_color(const GentleColor& color) {
        text_color_ = color;
        return *this;
    }
    
    GentleButton& with_border_radius(float radius) {
        border_radius_ = radius;
        return *this;
    }
    
    void render(const GentleTheme& theme) override {
        auto current_color = button_color_;
        
        if (is_pressed_) {
            // 按下时稍微变暗
            current_color.r *= 0.9f;
            current_color.g *= 0.9f;
            current_color.b *= 0.9f;
        } else if (is_hovered_) {
            // 悬停时稍微变亮
            current_color.r = std::min(1.0f, current_color.r * 1.1f);
            current_color.g = std::min(1.0f, current_color.g * 1.1f);
            current_color.b = std::min(1.0f, current_color.b * 1.1f);
        }
        
        std::cout << "💝 渲染按钮: \"" << label_ << "\" (圆角: " << border_radius_ << ")" << std::endl;
    }
    
    void handle_event(const GentleEvent& event) override {
        switch (event.type) {
            case GentleEvent::HOVER:
                is_hovered_ = true;
                break;
            case GentleEvent::BLUR:
                is_hovered_ = false;
                is_pressed_ = false;
                break;
            case GentleEvent::CLICK:
                is_pressed_ = !is_pressed_;
                break;
        }
        
        GentleComponent::handle_event(event);
    }
};

// === 输入框组件 ===
class GentleTextInput : public GentleComponent {
private:
    std::string value_;
    std::string placeholder_;
    GentleColor border_color_;
    GentleColor focus_color_;
    bool is_focused_ = false;
    bool is_password_ = false;
    int max_length_ = -1;
    
public:
    explicit GentleTextInput(const std::string& placeholder = "") 
        : placeholder_(placeholder), border_color_(GentleColor::MOONLIGHT_SILVER), focus_color_(GentleColor::PEACEFUL_BLUE) {}
    
    GentleTextInput& with_placeholder(const std::string& placeholder) {
        placeholder_ = placeholder;
        return *this;
    }
    
    GentleTextInput& with_value(const std::string& value) {
        value_ = value;
        return *this;
    }
    
    GentleTextInput& as_password(bool password = true) {
        is_password_ = password;
        return *this;
    }
    
    GentleTextInput& with_max_length(int length) {
        max_length_ = length;
        return *this;
    }
    
    void render(const GentleTheme& theme) override {
        auto current_border = is_focused_ ? focus_color_ : border_color_;
        std::string display_value = is_password_ ? std::string(value_.length(), '*') : value_;
        
        std::cout << "📝 渲染输入框: \"" << (value_.empty() ? placeholder_ : display_value) 
                  << "\" (聚焦: " << (is_focused_ ? "是" : "否") << ")" << std::endl;
    }
    
    void handle_event(const GentleEvent& event) override {
        switch (event.type) {
            case GentleEvent::FOCUS:
                is_focused_ = true;
                break;
            case GentleEvent::BLUR:
                is_focused_ = false;
                break;
            case GentleEvent::KEY_PRESS:
                if (auto key = event.get_data<std::string>("key")) {
                    if (*key == "Backspace" && !value_.empty()) {
                        value_.pop_back();
                    } else if (key->length() == 1 && (max_length_ < 0 || value_.length() < max_length_)) {
                        value_ += *key;
                    }
                }
                break;
        }
        
        GentleComponent::handle_event(event);
    }
    
    const std::string& get_value() const { return value_; }
};

// === 容器组件 ===
class GentleContainer : public GentleComponent {
public:
    explicit GentleContainer(const std::string& id = "") : GentleComponent(id) {}
    
    void render(const GentleTheme& theme) override {
        std::cout << "📦 渲染容器: " << id_ << " (子组件数: " << children_.size() << ")" << std::endl;
        
        // 渲染所有子组件
        for (auto& child : children_) {
            if (child->is_visible()) {
                child->render(theme);
            }
        }
    }
    
    void update(float delta_time) override {
        for (auto& child : children_) {
            child->update(delta_time);
        }
    }
};

// === 应用程序类 ===
class GentleApp {
private:
    std::unique_ptr<GentleComponent> root_component_;
    GentleTheme current_theme_;
    std::atomic<bool> should_exit_{false};
    std::chrono::steady_clock::time_point last_frame_time_;
    
    // 热重载支持
    std::filesystem::file_time_type last_ui_file_time_;
    bool hot_reload_enabled_ = true;
    
public:
    explicit GentleApp(const GentleTheme& theme = GentleTheme::create_light_theme()) 
        : current_theme_(theme), last_frame_time_(std::chrono::steady_clock::now()) {
        
        std::cout << "🌸 创建温柔应用，主题: " << theme.name << std::endl;
    }
    
    template<typename ComponentType, typename... Args>
    ComponentType& set_root(Args&&... args) {
        auto root = std::make_unique<ComponentType>(std::forward<Args>(args)...);
        ComponentType* ptr = root.get();
        root_component_ = std::move(root);
        return *ptr;
    }
    
    void set_theme(const GentleTheme& theme) {
        current_theme_ = theme;
        std::cout << "🎨 切换到主题: " << theme.name << std::endl;
    }
    
    void enable_hot_reload(bool enabled = true) {
        hot_reload_enabled_ = enabled;
        if (enabled) {
            std::cout << "🔥 热重载已启用" << std::endl;
        }
    }
    
    // 主循环
    void run() {
        std::cout << "🚀 启动温柔应用..." << std::endl;
        
        while (!should_exit_.load()) {
            auto current_time = std::chrono::steady_clock::now();
            auto delta = std::chrono::duration<float>(current_time - last_frame_time_).count();
            last_frame_time_ = current_time;
            
            // 检查热重载
            if (hot_reload_enabled_) {
                check_for_ui_changes();
            }
            
            // 更新和渲染
            if (root_component_) {
                root_component_->update(delta);
                root_component_->render(current_theme_);
            }
            
            // 限制帧率（60 FPS）
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
        
        std::cout << "👋 温柔应用已退出" << std::endl;
    }
    
    void exit_gracefully() {
        should_exit_.store(true);
        std::cout << "🌙 应用正在优雅地退出..." << std::endl;
    }
    
    // 事件注入（用于测试或外部控制）
    void inject_event(const GentleEvent& event) {
        if (root_component_) {
            root_component_->handle_event(event);
        }
    }
    
private:
    void check_for_ui_changes() {
        // 检查 UI 文件是否有变化
        const std::string ui_file = "ui_definition.herc";
        if (std::filesystem::exists(ui_file)) {
            auto current_time = std::filesystem::last_write_time(ui_file);
            if (current_time != last_ui_file_time_) {
                last_ui_file_time_ = current_time;
                std::cout << "🔄 检测到UI文件变化，正在热重载..." << std::endl;
                reload_ui_from_file(ui_file);
            }
        }
    }
    
    void reload_ui_from_file(const std::string& filename) {
        // 这里应该解析 HerLang UI 文件并重建组件树
        std::cout << "🌸 从文件重载UI: " << filename << std::endl;
        // 实际实现会调用 HerLang 解析器
    }
};

// === 便利函数 ===
namespace builder {
    
    inline std::unique_ptr<GentleText> text(const std::string& content) {
        return std::make_unique<GentleText>(content);
    }
    
    inline std::unique_ptr<GentleButton> button(const std::string& label) {
        return std::make_unique<GentleButton>(label);
    }
    
    inline std::unique_ptr<GentleTextInput> input(const std::string& placeholder = "") {
        return std::make_unique<GentleTextInput>(placeholder);
    }
    
    inline std::unique_ptr<GentleContainer> container(const std::string& id = "") {
        return std::make_unique<GentleContainer>(id);
    }
    
    // 布局帮助函数
    inline GentleLayout vertical_layout(float gap = 8.0f) {
        GentleLayout layout;
        layout.type = GentleLayout::VERTICAL;
        layout.gap = gap;
        return layout;
    }
    
    inline GentleLayout horizontal_layout(float gap = 8.0f) {
        GentleLayout layout;
        layout.type = GentleLayout::HORIZONTAL;
        layout.gap = gap;
        return layout;
    }
    
    inline GentleLayout grid_layout(int columns = 2, float gap = 8.0f) {
        GentleLayout layout;
        layout.type = GentleLayout::GRID;
        layout.gap = gap;
        return layout;
    }
}

} // namespace herlang::ui

// === 使用示例 ===
/*
使用方法示例：

#include "gentle_ui.hpp"

using namespace herlang::ui;

int main() {
    GentleApp app(GentleTheme::create_light_theme());
    app.enable_hot_reload();
    
    auto& root = app.set_root<GentleContainer>("main");
    root.with_layout(builder::vertical_layout())
        .with_background(GentleColor::WARM_CREAM);
    
    root.add_child<GentleText>()
        .with_text("🌸 欢迎使用 HerLang UI")
        .with_font_size(24.0f)
        .bold();
    
    auto& button = root.add_child<GentleButton>()
        .with_label("点击我")
        .with_button_color(GentleColor::GENTLE_PINK)
        .on_event(GentleEvent::CLICK, [](const GentleEvent&) {
            std::cout << "💖 按钮被温柔地点击了！" << std::endl;
        });
    
    auto& input = root.add_child<GentleTextInput>()
        .with_placeholder("在这里输入你的想法...")
        .on_event(GentleEvent::CHANGE, [](const GentleEvent& e) {
            if (auto text = e.get_data<std::string>("value")) {
                std::cout << "📝 输入内容: " << *text << std::endl;
            }
        });
    
    app.run();
    return 0;
}
*/