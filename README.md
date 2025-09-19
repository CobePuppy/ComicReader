# ComicReader

一个功能强大的现代化漫画阅读器，基于Qt 6开发，提供优秀的阅读体验和完善的管理功能。

## 🌟 核心特性

### � 智能书签系统
- **完整书签管理**: 支持添加、编辑、删除和搜索书签
- **阅读进度跟踪**: 自动记录每本漫画的阅读进度和时间
- **标签分类**: 使用标签对书签进行分类管理
- **导入导出**: 支持书签数据的备份和恢复
- **智能搜索**: 支持按标题、描述和标签搜索书签

### � 高性能缓存系统
- **两级缓存**: 内存+磁盘双重缓存机制
- **智能预加载**: 自动预加载相邻页面提升阅读体验
- **压缩存储**: JPEG压缩减少存储空间占用
- **LRU策略**: 最近最少使用的淘汰算法
- **异步加载**: 非阻塞的图片加载机制

### ⬇️ 强大的下载管理器
- **多线程下载**: 支持同时下载多个文件
- **断点续传**: 网络中断后可续传下载
- **速度控制**: 可设置下载速度限制
- **队列管理**: 完整的下载队列和状态管理
- **进度监控**: 实时显示下载进度和统计信息

### 🛡️ 统一错误处理
- **分级错误处理**: Info、Warning、Error、Critical四级分类
- **错误分类**: 按文件系统、网络、压缩包等类别分类
- **日志记录**: 完整的错误日志记录和管理
- **用户友好**: 提供直观的错误信息和解决建议
- **错误统计**: 详细的错误统计和分析

### 🎨 现代化界面
- **直观设计**: 简洁现代的用户界面
- **响应式布局**: 适应不同屏幕尺寸
- **主题支持**: 支持亮色和暗色主题
- **自定义设置**: 丰富的个性化设置选项

## 系统要求

- **操作系统**: Windows 10+, macOS 10.14+, Ubuntu 18.04+
- **Qt版本**: Qt 5.15+ 或 Qt 6.2+
- **编译器**: 支持 C++17 的编译器
- **内存**: 最少 512MB RAM
- **存储**: 至少 100MB 可用空间

## 编译安装

### 前置要求

1. 安装 Qt 开发环境
   - 下载并安装 [Qt](https://www.qt.io/download)
   - 确保安装了以下模块：
     - Qt Core
     - Qt Widgets
     - Qt Network
     - Qt WebEngine

2. 安装编译工具
   - **Windows**: Visual Studio 2019+ 或 MinGW
   - **macOS**: Xcode Command Line Tools
   - **Linux**: GCC 或 Clang

### 编译步骤

1. **克隆代码**
   ```bash
   git clone https://github.com/CobePuppy/ComicReader.git
   cd ComicReader
   ```

2. **使用 Qt Creator 编译**
   - 打开 Qt Creator
   - 选择 "打开项目"
   - 选择 `ComicReader.pro` 文件
   - 配置编译套件
   - 点击 "构建" 按钮

3. **使用命令行编译**
   ```bash
   # 生成 Makefile
   qmake ComicReader.pro
   
   # 编译项目
   make
   
   # Windows 用户使用
   nmake  # 或 mingw32-make
   ```

4. **运行程序**
   ```bash
   # 进入构建目录
   cd build/release  # 或 build/debug
   
   # 运行程序
   ./ComicReader      # Linux/macOS
   ComicReader.exe    # Windows
   ```

## 项目结构

```
ComicReader/
├── src/                    # 源代码目录
│   ├── core/              # 核心功能模块
│   │   ├── network/       # 网络请求相关
│   │   ├── parser/        # 网页解析相关
│   │   ├── downloader/    # 下载管理相关
│   │   ├── cache/         # 缓存管理相关
│   │   └── config/        # 配置管理相关
│   ├── ui/                # 界面组件
│   │   ├── mainwindow/    # 主窗口
│   │   ├── reader/        # 阅读区域
│   │   ├── browser/       # 网站浏览区域
│   │   ├── downloadmanager/ # 下载管理界面
│   │   └── settings/      # 设置界面
│   ├── utils/             # 工具类
│   └── main.cpp           # 程序入口
├── include/               # 头文件目录
│   ├── core/             # 核心模块头文件
│   ├── ui/               # 界面组件头文件
│   └── utils/            # 工具类头文件
├── resources/            # 资源文件
│   ├── icons/           # 图标资源
│   └── styles/          # 样式表
├── tests/               # 单元测试
├── ComicReader.pro      # Qt项目文件
└── README.md           # 项目说明文档
```

## 使用说明

### 基本操作

1. **打开漫画**
   - 点击 "文件" → "打开文件" 选择本地漫画文件
   - 或点击 "文件" → "打开URL" 输入在线漫画地址

2. **阅读控制**
   - 使用工具栏按钮或键盘快捷键翻页
   - 滚动鼠标滚轮进行缩放
   - 拖拽图像进行移动

3. **下载管理**
   - 点击 "工具" → "下载管理器" 打开下载界面
   - 添加下载URL，管理下载任务

4. **个性化设置**
   - 点击 "工具" → "设置" 打开设置界面
   - 根据需要调整各项配置

### 快捷键

- `Ctrl+O`: 打开文件
- `Ctrl+U`: 打开URL
- `Ctrl+D`: 下载管理器
- `Ctrl+,`: 设置
- `F11`: 全屏切换
- `Space`: 下一页
- `Backspace`: 上一页
- `Ctrl++`: 放大
- `Ctrl+-`: 缩小
- `Ctrl+0`: 重置缩放

## 开发计划

### 版本 1.0 (当前)
- [x] 基础阅读功能
- [x] 界面框架搭建
- [x] 配置管理系统
- [ ] 漫画格式解析
- [ ] 下载功能实现

### 版本 1.1 (计划中)
- [ ] 书签和历史记录
- [ ] 漫画库管理
- [ ] 在线漫画网站适配
- [ ] 插件系统

### 版本 1.2 (计划中)
- [ ] 云同步功能
- [ ] 多设备支持
- [ ] 社交分享功能
- [ ] 性能优化

## 贡献指南

欢迎贡献代码！请遵循以下步骤：

1. Fork 本项目
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 创建 Pull Request

### 代码规范

- 使用 C++17 标准
- 遵循 Qt 编码规范
- 添加适当的注释
- 编写单元测试

## 许可证

本项目采用 MIT 许可证 - 查看 [LICENSE](LICENSE) 文件了解详情。

## 联系方式

- 项目主页: https://github.com/CobePuppy/ComicReader
- 问题反馈: https://github.com/CobePuppy/ComicReader/issues
- 邮箱: 2026744219@qq.com

## 致谢

- Qt 框架团队
- 所有贡献者和用户
- 开源社区的支持

---

**ComicReader** - 让漫画阅读更加美好！ 📚✨
