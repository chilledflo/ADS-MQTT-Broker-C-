#include "../include/variable_batch.hpp"
#include "../include/binary_payload.hpp"
#include "../include/shared_memory.hpp"
#include <iostream>
#include <thread>
#include <chrono>

using namespace ads_realtime;

// Beispiel 1: Multi-Variable Batching
void example_batching() {
    std::cout << "=== Multi-Variable Batching ===" << std::endl;
    
    VariableBatch batch(10, std::chrono::milliseconds(5));
    
    // Füge mehrere Variables hinzu
    int32_t motor_speed = 1500;
    bool motor_running = true;
    float temperature = 45.7f;
    
    batch.add_variable("Motor.Speed", &motor_speed, sizeof(motor_speed));
    batch.add_variable("Motor.Running", &motor_running, sizeof(motor_running));
    batch.add_variable("Sensor.Temperature", &temperature, sizeof(temperature));
    
    // Serialisiere Batch
    auto data = batch.serialize();
    std::cout << "Batch size: " << data.size() << " bytes" << std::endl;
    std::cout << "Variables in batch: " << batch.size() << std::endl;
    
    batch.clear();
}

// Beispiel 2: Binary Payload Format
void example_binary_payload() {
    std::cout << "\n=== Binary Payload Format ===" << std::endl;
    
    BinaryPayloadBuilder builder;
    
    // Single Variable
    int32_t value = 42;
    auto payload = builder.create_single("Test.Value", AdsDataType::Int32, &value, sizeof(value));
    std::cout << "Single payload size: " << payload.size() << " bytes" << std::endl;
    
    // Batch
    std::vector<std::tuple<std::string, AdsDataType, const void*, size_t>> variables;
    int32_t speed = 1500;
    float temp = 25.5f;
    bool running = true;
    
    variables.emplace_back("Speed", AdsDataType::Int32, &speed, sizeof(speed));
    variables.emplace_back("Temp", AdsDataType::Real32, &temp, sizeof(temp));
    variables.emplace_back("Running", AdsDataType::Bool, &running, sizeof(running));
    
    auto batch_payload = builder.create_batch(variables);
    std::cout << "Batch payload size: " << batch_payload.size() << " bytes" << std::endl;
    
    // Decode Header
    BinaryPayloadHeader header;
    if (BinaryPayloadBuilder::decode_header(batch_payload.data(), batch_payload.size(), header)) {
        std::cout << "Version: " << (int)header.version << std::endl;
        std::cout << "Variables: " << header.variable_count << std::endl;
        std::cout << "Sequence: " << header.sequence_number << std::endl;
    }
}

// Beispiel 3: Shared Memory IPC
void example_shared_memory() {
    std::cout << "\n=== Shared Memory IPC ===" << std::endl;
    
    try {
        // Writer Process
        SharedMemoryRingBuffer<1024 * 64> writer("ADS_IPC", true);
        
        // Schreibe Daten
        const char* msg1 = "Hello from C++ Realtime Engine!";
        const char* msg2 = "High-performance IPC";
        
        writer.write(msg1, strlen(msg1) + 1);
        writer.write(msg2, strlen(msg2) + 1);
        
        std::cout << "Written: " << writer.available_read() << " bytes" << std::endl;
        
        // Reader in separatem Thread (simuliert zweiten Prozess)
        std::thread reader_thread([&]() {
            try {
                SharedMemoryRingBuffer<1024 * 64> reader("ADS_IPC", false);
                
                char buffer[256];
                size_t len = sizeof(buffer);
                
                while (reader.read(buffer, len)) {
                    std::cout << "Read: " << buffer << " (" << len << " bytes)" << std::endl;
                    len = sizeof(buffer);
                }
            } catch (const std::exception& e) {
                std::cerr << "Reader error: " << e.what() << std::endl;
            }
        });
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        reader_thread.join();
        
    } catch (const std::exception& e) {
        std::cerr << "Shared memory error: " << e.what() << std::endl;
    }
}

// Benchmark
void benchmark_all() {
    std::cout << "\n=== Performance Benchmark ===" << std::endl;
    
    const int iterations = 10000;
    
    // Batching Benchmark
    {
        auto start = std::chrono::high_resolution_clock::now();
        VariableBatch batch;
        int32_t value = 42;
        
        for (int i = 0; i < iterations; ++i) {
            batch.add_variable("Test", &value, sizeof(value));
            if (batch.should_flush()) {
                auto data = batch.serialize();
                batch.clear();
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        std::cout << "Batching: " << (duration_us / iterations) << " µs/op" << std::endl;
    }
    
    // Binary Payload Benchmark
    {
        auto start = std::chrono::high_resolution_clock::now();
        BinaryPayloadBuilder builder;
        int32_t value = 42;
        
        for (int i = 0; i < iterations; ++i) {
            auto payload = builder.create_single("Test", AdsDataType::Int32, &value, sizeof(value));
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        std::cout << "Binary Payload: " << (duration_us / iterations) << " µs/op" << std::endl;
    }
}

int main() {
    std::cout << "ADS Realtime C++ - Feature Examples\n" << std::endl;
    
    example_batching();
    example_binary_payload();
    example_shared_memory();
    benchmark_all();
    
    return 0;
}
