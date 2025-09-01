# ComicReader
(正在制作中，目前正在搭建后端逻辑)
一个功能强大的漫画阅读器，支持本地和在线漫画阅读、下载管理等功能。

## 功能特点

### 🔍 多格式支持
- 支持常见漫画格式：CBZ, CBR, ZIP, RAR
- 支持图片格式：JPG, PNG, GIF, BMP, WEBP
- 智能文件识别和解析

### 📖 优秀的阅读体验
- 多种阅读模式：单页、双页、连续滚动
- 灵活的缩放控制：适应窗口、适应宽度、适应高度、自定义缩放
- 平滑滚动和页面切换
- 全屏阅读支持

### 🌐 在线漫画浏览
- 内置网页浏览器
- 支持在线漫画网站浏览
- 智能内容识别和提取

### ⬇️ 下载管理
- 多线程下载支持
- 下载进度监控
- 断点续传功能
- 下载队列管理

### ⚙️ 丰富的设置选项
- 界面主题切换
- 多语言支持
- 网络代理配置
- 缓存管理
- 自定义快捷键

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
