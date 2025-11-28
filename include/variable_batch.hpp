#pragma once

#include <vector>
#include <string>
#include <cstring>
#include <chrono>

namespace ads_realtime {

// Binary batch format für effizientes Senden mehrerer Variables
struct VariableBatch {
    struct Entry {
        std::string name;
        std::vector<uint8_t> data;
        uint64_t timestamp_us;
        
        Entry(const std::string& n, const void* d, size_t len)
            : name(n), data(static_cast<const uint8_t*>(d), static_cast<const uint8_t*>(d) + len) {
            auto now = std::chrono::high_resolution_clock::now();
            timestamp_us = std::chrono::duration_cast<std::chrono::microseconds>(
                now.time_since_epoch()).count();
        }
    };
    
    std::vector<Entry> entries;
    size_t max_batch_size;
    std::chrono::microseconds batch_timeout;
    std::chrono::high_resolution_clock::time_point last_flush;
    
    VariableBatch(size_t max_size = 100, std::chrono::microseconds timeout = std::chrono::milliseconds(10))
        : max_batch_size(max_size), batch_timeout(timeout), last_flush(std::chrono::high_resolution_clock::now()) {}
    
    // Fügt Variable zum Batch hinzu
    bool add_variable(const std::string& name, const void* data, size_t size) {
        entries.emplace_back(name, data, size);
        return should_flush();
    }
    
    // Prüft ob Batch geflusht werden soll
    bool should_flush() const {
        if (entries.size() >= max_batch_size) return true;
        
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - last_flush);
        return elapsed >= batch_timeout;
    }
    
    // Serialisiert Batch in Binary Format
    // Format: [count:4][entry1_len:4][entry1_data][entry2_len:4][entry2_data]...
    // Entry Format: [name_len:2][name][timestamp:8][data_len:4][data]
    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> buffer;
        
        // Reserve geschätzte Größe
        size_t estimated_size = 4; // count
        for (const auto& entry : entries) {
            estimated_size += 4 + 2 + entry.name.size() + 8 + 4 + entry.data.size();
        }
        buffer.reserve(estimated_size);
        
        // Anzahl Entries
        uint32_t count = static_cast<uint32_t>(entries.size());
        buffer.insert(buffer.end(), reinterpret_cast<const uint8_t*>(&count), 
                      reinterpret_cast<const uint8_t*>(&count) + sizeof(count));
        
        // Entries
        for (const auto& entry : entries) {
            // Entry Length (placeholder, wird später gefüllt)
            size_t entry_start = buffer.size();
            uint32_t entry_len = 0;
            buffer.insert(buffer.end(), reinterpret_cast<const uint8_t*>(&entry_len), 
                         reinterpret_cast<const uint8_t*>(&entry_len) + sizeof(entry_len));
            
            size_t data_start = buffer.size();
            
            // Name Length + Name
            uint16_t name_len = static_cast<uint16_t>(entry.name.size());
            buffer.insert(buffer.end(), reinterpret_cast<const uint8_t*>(&name_len), 
                         reinterpret_cast<const uint8_t*>(&name_len) + sizeof(name_len));
            buffer.insert(buffer.end(), entry.name.begin(), entry.name.end());
            
            // Timestamp
            buffer.insert(buffer.end(), reinterpret_cast<const uint8_t*>(&entry.timestamp_us), 
                         reinterpret_cast<const uint8_t*>(&entry.timestamp_us) + sizeof(entry.timestamp_us));
            
            // Data Length + Data
            uint32_t data_len = static_cast<uint32_t>(entry.data.size());
            buffer.insert(buffer.end(), reinterpret_cast<const uint8_t*>(&data_len), 
                         reinterpret_cast<const uint8_t*>(&data_len) + sizeof(data_len));
            buffer.insert(buffer.end(), entry.data.begin(), entry.data.end());
            
            // Entry Length zurückschreiben
            entry_len = static_cast<uint32_t>(buffer.size() - data_start);
            std::memcpy(&buffer[entry_start], &entry_len, sizeof(entry_len));
        }
        
        return buffer;
    }
    
    // Leert Batch
    void clear() {
        entries.clear();
        last_flush = std::chrono::high_resolution_clock::now();
    }
    
    // Anzahl Entries
    size_t size() const { return entries.size(); }
    
    // Ist leer?
    bool empty() const { return entries.empty(); }
};

} // namespace ads_realtime
