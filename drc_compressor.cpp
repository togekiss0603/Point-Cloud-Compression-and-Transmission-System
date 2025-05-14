// drc_compressor.cpp
#include <vector>
#include <string>
#include <stdexcept>
#include <fstream>
#include <chrono>
#include "draco/compression/encode.h"
#include "draco/core/encoder_buffer.h" 
#include "draco/point_cloud/point_cloud_builder.h"


extern "C" {
    // 封装压缩点云的函数，使用C兼容的接口
    void write_compressed_point_cloud_to_drc(
        const int16_t* point_cloud, size_t point_cloud_size,
        const uint8_t* color, size_t color_size,
        const char* file_name) {
        try {
            // auto start_time = std::chrono::high_resolution_clock::now();
            size_t num_points = point_cloud_size / 3;
            if (color_size / 3 != num_points) {
                throw std::runtime_error("Point cloud and color data size mismatch.");
            }

            // 创建Draco点云构建器
            draco::PointCloudBuilder builder;
            builder.Start(num_points);

            // 添加位置属性
            const int pos_att_id = builder.AddAttribute(
                draco::GeometryAttribute::POSITION, 3, draco::DataType::DT_INT16);
            for (size_t i = 0; i < num_points; ++i) {
                const int16_t xyz[3] = {
                    point_cloud[3 * i + 0], 
                    point_cloud[3 * i + 1], 
                    point_cloud[3 * i + 2]
                };
                builder.SetAttributeValueForPoint(pos_att_id, draco::PointIndex(i), xyz);
            }

            // 添加颜色属性
            const int color_att_id = builder.AddAttribute(
                draco::GeometryAttribute::COLOR, 3, draco::DataType::DT_UINT8);
            for (size_t i = 0; i < num_points; ++i) {
                const uint8_t rgb[3] = {
                    color[3 * i + 0], 
                    color[3 * i + 1], 
                    color[3 * i + 2]
                };
                builder.SetAttributeValueForPoint(color_att_id, draco::PointIndex(i), rgb);
            }

            // 完成点云构建
            std::unique_ptr<draco::PointCloud> point_cloud_ptr = builder.Finalize(true);

            // 创建编码器并编码点云
            draco::Encoder encoder;
            draco::EncoderBuffer buffer;
            
            // (编码速度，解码速度)，（越大越快，越小越快）
            const int dracoCompressionSpeed = 10;
            const int dracoPositionBits = 14;

            encoder.SetSpeedOptions(dracoCompressionSpeed, dracoCompressionSpeed);

            // 位置精度，越高精度越好
            encoder.SetAttributeQuantization(draco::GeometryAttribute::POSITION, dracoPositionBits);

            encoder.SetEncodingMethod(draco::PointCloudEncodingMethod::POINT_CLOUD_KD_TREE_ENCODING);


            if (!point_cloud_ptr) {
                throw std::runtime_error("Failed to finalize point cloud.");
            }

            draco::Status status = encoder.EncodePointCloudToBuffer(*point_cloud_ptr, &buffer);
            if (!status.ok()) {
                throw std::runtime_error("Failed to encode point cloud: " + status.error_msg_string());
            }

            // 将缓冲区写入文件
            std::ofstream ofs(file_name, std::ios::binary);
            if (!ofs) {
                throw std::runtime_error("Failed to open file for writing: " + std::string(file_name));
            }
            ofs.write(buffer.data(), buffer.size());
            ofs.close();

             // 结束计时并输出
           // auto end_time = std::chrono::high_resolution_clock::now();
            // auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
            // std::cout << "Draco编码耗时: " << duration << "ms" << std::endl;
            // fprintf(stderr, "Exception: %.5f\n", duration);
            
            //  // 写入日志
            // std::ofstream log_file("./ply_to_draco/draco_encoding.log", std::ios::app);
            // if (log_file.is_open()) {
            //     log_file << "文件: " << file_name 
            //              << ", 点数: " << point_cloud_size/3 
            //              << ", 编码时间: " << duration << " 毫秒" << std::endl;
            //     log_file.close();
            // }
        } catch (const std::exception& e) {
            // 打印错误信息（在实际应用中可以记录日志）
            fprintf(stderr, "Exception: %s\n", e.what());
        }
    }
}