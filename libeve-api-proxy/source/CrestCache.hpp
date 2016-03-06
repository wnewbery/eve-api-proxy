#pragma once
#include "CrestCacheEntry.hpp"
#include "CrestConnectionPool.hpp"
#include <string>
#include <unordered_map>
#include <mutex>

class CrestCache
{
public:
    struct CacheLookupResults
    {
        CrestCacheEntry *entry;
        /**Lock on entry->mutex*/
        std::unique_lock<std::mutex> lock;
    };
    struct CacheLookupFutureResults
    {
        CrestCacheEntry *entry;
        CrestCacheEntry::Status status;
        std::vector<uint8_t> data;
        void wait()
        {
            std::unique_lock<std::mutex> lock(entry->mutex);
            entry->wait(lock);
            status = entry->status;
            data = entry->data;
        }
    };
    
    CrestCache();
    ~CrestCache();
    
    /**Get CREST data.
     * @param path The path to get from CREST.
     */
    CacheLookupFutureResults get_future(const std::string &path);
    CacheLookupResults get_now(const std::string &path);

private:
    std::mutex cache_mutex;
    std::unordered_map<std::string, CrestCacheEntry> cache;
    /**CrestConnectionPool for updating cache entries.*/
    CrestConnectionPool crest_connection_pool;

    CrestCacheEntry *get_locked(const std::string &path, std::unique_lock<std::mutex> &lock);
    void update_entry(CrestCacheEntry &entry);
    void update_entry_completion(CrestCacheEntry *entry, CrestHttpRequest *request);
};