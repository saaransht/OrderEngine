#pragma once

#include <memory>
#include <vector>
#include <stack>
#include <mutex>

namespace OrderEngine {

template<typename T>
class MemoryPool {
public:
    explicit MemoryPool(size_t initial_size = 1000) {
        reserve(initial_size);
    }
    
    ~MemoryPool() = default;
    
    template<typename... Args>
    std::unique_ptr<T> acquire(Args&&... args) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (available_.empty()) {
            reserve(pool_.size() == 0 ? initial_size_ : pool_.size() * 2);
        }
        
        T* ptr = available_.top();
        available_.pop();
        
        // Destroy existing object and construct new one in-place
        ptr->~T();
        new(ptr) T(std::forward<Args>(args)...);
        
        return std::unique_ptr<T>(ptr, [this](T* p) { release(p); });
    }
    
private:
    std::vector<std::unique_ptr<T>> pool_;
    std::stack<T*> available_;
    std::mutex mutex_;
    size_t initial_size_{1000};
    
    void reserve(size_t size) {
        size_t current_size = pool_.size();
        pool_.reserve(size);
        
        for (size_t i = current_size; i < size; ++i) {
            auto ptr = std::make_unique<T>();
            available_.push(ptr.get());
            pool_.push_back(std::move(ptr));
        }
    }
    
    void release(T* ptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        available_.push(ptr);
    }
};

} // namespace OrderEngine
