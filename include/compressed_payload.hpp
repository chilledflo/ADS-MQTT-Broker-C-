#pragma once

#include "binary_payload.hpp"
#include "payload_compression.hpp"
#include <vector>

namespace ads_realtime {

// Erweitertes Binary Payload mit Compression
class CompressedPayloadBuilder : public BinaryPayloadBuilder {
private:
    bool enable_compression = true;
    PayloadCompressor::Method preferred_method = PayloadCompressor::Method::RLE;
    
public:
    CompressedPayloadBuilder(bool compress = true) 
        : enable_compression(compress) {}
    
    // Erstellt komprimiertes Single Payload
    std::vector<uint8_t> create_single_compressed(
        const std::string& name, AdsDataType type,
        const void* data, size_t data_len) {
        
        // Erstelle unkomprimiertes Payload
        auto uncompressed = create_single(name, type, data, data_len);
        
        if (!enable_compression || uncompressed.size() < 128) {
            return uncompressed; // Zu klein für Compression
        }
        
        // Komprimiere Payload (ab Header-Ende)
        size_t header_size = sizeof(BinaryPayloadHeader);
        const uint8_t* payload_data = uncompressed.data() + header_size;
        size_t payload_size = uncompressed.size() - header_size;
        
        auto [compressed_data, method] = PayloadCompressor::compress_auto(
            payload_data, payload_size);
        
        // Wenn Compression nicht lohnt, original zurückgeben
        if (method == PayloadCompressor::Method::None) {
            return uncompressed;
        }
        
        // Neues Payload mit Compression
        std::vector<uint8_t> result;
        result.reserve(header_size + compressed_data.size());
        
        // Header kopieren und anpassen
        BinaryPayloadHeader header;
        std::memcpy(&header, uncompressed.data(), header_size);
        header.type = static_cast<uint8_t>(PayloadType::Compressed);
        header.total_size = static_cast<uint32_t>(header_size + 1 + compressed_data.size());
        
        // Header schreiben
        result.insert(result.end(), 
            reinterpret_cast<const uint8_t*>(&header),
            reinterpret_cast<const uint8_t*>(&header) + header_size);
        
        // Compression Method
        result.push_back(static_cast<uint8_t>(method));
        
        // Komprimierte Daten
        result.insert(result.end(), compressed_data.begin(), compressed_data.end());
        
        return result;
    }
    
    // Erstellt komprimiertes Batch Payload
    std::vector<uint8_t> create_batch_compressed(
        const std::vector<std::tuple<std::string, AdsDataType, const void*, size_t>>& variables) {
        
        // Erstelle unkomprimiertes Batch
        auto uncompressed = create_batch(variables);
        
        if (!enable_compression || uncompressed.size() < 256) {
            return uncompressed; // Zu klein
        }
        
        // Komprimiere ab Header-Ende
        size_t header_size = sizeof(BinaryPayloadHeader);
        const uint8_t* payload_data = uncompressed.data() + header_size;
        size_t payload_size = uncompressed.size() - header_size;
        
        auto [compressed_data, method] = PayloadCompressor::compress_auto(
            payload_data, payload_size);
        
        // Nur wenn signifikante Verbesserung
        if (method == PayloadCompressor::Method::None || 
            compressed_data.size() >= payload_size * 0.9) {
            return uncompressed;
        }
        
        // Neues Payload
        std::vector<uint8_t> result;
        result.reserve(header_size + 1 + compressed_data.size());
        
        // Header anpassen
        BinaryPayloadHeader header;
        std::memcpy(&header, uncompressed.data(), header_size);
        header.type = static_cast<uint8_t>(PayloadType::Compressed);
        header.total_size = static_cast<uint32_t>(header_size + 1 + compressed_data.size());
        
        // Schreibe alles
        result.insert(result.end(),
            reinterpret_cast<const uint8_t*>(&header),
            reinterpret_cast<const uint8_t*>(&header) + header_size);
        result.push_back(static_cast<uint8_t>(method));
        result.insert(result.end(), compressed_data.begin(), compressed_data.end());
        
        return result;
    }
    
    // Dekomprimiert Payload
    static std::vector<uint8_t> decompress_payload(const uint8_t* data, size_t len) {
        if (len < sizeof(BinaryPayloadHeader) + 1) {
            return std::vector<uint8_t>(data, data + len);
        }
        
        BinaryPayloadHeader header;
        std::memcpy(&header, data, sizeof(header));
        
        if (header.type != static_cast<uint8_t>(PayloadType::Compressed)) {
            // Nicht komprimiert
            return std::vector<uint8_t>(data, data + len);
        }
        
        // Lese Compression Method
        PayloadCompressor::Method method = static_cast<PayloadCompressor::Method>(
            data[sizeof(BinaryPayloadHeader)]);
        
        // Dekomprimiere Payload
        const uint8_t* compressed_data = data + sizeof(BinaryPayloadHeader) + 1;
        size_t compressed_size = len - sizeof(BinaryPayloadHeader) - 1;
        
        auto decompressed = PayloadCompressor::decompress(
            compressed_data, compressed_size, method);
        
        // Setze Header zurück auf Original-Type
        std::vector<uint8_t> result;
        result.reserve(sizeof(BinaryPayloadHeader) + decompressed.size());
        
        // Original Header wiederherstellen
        header.type = static_cast<uint8_t>(PayloadType::Batch);
        header.total_size = static_cast<uint32_t>(
            sizeof(BinaryPayloadHeader) + decompressed.size());
        
        result.insert(result.end(),
            reinterpret_cast<const uint8_t*>(&header),
            reinterpret_cast<const uint8_t*>(&header) + sizeof(header));
        result.insert(result.end(), decompressed.begin(), decompressed.end());
        
        return result;
    }
    
    // Aktiviere/Deaktiviere Compression
    void set_compression_enabled(bool enabled) {
        enable_compression = enabled;
    }
    
    bool is_compression_enabled() const {
        return enable_compression;
    }
};

} // namespace ads_realtime
