#pragma once
#include <vector>
#include <cstdint>

void write_compressed_point_cloud_to_drc(const std::vector<int16_t> &point_cloud,
                                         const std::vector<uint8_t> &color,
                                         const char *file_name);
