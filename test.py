import numpy as np
import ctypes
from plyfile import PlyData
import logging
import time
import os

# 加载共享库
lib = ctypes.CDLL('./libdrc_compressor.so')  # Linux/macOS
# lib = ctypes.CDLL('./drc_compressor.dll')  # Windows

# 定义函数参数和返回类型
lib.write_compressed_point_cloud_to_drc.argtypes = [
    np.ctypeslib.ndpointer(dtype=np.int16, flags='C_CONTIGUOUS'),
    ctypes.c_size_t,
    np.ctypeslib.ndpointer(dtype=np.uint8, flags='C_CONTIGUOUS'),
    ctypes.c_size_t,
    ctypes.c_char_p
]
lib.write_compressed_point_cloud_to_drc.restype = None

# 配置日志
logging.basicConfig(
    filename='draco_encoding.log',
    level=logging.INFO,
    format='%(asctime)s - %(message)s',
    datefmt='%Y-%m-%d %H:%M:%S'
)

def compress_ply_to_drc(ply_file_path, output_drc_file_path):
    original_size = os.path.getsize(ply_file_path)

    start_time = time.time()

    # 读取PLY文件
    ply_data = PlyData.read(ply_file_path)
    
    # 提取点云数据
    vertices = ply_data['vertex']
    
    # 提取坐标
    points = np.vstack([vertices['x'], vertices['y'], vertices['z']]).T
    # 转换为int16（根据需要进行缩放）
    points_min = np.min(points, axis=0)
    points_max = np.max(points, axis=0)
    points_range = points_max - points_min
    # 缩放因子，根据点云范围调整
    scale = 32767.0 / np.max(points_range)
    points_scaled = (points - points_min) * scale
    points_int16 = points_scaled.astype(np.int16)
    
    # 提取颜色（如果存在）
    if 'red' in vertices and 'green' in vertices and 'blue' in vertices:
        colors = np.vstack([vertices['red'], vertices['green'], vertices['blue']]).T
        colors_uint8 = colors.astype(np.uint8) 
    else:
        # 如果没有颜色，创建默认颜色（白色）
        num_points = len(points)
        colors_uint8 = np.ones((num_points, 3), dtype=np.uint8) * 255
    num_points = len(vertices)

    # 展平数组
    points_flat = points_int16.flatten()
    colors_flat = colors_uint8.flatten()
    
    # 调用C++函数
    lib.write_compressed_point_cloud_to_drc(
        points_flat, 
        len(points_flat),
        colors_flat,
        len(colors_flat),
        output_drc_file_path.encode('utf-8')
    )
    
    compressed_size = os.path.getsize(output_drc_file_path)

    total_time = (time.time() - start_time) * 1000
    logging.info(
        f"文件: {ply_file_path}, "
        f"点数: {num_points}, "
        f"原始大小: {original_size/1024/1024:.2f} MB, "
        f"压缩后: {compressed_size/1024/1024:.2f} MB, "
        f"编码时间: {total_time:.2f} ms"
    )
    print(f"点云已压缩并保存到 {output_drc_file_path}")

# 使用示例
compress_ply_to_drc("woman1.ply", "woman1_encode_fast.drc")