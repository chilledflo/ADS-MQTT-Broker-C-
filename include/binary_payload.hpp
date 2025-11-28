#pragma once

#include <cstdint>
#include <cstring>
#include <vector>
#include <string>

namespace ads_realtime {

// Binary Payload Format für minimale Latenz
// Kompaktes fixed-size Header + variable payload
#pragma pack(push, 1)
struct BinaryPayloadHeader {
    uint8_t version;           // Protocol version (1)
    uint8_t type;              // Payload type (0=single, 1=batch, 2=compressed)
    uint16_t variable_count;   // Anzahl Variablen
    uint32_t total_size;       // Gesamtgröße payload
    uint64_t timestamp_us;     // Timestamp in Mikrosekunden
    uint32_t sequence_number;  // Sequence für Lost-Detection
};

struct VariableHeader {
    uint16_t name_length;      // Länge des Variablennamens
    uint8_t data_type;         // ADS Datentyp (BOOL=1, INT=2, DINT=3, REAL=4, etc.)
    uint32_t data_length;      // Länge der Daten
    uint64_t timestamp_us;     // Variable-spezifischer Timestamp
};
#pragma pack(pop)

enum class PayloadType : uint8_t {
    Single = 0,
    Batch = 1,
    Compressed = 2
};

enum class AdsDataType : uint8_t {
    Bool = 1,
    Byte = 2,
    Word = 3,
    Dword = 4,
    Int16 = 5,
    Int32 = 6,
    Int64 = 7,
    Real32 = 8,
    Real64 = 9,
    String = 10,
    Custom = 255
};

class BinaryPayloadBuilder {
private:
    std::vector<uint8_t> buffer;
    uint32_t sequence_counter = 0;
    
public:
    // Erstellt Single-Variable Payload
    std::vector<uint8_t> create_single(const std::string& name, AdsDataType type, 
                                       const void* data, size_t data_len) {
        buffer.clear();
        
        // Header
        BinaryPayloadHeader header{};
        header.version = 1;
        header.type = static_cast<uint8_t>(PayloadType::Single);
        header.variable_count = 1;
        header.sequence_number = sequence_counter++;
        header.timestamp_us = get_timestamp_us();
        
        // Variable Header
        VariableHeader var_header{};
        var_header.name_length = static_cast<uint16_t>(name.size());
        var_header.data_type = static_cast<uint8_t>(type);
        var_header.data_length = static_cast<uint32_t>(data_len);
        var_header.timestamp_us = header.timestamp_us;
        
        // Total Size berechnen
        header.total_size = sizeof(BinaryPayloadHeader) + sizeof(VariableHeader) + 
                           name.size() + data_len;
        
        // Serialisieren
        append_bytes(&header, sizeof(header));
        append_bytes(&var_header, sizeof(var_header));
        append_bytes(name.data(), name.size());
        append_bytes(data, data_len);
        
        return buffer;
    }
    
    // Erstellt Batch Payload
    std::vector<uint8_t> create_batch(const std::vector<std::tuple<std::string, AdsDataType, 
                                      const void*, size_t>>& variables) {
        buffer.clear();
        
        // Header
        BinaryPayloadHeader header{};
        header.version = 1;
        header.type = static_cast<uint8_t>(PayloadType::Batch);
        header.variable_count = static_cast<uint16_t>(variables.size());
        header.sequence_number = sequence_counter++;
        header.timestamp_us = get_timestamp_us();
        
        // Total Size berechnen
        size_t total = sizeof(BinaryPayloadHeader);
        for (const auto& [name, type, data, len] : variables) {
            total += sizeof(VariableHeader) + name.size() + len;
        }
        header.total_size = static_cast<uint32_t>(total);
        
        // Header schreiben
        append_bytes(&header, sizeof(header));
        
        // Variables schreiben
        for (const auto& [name, type, data, len] : variables) {
            VariableHeader var_header{};
            var_header.name_length = static_cast<uint16_t>(name.size());
            var_header.data_type = static_cast<uint8_t>(type);
            var_header.data_length = static_cast<uint32_t>(len);
            var_header.timestamp_us = get_timestamp_us();
            
            append_bytes(&var_header, sizeof(var_header));
            append_bytes(name.data(), name.size());
            append_bytes(data, len);
        }
        
        return buffer;
    }
    
    // Dekodiert Binary Payload
    static bool decode_header(const uint8_t* data, size_t len, BinaryPayloadHeader& header) {
        if (len < sizeof(BinaryPayloadHeader)) return false;
        std::memcpy(&header, data, sizeof(BinaryPayloadHeader));
        return header.version == 1;
    }
    
private:
    void append_bytes(const void* data, size_t len) {
        const uint8_t* bytes = static_cast<const uint8_t*>(data);
        buffer.insert(buffer.end(), bytes, bytes + len);
    }
    
    static uint64_t get_timestamp_us() {
        auto now = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(
            now.time_since_epoch()).count();
    }
};

} // namespace ads_realtime
