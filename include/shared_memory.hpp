#pragma once

#include <windows.h>
#include <string>
#include <stdexcept>
#include <cstring>
#include <atomic>

namespace ads_realtime {

// Shared Memory Ring Buffer für Lock-Free IPC
template<size_t BufferSize = 1024 * 1024> // 1MB default
class SharedMemoryRingBuffer {
private:
    struct Header {
        std::atomic<uint64_t> write_pos;
        std::atomic<uint64_t> read_pos;
        uint64_t buffer_size;
        std::atomic<uint32_t> writer_pid;
        std::atomic<uint32_t> reader_pid;
    };
    
    HANDLE hMapFile = nullptr;
    void* pBuf = nullptr;
    Header* header = nullptr;
    uint8_t* data_buffer = nullptr;
    bool is_creator = false;
    
public:
    SharedMemoryRingBuffer(const std::string& name, bool create = true) {
        std::wstring wname(name.begin(), name.end());
        
        if (create) {
            // Erstelle Shared Memory
            hMapFile = CreateFileMappingW(
                INVALID_HANDLE_VALUE,
                nullptr,
                PAGE_READWRITE,
                0,
                sizeof(Header) + BufferSize,
                wname.c_str()
            );
            
            if (hMapFile == nullptr) {
                throw std::runtime_error("CreateFileMapping failed");
            }
            
            is_creator = (GetLastError() != ERROR_ALREADY_EXISTS);
        } else {
            // Öffne existierendes Shared Memory
            hMapFile = OpenFileMappingW(
                FILE_MAP_ALL_ACCESS,
                FALSE,
                wname.c_str()
            );
            
            if (hMapFile == nullptr) {
                throw std::runtime_error("OpenFileMapping failed");
            }
        }
        
        // Map View
        pBuf = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        if (pBuf == nullptr) {
            CloseHandle(hMapFile);
            throw std::runtime_error("MapViewOfFile failed");
        }
        
        header = static_cast<Header*>(pBuf);
        data_buffer = static_cast<uint8_t*>(pBuf) + sizeof(Header);
        
        if (is_creator) {
            // Initialisiere Header
            header->write_pos.store(0);
            header->read_pos.store(0);
            header->buffer_size = BufferSize;
            header->writer_pid.store(GetCurrentProcessId());
            header->reader_pid.store(0);
        }
    }
    
    ~SharedMemoryRingBuffer() {
        if (pBuf) UnmapViewOfFile(pBuf);
        if (hMapFile) CloseHandle(hMapFile);
    }
    
    // Schreibt Daten (Writer)
    bool write(const void* data, size_t len) {
        if (len > BufferSize / 2) return false; // Zu groß
        
        uint64_t write_pos = header->write_pos.load(std::memory_order_acquire);
        uint64_t read_pos = header->read_pos.load(std::memory_order_acquire);
        
        // Prüfe ob genug Platz
        uint64_t available = (read_pos <= write_pos) ?
            (BufferSize - (write_pos - read_pos)) :
            (read_pos - write_pos);
        
        if (available < len + sizeof(uint32_t)) return false;
        
        // Schreibe Länge
        uint32_t msg_len = static_cast<uint32_t>(len);
        write_wrapped(&msg_len, sizeof(msg_len), write_pos);
        write_pos = (write_pos + sizeof(msg_len)) % BufferSize;
        
        // Schreibe Daten
        write_wrapped(data, len, write_pos);
        write_pos = (write_pos + len) % BufferSize;
        
        header->write_pos.store(write_pos, std::memory_order_release);
        return true;
    }
    
    // Liest Daten (Reader)
    bool read(void* data, size_t& len) {
        uint64_t write_pos = header->write_pos.load(std::memory_order_acquire);
        uint64_t read_pos = header->read_pos.load(std::memory_order_acquire);
        
        if (read_pos == write_pos) return false; // Leer
        
        // Lese Länge
        uint32_t msg_len;
        read_wrapped(&msg_len, sizeof(msg_len), read_pos);
        read_pos = (read_pos + sizeof(msg_len)) % BufferSize;
        
        if (msg_len > len) {
            len = msg_len;
            return false; // Buffer zu klein
        }
        
        // Lese Daten
        read_wrapped(data, msg_len, read_pos);
        read_pos = (read_pos + msg_len) % BufferSize;
        
        header->read_pos.store(read_pos, std::memory_order_release);
        len = msg_len;
        return true;
    }
    
    // Statistiken
    size_t available_read() const {
        uint64_t write_pos = header->write_pos.load(std::memory_order_acquire);
        uint64_t read_pos = header->read_pos.load(std::memory_order_acquire);
        return (write_pos >= read_pos) ? (write_pos - read_pos) : 
               (BufferSize - read_pos + write_pos);
    }
    
    size_t available_write() const {
        return BufferSize - available_read();
    }
    
    bool is_empty() const {
        return header->write_pos.load() == header->read_pos.load();
    }
    
private:
    void write_wrapped(const void* data, size_t len, uint64_t pos) {
        pos = pos % BufferSize;
        const uint8_t* src = static_cast<const uint8_t*>(data);
        
        if (pos + len <= BufferSize) {
            std::memcpy(data_buffer + pos, src, len);
        } else {
            size_t first_part = BufferSize - pos;
            std::memcpy(data_buffer + pos, src, first_part);
            std::memcpy(data_buffer, src + first_part, len - first_part);
        }
    }
    
    void read_wrapped(void* data, size_t len, uint64_t pos) {
        pos = pos % BufferSize;
        uint8_t* dst = static_cast<uint8_t*>(data);
        
        if (pos + len <= BufferSize) {
            std::memcpy(dst, data_buffer + pos, len);
        } else {
            size_t first_part = BufferSize - pos;
            std::memcpy(dst, data_buffer + pos, first_part);
            std::memcpy(dst + first_part, data_buffer, len - first_part);
        }
    }
};

} // namespace ads_realtime
