
# 编译共享库（编码）
    g++ -shared -fPIC -o libdrc_compressor.so drc_compressor.cpp -I draco/src -L draco/build -ldraco -Wl,-rpath,draco/build
# 编译共享库（解码）
    g++ -shared -fPIC -o libdrc_decoder.so drc2ply.cpp -I draco/src -L draco/build -ldraco -Wl,-rpath,draco/build

    
  -I/path/to/draco/include \
  -L/path/to/draco/lib \
  -ldraco

    g++ -shared -fPIC -o libdrc_compressor.so drc_compressor.cpp \
  -I~/draco \
  -L~/draco/build \
  -ldraco \
  -Wl,-rpath,~/draco/build  # 关键选项：嵌入路径
# 


  -I~/draco \
  -L~/draco/build \
  -ldraco \
  -Wl,-rpath,~/draco/build
g++ test.cpp -o test -I/path/to/draco/src -L/path/to/draco/build -ldraco
draco/src

g++ -shared -fPIC -o libdrc_compressor.so drc_compressor.cpp \
  -I draco/src \
  -L draco/build \
  -ldraco \
  -Wl,-rpath,draco/build