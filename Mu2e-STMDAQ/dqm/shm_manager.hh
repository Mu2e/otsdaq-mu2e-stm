#ifndef SHM_MANAGER_HH
#define SHM_MANAGER_HH

#include "Mu2e-STMDAQ/dqm/dqm_shm_block.hh"
#include "Mu2e-STMDAQ/dqm/dqm_structs.hh"
#include <map>
#include <memory>
#include <mutex>

class SHMmanager {
public:
    // Singleton access
    static SHMmanager& Instance() {
        static SHMmanager instance;
        return instance;
    }

    template <typename T>
    void registerBlock(DQMPageType type, const std::string& shm_name, size_t size, bool persist=false) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (blocks_.count(type)) return; // Already exists
        
        blocks_[type] = std::make_shared<DQMBlock<T>>(shm_name, size, persist);
    }

    template <typename T>
    T* get(DQMPageType type) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!blocks_.count(type)) return nullptr;
        return static_cast<DQMBlock<T>*>(blocks_[type].get())->getTyped();
    }

private:
    SHMmanager() = default;
    std::map<DQMPageType, std::shared_ptr<void>> blocks_;
    std::mutex mutex_;
};
#endif

