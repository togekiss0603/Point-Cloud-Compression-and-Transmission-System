#include <draco/compression/decode.h>
#include <draco/core/decoder_buffer.h>
#include <draco/point_cloud/point_cloud_builder.h>
#include <draco/attributes/point_attribute.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <cstdint>
#include <chrono>

int drc2ply(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <drc_file> <ply_filename>" << std::endl;
        return 1;
    }

    const char *drc_file_name = argv[1];
    const char *ply_file_name = argv[2];


    // Read the compressed data from the file.
    std::ifstream ifs(drc_file_name, std::ios::binary);
    if (!ifs)
    {
        std::cerr << "Failed to open file for reading: " << drc_file_name << std::endl;
        return 1;
    }

    std::vector<char> data((std::istreambuf_iterator<char>(ifs)),
                           std::istreambuf_iterator<char>());
    ifs.close();

    // Create a decoder buffer and initialize it with the compressed data.
    draco::DecoderBuffer buffer;
    buffer.Init(data.data(), data.size());

    // Create a decoder.
    draco::Decoder decoder;

    // Decode the point cloud.
    std::unique_ptr<draco::PointCloud> pc = decoder.DecodePointCloudFromBuffer(&buffer).value();
    if (!pc)
    {
        std::cerr << "Failed to decode point cloud." << std::endl;
        return 1;
    }

    const int num_points = pc->num_points();
    const int pos_att_id = pc->GetNamedAttributeId(draco::GeometryAttribute::POSITION);
    const int color_att_id = pc->GetNamedAttributeId(draco::GeometryAttribute::COLOR);

    if (pos_att_id == -1)
    {
        std::cerr << "Position attribute not found." << std::endl;
        return 1;
    }

    const draco::PointAttribute *pos_att = pc->attribute(pos_att_id);
    const draco::PointAttribute *color_att = color_att_id != -1 ? pc->attribute(color_att_id) : nullptr;

    // Write to PLY file
    std::stringstream ss;
    std::ofstream ply_file(ply_file_name);
    if (!ply_file.is_open())
    {
        std::cerr << "Failed to open" << ply_file_name << "for writing." << std::endl;
        return 1;
    }

    ply_file << "ply\n";
    ply_file << "format ascii 1.0\n";
    ply_file << "element vertex " << num_points << "\n";
    ply_file << "property float x\n";
    ply_file << "property float y\n";
    ply_file << "property float z\n";
    if (color_att)
    {
        ply_file << "property uchar red\n";
        ply_file << "property uchar green\n";
        ply_file << "property uchar blue\n";
    }
    ply_file << "end_header\n";

    int16_t pos_buffer_int16[3]; // Use int16_t buffer
    float pos_buffer_float[3];   // Float buffer for output
    uint8_t color_buffer[3];

    for (draco::PointIndex i(0); i < num_points; ++i)
    {
        draco::AttributeValueIndex att_index(i.value());
        pos_att->GetValue(att_index, static_cast<void *>(pos_buffer_int16));
        for (int j = 0; j < 3; ++j)
        {
            pos_buffer_float[j] = static_cast<float>(pos_buffer_int16[j]);
        }
        ply_file << pos_buffer_float[0] << " " << pos_buffer_float[1] << " " << pos_buffer_float[2] << " ";
        if (color_att)
        {
            color_att->GetValue(att_index, static_cast<void *>(color_buffer));
            ply_file << (int)color_buffer[0] << " " << (int)color_buffer[1] << " " << (int)color_buffer[2];
        }
        ply_file << "\n";
    }

    ply_file.close();
    std::cout << "Point cloud written to " << ply_file_name << std::endl;
    return 0;
}

extern "C" {
    bool decode_drc_to_ply(const char* drc_file, const char* ply_file) {
        char* argv[3] = {
            const_cast<char*>("drc2ply"),  // 程序名
            const_cast<char*>(drc_file),   // 输入文件
            const_cast<char*>(ply_file)    // 输出文件
        };
        return drc2ply(3, argv) == 0;  // 返回成功/失败
    }
}