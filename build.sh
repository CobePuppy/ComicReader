#!/bin/bash

echo "ComicReader Build Script"
echo "========================"

# 检查Qt环境
if ! command -v qmake &> /dev/null; then
    echo "Error: qmake not found in PATH"
    echo "Please install Qt and add qmake to your PATH"
    exit 1
fi

# 显示Qt版本
echo "Qt Version:"
qmake --version

# 创建构建目录
mkdir -p build
cd build

# 配置项目
echo ""
echo "Configuring project..."
qmake ../ComicReader.pro -config release

if [ $? -ne 0 ]; then
    echo "Error: Failed to configure project"
    exit 1
fi

# 编译项目
echo ""
echo "Building project..."
make -j$(nproc)

if [ $? -ne 0 ]; then
    echo "Error: Build failed"
    exit 1
fi

echo ""
echo "Build completed successfully!"
echo "Executable: build/ComicReader"

# 询问是否运行测试
echo ""
read -p "Run unit tests? (y/n): " runTests
if [[ $runTests == "y" || $runTests == "Y" ]]; then
    echo ""
    echo "Building tests..."
    cd ../tests
    mkdir -p build
    cd build
    qmake ../tests.pro
    make -j$(nproc)
    if [ $? -eq 0 ]; then
        echo ""
        echo "Running tests..."
        ./ComicReaderTests
    fi
fi

echo ""
echo "Done!"