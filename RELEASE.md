# ComicReader 发布说明

## 发布准备

### 1. 编译发布版本

**Windows:**
```bash
# 运行构建脚本
build.bat

# 或手动编译
mkdir build && cd build
qmake ../ComicReader.pro -config release
nmake
```

**Linux/macOS:**
```bash
# 运行构建脚本
./build.sh

# 或手动编译
mkdir build && cd build
qmake ../ComicReader.pro -config release
make -j$(nproc)
```

### 2. 运行测试

确保所有单元测试通过：
```bash
cd tests/build
./ComicReaderTests  # Linux/macOS
ComicReaderTests.exe  # Windows
```

### 3. 打包发布

**Windows:**
```bash
# 使用windeployqt部署Qt依赖
windeployqt.exe ComicReader.exe

# 创建安装包（可选）
# 使用NSIS或Inno Setup创建安装程序
```

**Linux:**
```bash
# 创建AppImage
linuxdeploy --appdir AppDir --output appimage

# 或创建DEB包
dpkg-buildpackage -us -uc
```

**macOS:**
```bash
# 部署Qt框架
macdeployqt ComicReader.app

# 代码签名（如果有开发者证书）
codesign --deep --force --verify --verbose --sign "Developer ID" ComicReader.app
```

## 版本发布清单

- [ ] 更新版本号 (在main.cpp或version.h中)
- [ ] 更新CHANGELOG.md
- [ ] 运行所有单元测试
- [ ] 编译发布版本
- [ ] 测试基本功能
- [ ] 创建发布包
- [ ] 上传到发布平台
- [ ] 更新文档
- [ ] 发布公告

## 文件清单

发布包应包含：
- ComicReader可执行文件
- Qt运行时库（Windows）
- 用户指南 (docs/用户指南.md)
- 许可证文件
- README.md

## 更新日志模板

```markdown
## v1.0.0 - 2024-01-01

### 新增功能
- 智能书签系统
- 高性能缓存机制
- 强大的下载管理器
- 统一错误处理系统
- 完整的单元测试框架

### 改进
- 优化界面响应速度
- 提升内存使用效率
- 改善错误提示信息

### 修复
- 修复缓存清理问题
- 修复书签搜索bug
- 修复下载进度显示问题

### 已知问题
- 暂无
```