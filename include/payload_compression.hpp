#pragma once

#include <vector>
#include <cstdint>
#include <cstring>
#include <algorithm>

namespace ads_realtime {

// Einfache, schnelle Run-Length Encoding (RLE) Kompression
// Optimal für repetitive Daten (z.B. viele gleiche Werte)
// Sehr schnell (<1µs für kleine Payloads), moderate Compression Ratio
class SimpleCompressor {
public:
    // Komprimiert Daten mit RLE
    static std::vector<uint8_t> compress(const uint8_t* data, size_t len) {
        std::vector<uint8_t> compressed;
        compressed.reserve(len); // Worst case: keine Kompression
        
        size_t i = 0;
        while (i < len) {
            uint8_t current = data[i];
            size_t run_length = 1;
            
            // Zähle aufeinanderfolgende gleiche Bytes
            while (i + run_length < len && data[i + run_length] == current && run_length < 255) {
                run_length++;
            }
            
            if (run_length >= 3) {
                // RLE: [0xFF][count][value]
                compressed.push_back(0xFF); // Escape marker
                compressed.push_back(static_cast<uint8_t>(run_length));
                compressed.push_back(current);
                i += run_length;
            } else {
                // Literal: schreibe direkt (escape 0xFF)
                if (current == 0xFF) {
                    compressed.push_back(0xFF);
                    compressed.push_back(0x00); // 0xFF 0x00 = literal 0xFF
                } else {
                    compressed.push_back(current);
                }
                i++;
            }
        }
        
        return compressed;
    }
    
    // Dekomprimiert RLE Daten
    static std::vector<uint8_t> decompress(const uint8_t* data, size_t len) {
        std::vector<uint8_t> decompressed;
        decompressed.reserve(len * 2); // Schätzung
        
        size_t i = 0;
        while (i < len) {
            if (data[i] == 0xFF) {
                if (i + 1 < len && data[i + 1] == 0x00) {
                    // Literal 0xFF
                    decompressed.push_back(0xFF);
                    i += 2;
                } else if (i + 2 < len) {
                    // RLE: [0xFF][count][value]
                    uint8_t count = data[i + 1];
                    uint8_t value = data[i + 2];
                    for (int j = 0; j < count; j++) {
                        decompressed.push_back(value);
                    }
                    i += 3;
                } else {
                    i++;
                }
            } else {
                decompressed.push_back(data[i]);
                i++;
            }
        }
        
        return decompressed;
    }
    
    // Prüft ob Kompression sinnvoll ist
    static bool should_compress(const uint8_t* data, size_t len) {
        if (len < 64) return false; // Zu klein
        
        // Quick-Check: Zähle repetitive Bytes
        int repetitions = 0;
        for (size_t i = 1; i < std::min(len, size_t(100)); i++) {
            if (data[i] == data[i-1]) repetitions++;
        }
        
        // Wenn >20% repetitiv, lohnt sich Kompression
        return (repetitions * 100 / std::min(len, size_t(100))) > 20;
    }
};

// Dictionary-based Kompression (LZ77-Style)
// Bessere Compression Ratio als RLE, etwas langsamer
class DictionaryCompressor {
private:
    static constexpr size_t WINDOW_SIZE = 4096;
    static constexpr size_t MIN_MATCH = 3;
    static constexpr size_t MAX_MATCH = 18;
    
public:
    // Findet längste Übereinstimmung im Sliding Window
    static std::pair<uint16_t, uint8_t> find_match(
        const uint8_t* data, size_t pos, size_t len) {
        
        size_t window_start = (pos >= WINDOW_SIZE) ? (pos - WINDOW_SIZE) : 0;
        size_t best_offset = 0;
        size_t best_length = 0;
        
        for (size_t i = window_start; i < pos; i++) {
            size_t match_len = 0;
            while (match_len < MAX_MATCH && 
                   pos + match_len < len &&
                   data[i + match_len] == data[pos + match_len]) {
                match_len++;
            }
            
            if (match_len >= MIN_MATCH && match_len > best_length) {
                best_length = match_len;
                best_offset = pos - i;
            }
        }
        
        return {static_cast<uint16_t>(best_offset), static_cast<uint8_t>(best_length)};
    }
    
    // Komprimiert mit Dictionary
    static std::vector<uint8_t> compress(const uint8_t* data, size_t len) {
        std::vector<uint8_t> compressed;
        compressed.reserve(len);
        
        size_t pos = 0;
        while (pos < len) {
            auto [offset, match_len] = find_match(data, pos, len);
            
            if (match_len >= MIN_MATCH) {
                // Match gefunden: [1][offset:12bit][length:4bit]
                compressed.push_back(0x80 | ((offset >> 8) & 0x0F));
                compressed.push_back(offset & 0xFF);
                compressed.push_back((match_len - MIN_MATCH) & 0x0F);
                pos += match_len;
            } else {
                // Literal: [0][value]
                compressed.push_back(data[pos] & 0x7F);
                pos++;
            }
        }
        
        return compressed;
    }
    
    // Dekomprimiert Dictionary Daten
    static std::vector<uint8_t> decompress(const uint8_t* data, size_t len) {
        std::vector<uint8_t> decompressed;
        decompressed.reserve(len * 2);
        
        size_t i = 0;
        while (i < len) {
            if (data[i] & 0x80) {
                // Match: [1][offset:12bit][length:4bit]
                if (i + 2 < len) {
                    uint16_t offset = ((data[i] & 0x0F) << 8) | data[i + 1];
                    uint8_t match_len = (data[i + 2] & 0x0F) + MIN_MATCH;
                    
                    size_t copy_pos = decompressed.size() - offset;
                    for (int j = 0; j < match_len; j++) {
                        decompressed.push_back(decompressed[copy_pos + j]);
                    }
                    i += 3;
                }
            } else {
                // Literal
                decompressed.push_back(data[i]);
                i++;
            }
        }
        
        return decompressed;
    }
};

// Auto-Kompressor: Wählt beste Methode
class PayloadCompressor {
public:
    enum class Method : uint8_t {
        None = 0,
        RLE = 1,
        Dictionary = 2
    };
    
    // Komprimiert automatisch mit bester Methode
    static std::pair<std::vector<uint8_t>, Method> compress_auto(
        const uint8_t* data, size_t len) {
        
        // Zu klein? Keine Kompression
        if (len < 64) {
            return {std::vector<uint8_t>(data, data + len), Method::None};
        }
        
        // Teste beide Methoden
        auto rle = SimpleCompressor::compress(data, len);
        auto dict = DictionaryCompressor::compress(data, len);
        
        // Wähle kleinere
        if (rle.size() < dict.size() && rle.size() < len * 0.9) {
            return {rle, Method::RLE};
        } else if (dict.size() < len * 0.9) {
            return {dict, Method::Dictionary};
        } else {
            return {std::vector<uint8_t>(data, data + len), Method::None};
        }
    }
    
    // Dekomprimiert basierend auf Methode
    static std::vector<uint8_t> decompress(
        const uint8_t* data, size_t len, Method method) {
        
        switch (method) {
            case Method::RLE:
                return SimpleCompressor::decompress(data, len);
            case Method::Dictionary:
                return DictionaryCompressor::decompress(data, len);
            case Method::None:
            default:
                return std::vector<uint8_t>(data, data + len);
        }
    }
    
    // Benchmark
    struct CompressionStats {
        size_t original_size;
        size_t compressed_size;
        double compression_ratio;
        double compression_time_us;
        Method method;
    };
    
    static CompressionStats benchmark(const uint8_t* data, size_t len) {
        auto start = std::chrono::high_resolution_clock::now();
        auto [compressed, method] = compress_auto(data, len);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(
            end - start).count();
        
        return {
            len,
            compressed.size(),
            len > 0 ? static_cast<double>(len) / compressed.size() : 1.0,
            static_cast<double>(duration_us),
            method
        };
    }
};

} // namespace ads_realtime
