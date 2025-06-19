// gentle_concurrency.hpp - HerLang 温柔并发系统
// 实现轻量级协程、所有权机制和并发安全
// 设计理念：让并发编程变得温柔而安全

#pragma once

#include <memory>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>
#include <coroutine>
#include <thread>
#include <chrono>
#include <unordered_map>
#include <string>
#include <iostream>

namespace herlang::runtime {

// === 温柔的所有权系统 ===
template<typename T>
class GentleOwnership {
private:
    std::unique_ptr<T> data_;
    mutable std::mutex mutex_;
    std::string owner_name_;
    
public:
    explicit GentleOwnership(T&& value, const std::string& owner = "anonymous") 
        : data_(std::make_unique<T>(std::move(value))), owner_name_(owner) {}
    
    // 温柔地借用（只读访问）
    template<typename Func>
    auto borrow_gently(Func&& func) const {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!data_) {
            throw std::runtime_error("💝 温柔提醒：数据已被转移，无法访问");
        }
        return func(*data_);
    }
    
    // 温柔地可变借用
    template<typename Func>
    auto borrow_mutably(Func&& func) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!data_) {
            throw std::runtime_error("💝 温柔提醒：数据已被转移，无法修改");
        }
        return func(*data_);
    }
    
    // 温柔地转移所有权
    std::unique_ptr<T> transfer_with_care(const std::string& new_owner) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!data_) {
            throw std::runtime_error("💝 温柔提醒：数据已被转移给其他人");
        }
        
        std::cout << "🤝 所有权从 '" << owner_name_ 
                  << "' 温柔地转移给 '" << new_owner << "'" << std::endl;
        
        owner_name_ = new_owner;
        return std::move(data_);
    }
    
    bool is_available() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return data_ != nullptr;
    }
    
    const std::string& current_owner() const { return owner_name_; }
};

// === 温柔的协程任务 ===
struct GentleTask {
    struct promise_type {
        GentleTask get_return_object() {
            return GentleTask{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {
            std::cout << "💔 协程遇到了问题，但我们会温柔地处理它" << std::endl;
        }
    };
    
    std::coroutine_handle<promise_type> handle_;
    std::string task_name_;
    std::chrono::steady_clock::time_point created_at_;
    
    explicit GentleTask(std::coroutine_handle<promise_type> h, 
                       const std::string& name = "unnamed_task")
        : handle_(h), task_name_(name), created_at_(std::chrono::steady_clock::now()) {}
    
    ~GentleTask() {
        if (handle_) {
            handle_.destroy();
        }
    }
    
    // 禁止复制，只允许移动
    GentleTask(const GentleTask&) = delete;
    GentleTask& operator=(const GentleTask&) = delete;
    
    GentleTask(GentleTask&& other) noexcept 
        : handle_(std::exchange(other.handle_, {}))
        , task_name_(std::move(other.task_name_))
        , created_at_(other.created_at_) {}
    
    GentleTask& operator=(GentleTask&& other) noexcept {
        if (this != &other) {
            if (handle_) handle_.destroy();
            handle_ = std::exchange(other.handle_, {});
            task_name_ = std::move(other.task_name_);
            created_at_ = other.created_at_;
        }
        return *this;
    }
    
    bool is_done() const {
        return !handle_ || handle_.done();
    }
    
    void resume_gently() {
        if (handle_ && !handle_.done()) {
            try {
                handle_.resume();
            } catch (const std::exception& e) {
                std::cout << "🌸 任务 '" << task_name_ 
                          << "' 遇到问题: " << e.what() << std::endl;
            }
        }
    }
};

// === 温柔的等待原语 ===
struct GentleYield {
    std::chrono::milliseconds duration_;
    
    explicit GentleYield(std::chrono::milliseconds ms) : duration_(ms) {}
    
    bool await_ready() const noexcept { return false; }
    
    void await_suspend(std::coroutine_handle<>) const {
        std::this_thread::sleep_for(duration_);
    }
    
    void await_resume() const noexcept {}
};

// 温柔地让出控制权
inline GentleYield yield_kindly(std::chrono::milliseconds ms = std::chrono::milliseconds(1)) {
    return GentleYield{ms};
}

// === 温柔的协程调度器 ===
class GentleScheduler {
private:
    std::vector<std::unique_ptr<GentleTask>> tasks_;
    std::mutex tasks_mutex_;
    std::condition_variable cv_;
    std::atomic<bool> should_stop_{false};
    std::vector<std::thread> worker_threads_;
    std::atomic<size_t> active_tasks_{0};
    
    // 统计信息
    std::atomic<size_t> total_tasks_created_{0};
    std::atomic<size_t> total_tasks_completed_{0};
    
public:
    explicit GentleScheduler(size_t num_threads = std::thread::hardware_concurrency()) {
        std::cout << "🌸 创建温柔调度器，使用 " << num_threads << " 个工作线程" << std::endl;
        
        for (size_t i = 0; i < num_threads; ++i) {
            worker_threads_.emplace_back([this, i]() {
                worker_loop(i);
            });
        }
    }
    
    ~GentleScheduler() {
        shutdown_gracefully();
    }
    
    // 温柔地添加任务
    void spawn_gently(std::unique_ptr<GentleTask> task) {
        {
            std::lock_guard<std::mutex> lock(tasks_mutex_);
            tasks_.push_back(std::move(task));
            active_tasks_.fetch_add(1);
            total_tasks_created_.fetch_add(1);
        }
        cv_.notify_one();
        
        std::cout << "🦋 新任务已温柔地加入调度队列" << std::endl;
    }
    
    // 等待所有任务完成
    void await_all_with_patience() {
        std::cout << "🕰️ 耐心等待所有任务完成..." << std::endl;
        
        while (active_tasks_.load() > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        std::cout << "✨ 所有任务都已完成！总共处理了 " 
                  << total_tasks_completed_.load() << " 个任务" << std::endl;
    }
    
    // 优雅地关闭调度器
    void shutdown_gracefully() {
        std::cout << "🌙 温柔地关闭调度器..." << std::endl;
        
        should_stop_.store(true);
        cv_.notify_all();
        
        for (auto& thread : worker_threads_) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        
        std::cout << "💤 调度器已安全关闭" << std::endl;
    }
    
    // 获取调度器状态
    struct SchedulerStats {
        size_t active_tasks;
        size_t total_created;
        size_t total_completed;
        size_t worker_threads;
    };
    
    SchedulerStats get_gentle_stats() const {
        return {
            active_tasks_.load(),
            total_tasks_created_.load(),
            total_tasks_completed_.load(),
            worker_threads_.size()
        };
    }
    
private:
    void worker_loop(size_t worker_id) {
        std::cout << "🌻 工作线程 " << worker_id << " 开始温柔地工作" << std::endl;
        
        while (!should_stop_.load()) {
            std::unique_ptr<GentleTask> task;
            
            {
                std::unique_lock<std::mutex> lock(tasks_mutex_);
                cv_.wait(lock, [this]() {
                    return should_stop_.load() || !tasks_.empty();
                });
                
                if (should_stop_.load()) break;
                
                if (!tasks_.empty()) {
                    task = std::move(tasks_.back());
                    tasks_.pop_back();
                }
            }
            
            if (task) {
                // 温柔地执行任务
                task->resume_gently();
                
                if (task->is_done()) {
                    active_tasks_.fetch_sub(1);
                    total_tasks_completed_.fetch_add(1);
                    std::cout << "✅ 任务 '" << task->task_name_ 
                              << "' 已温柔地完成" << std::endl;
                } else {
                    // 任务还没完成，重新加入队列
                    std::lock_guard<std::mutex> lock(tasks_mutex_);
                    tasks_.push_back(std::move(task));
                }
            }
        }
        
        std::cout << "🌛 工作线程 " << worker_id << " 温柔地结束工作" << std::endl;
    }
};

// === 全局调度器实例 ===
inline GentleScheduler& get_global_scheduler() {
    static GentleScheduler scheduler;
    return scheduler;
}

// === 便利的协程创建函数 ===
template<typename Func>
GentleTask create_gentle_task(Func&& func, const std::string& name = "gentle_task") {
    co_await func();
    std::cout << "🌸 任务 '" << name << "' 正在温柔地执行" << std::endl;
}

// === 温柔的同步原语 ===
class GentleMutex {
private:
    std::mutex mutex_;
    std::string current_holder_;
    
public:
    template<typename Func>
    auto with_gentle_lock(Func&& func, const std::string& holder_name = "anonymous") {
        std::lock_guard<std::mutex> lock(mutex_);
        current_holder_ = holder_name;
        
        try {
            auto result = func();
            current_holder_.clear();
            return result;
        } catch (...) {
            current_holder_.clear();
            throw;
        }
    }
    
    const std::string& current_holder() const { return current_holder_; }
};

// === 温柔的通道通信 ===
template<typename T>
class GentleChannel {
private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool closed_ = false;
    size_t max_capacity_;
    
public:
    explicit GentleChannel(size_t capacity = 100) : max_capacity_(capacity) {}
    
    // 温柔地发送数据
    bool send_with_care(T&& value) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        if (closed_) {
            std::cout << "💔 通道已关闭，无法发送数据" << std::endl;
            return false;
        }
        
        cv_.wait(lock, [this]() {
            return closed_ || queue_.size() < max_capacity_;
        });
        
        if (closed_) return false;
        
        queue_.emplace(std::forward<T>(value));
        cv_.notify_one();
        
        std::cout << "📨 数据已温柔地发送到通道" << std::endl;
        return true;
    }
    
    // 温柔地接收数据
    std::optional<T> receive_with_patience() {
        std::unique_lock<std::mutex> lock(mutex_);
        
        cv_.wait(lock, [this]() {
            return !queue_.empty() || closed_;
        });
        
        if (queue_.empty()) {
            std::cout << "📭 通道为空且已关闭" << std::endl;
            return std::nullopt;
        }
        
        T value = std::move(queue_.front());
        queue_.pop();
        cv_.notify_one();
        
        std::cout << "📬 从通道温柔地接收到数据" << std::endl;
        return value;
    }
    
    // 温柔地关闭通道
    void close_gently() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            closed_ = true;
        }
        cv_.notify_all();
        std::cout << "🔒 通道已温柔地关闭" << std::endl;
    }
    
    bool is_closed() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return closed_;
    }
    
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }
};

// === 温柔的内存管理 ===
class GentleMemoryPool {
private:
    std::vector<std::unique_ptr<std::byte[]>> pools_;
    std::vector<void*> free_blocks_;
    std::mutex mutex_;
    size_t block_size_;
    size_t blocks_per_pool_;
    
public:
    GentleMemoryPool(size_t block_size, size_t initial_blocks = 1024)
        : block_size_(block_size), blocks_per_pool_(initial_blocks) {
        allocate_new_pool();
    }
    
    void* allocate_gently() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (free_blocks_.empty()) {
            allocate_new_pool();
        }
        
        void* ptr = free_blocks_.back();
        free_blocks_.pop_back();
        
        std::cout << "🌱 温柔地分配了 " << block_size_ << " 字节内存" << std::endl;
        return ptr;
    }
    
    void deallocate_gently(void* ptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        free_blocks_.push_back(ptr);
        std::cout << "🌿 温柔地回收了内存" << std::endl;
    }
    
private:
    void allocate_new_pool() {
        auto pool = std::make_unique<std::byte[]>(block_size_ * blocks_per_pool_);
        std::byte* base = pool.get();
        
        for (size_t i = 0; i < blocks_per_pool_; ++i) {
            free_blocks_.push_back(base + i * block_size_);
        }
        
        pools_.push_back(std::move(pool));
        std::cout << "🌳 创建了新的内存池，包含 " << blocks_per_pool_ << " 个块" << std::endl;
    }
};

} // namespace herlang::runtime

// === 使用示例宏定义 ===
#define GENTLE_SPAWN(name, ...) \
    get_global_scheduler().spawn_gently( \
        std::make_unique<GentleTask>(create_gentle_task([=]() -> GentleTask { \
            __VA_ARGS__ \
        }, name)))

#define YIELD_KINDLY(ms) co_await yield_kindly(std::chrono::milliseconds(ms))

#define AWAIT_ALL() get_global_scheduler().await_all_with_patience()