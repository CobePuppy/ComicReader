@echo off
echo ComicReader Build Script
echo ========================

:: 检查Qt环境
where qmake >nul 2>&1
if errorlevel 1 (
    echo Error: qmake not found in PATH
    echo Please install Qt and add qmake to your PATH
    pause
    exit /b 1
)

:: 显示Qt版本
echo Qt Version:
qmake --version

:: 创建构建目录
if not exist "build" mkdir build
cd build

:: 配置项目
echo.
echo Configuring project...
qmake ../ComicReader.pro -config release

if errorlevel 1 (
    echo Error: Failed to configure project
    pause
    exit /b 1
)

:: 编译项目
echo.
echo Building project...

:: 检测编译器类型
where nmake >nul 2>&1
if errorlevel 1 (
    echo Using MinGW make...
    mingw32-make
) else (
    echo Using MSVC nmake...
    nmake
)

if errorlevel 1 (
    echo Error: Build failed
    pause
    exit /b 1
)

echo.
echo Build completed successfully!
echo Executable: build/release/ComicReader.exe

:: 询问是否运行测试
echo.
set /p runTests="Run unit tests? (y/n): "
if /i "%runTests%"=="y" (
    echo.
    echo Building tests...
    cd ../tests
    if not exist "build" mkdir build
    cd build
    qmake ../tests.pro
    nmake
    if not errorlevel 1 (
        echo.
        echo Running tests...
        ComicReaderTests.exe
    )
)

echo.
echo Done!
pause