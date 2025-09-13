# ComicReader 图标资源说明

## 目录结构
```
resources/icons/
├── app/                 # 应用程序图标
│   ├── icon_16.png      # 16x16 应用图标
│   ├── icon_32.png      # 32x32 应用图标
│   ├── icon_48.png      # 48x48 应用图标
│   ├── icon_64.png      # 64x64 应用图标
│   ├── icon_128.png     # 128x128 应用图标
│   └── icon_256.png     # 256x256 应用图标
├── toolbar/             # 工具栏图标
│   ├── open.png         # 打开文件
│   ├── close.png        # 关闭文件
│   ├── prev_page.png    # 上一页
│   ├── next_page.png    # 下一页
│   ├── zoom_in.png      # 放大
│   ├── zoom_out.png     # 缩小
│   ├── zoom_fit.png     # 适合窗口
│   ├── fullscreen.png   # 全屏模式
│   └── settings.png     # 设置
├── menu/                # 菜单图标
│   ├── file.png         # 文件菜单
│   ├── edit.png         # 编辑菜单
│   ├── view.png         # 视图菜单
│   ├── tools.png        # 工具菜单
│   └── help.png         # 帮助菜单
├── library/             # 图书馆相关图标
│   ├── folder.png       # 文件夹
│   ├── comic.png        # 漫画文件
│   ├── favorite.png     # 收藏
│   ├── recent.png       # 最近阅读
│   └── search.png       # 搜索
└── status/              # 状态图标
    ├── loading.png      # 加载中
    ├── error.png        # 错误
    ├── success.png      # 成功
    └── warning.png      # 警告
```

## 图标规格要求

### 应用图标
- **格式**: PNG，支持透明背景
- **尺寸**: 16x16, 32x32, 48x48, 64x64, 128x128, 256x256 像素
- **设计**: 简洁现代，体现漫画阅读应用的特色
- **颜色**: 使用现代扁平化设计风格

### 工具栏图标
- **格式**: PNG，支持透明背景
- **尺寸**: 24x24 像素（标准），32x32 像素（高DPI）
- **设计**: 单色线条图标，适配浅色和深色主题
- **颜色**: 深灰色 (#333333) 或白色 (#FFFFFF)

### 菜单图标
- **格式**: PNG，支持透明背景
- **尺寸**: 16x16 像素
- **设计**: 简单清晰的符号
- **颜色**: 与系统主题保持一致

## 图标设计建议

### 主应用图标设计元素
1. **基础形状**: 书本、漫画书或阅读器造型
2. **视觉元素**: 
   - 可以包含漫画气泡、对话框
   - 使用鲜明的色彩搭配
   - 体现数字化、现代感
3. **色彩方案**: 
   - 主色调：蓝色系 (#2196F5)
   - 辅助色：橙色 (#FF9800) 或绿色 (#4CAF50)

### 工具栏图标设计原则
1. **一致性**: 所有图标使用相同的线条粗细和风格
2. **识别性**: 每个图标功能明确，易于理解
3. **适配性**: 支持不同主题和分辨率

## 在Qt中使用图标

### 设置应用图标
```cpp
// 在main.cpp中设置应用图标
QApplication app(argc, argv);
app.setWindowIcon(QIcon(":/icons/app/icon_64.png"));
```

### 在工具栏中使用图标
```cpp
// 在MainWindow中创建工具栏按钮
QAction *openAction = new QAction(QIcon(":/icons/toolbar/open.png"), "打开", this);
QAction *prevAction = new QAction(QIcon(":/icons/toolbar/prev_page.png"), "上一页", this);
```

## 资源文件配置
需要创建 resources.qrc 文件来管理图标资源。

## 实现状态

- [x] 创建图标目录结构
- [x] 编写图标使用说明文档
- [ ] 获取或设计具体的图标文件
- [ ] 创建Qt资源文件(resources.qrc)
- [ ] 在代码中集成图标使用
