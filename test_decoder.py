import ctypes
import os
import logging
import time
import os

lib = ctypes.CDLL('./libdrc_decoder.so')  # Linux/macOS

# 配置日志
logging.basicConfig(
    filename='draco_decoding.log',
    level=logging.INFO,
    format='%(asctime)s - %(message)s',
    datefmt='%Y-%m-%d %H:%M:%S'
)

def decoder_drc_to_ply(drc_file_path, ply_file_path):
        
    start_time = time.time()

    drc_file = drc_file_path.encode('utf-8')
    ply_file = ply_file_path.encode('utf-8')

    success = lib.decode_drc_to_ply(drc_file, ply_file)

    # 检查结果
    if success:
        drc_size = os.path.getsize(drc_file)
        decoded_size = os.path.getsize(ply_file)
        total_time = (time.time() - start_time) * 1000
        logging.info(
            f"文件:{ply_file_path},"
            f"解压前大小: {drc_size/1024/1024:.2f} MB, "
            f"解压后大小: {decoded_size/1024/1024:.2f} MB, "
            f"解码时间: {total_time:.2f} ms"
        )
        return True
    else:
        print(f"解码失败! 输入文件: {drc_file}")
        return False


decoder_drc_to_ply("output_ori_pc_0.05.drc", "ori_pc_0.05_decode.fbx")
