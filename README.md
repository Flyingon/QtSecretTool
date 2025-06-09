# Qt Secret Tool - 密码管理器

一个基于Qt 6.9的跨平台密码管理工具，类似于1Password，但功能更简单实用。

## 功能特性

### 第一版本 (v1.0)
- ✅ 密码的增删改查
- ✅ 全量搜索功能
- ✅ 密码分类管理
- ✅ 收藏功能
- ✅ 数据导入导出（JSON/CSV格式）
- ✅ 本地SQLite数据库存储
- ✅ 现代化的QML界面

### 计划功能
- 🔄 版本管理和数据同步
- 🔄 云端同步（加密）
- 🔄 跨平台数据共享
- 🔄 密码强度检测
- 🔄 自动密码生成
- 🔄 浏览器扩展集成

## 技术架构

### 后端 (C++)
- **Qt 6.9** - 核心框架
- **SQLite** - 本地数据库
- **模块化设计** - 分层架构

### 前端 (QML)
- **Qt Quick 2.15** - 现代UI框架
- **Material Design** - 界面设计风格
- **响应式布局** - 适配不同屏幕尺寸

### 项目结构
```
QtSecretTool/
├── src/                     # C++源代码
│   ├── core/               # 核心业务逻辑
│   │   ├── Application.h/cpp
│   │   └── PasswordManager.h/cpp
│   ├── models/             # 数据模型
│   │   ├── PasswordItem.h/cpp
│   │   └── PasswordListModel.h/cpp
│   ├── database/           # 数据库操作
│   │   └── DatabaseManager.h/cpp
│   ├── crypto/             # 加密相关
│   ├── import_export/      # 导入导出功能
│   └── utils/              # 工具类
├── qml/                    # QML界面文件
│   ├── components/         # 可复用组件
│   ├── pages/              # 页面
│   └── styles/             # 样式
├── resources/              # 资源文件
└── tests/                  # 测试文件
```

## 编译要求

- **Qt 6.8+** (推荐6.9)
- **CMake 3.16+**
- **C++17**编译器
- **SQLite** (Qt内置)

## 编译说明

### macOS
```bash
# 确保安装了Qt 6.9
brew install qt6

# 克隆项目
git clone https://github.com/FlyingonTemp/QtSecretTool.git
cd QtSecretTool

# 创建构建目录
mkdir build && cd build

# 配置和编译
cmake ..
make -j$(nproc)

# 运行
./appQtSecretTool
```

### Windows
```cmd
# 使用Qt Creator打开CMakeLists.txt
# 或者使用命令行：
mkdir build && cd build
cmake -G "Visual Studio 17 2022" ..
cmake --build . --config Release
```

### Linux
```bash
# 安装Qt6开发包
sudo apt install qt6-base-dev qt6-declarative-dev

# 编译
mkdir build && cd build
cmake ..
make -j$(nproc)
./appQtSecretTool
```

## 使用说明

1. **首次启动** - 程序会自动创建本地数据库
2. **添加密码** - 点击"添加密码"按钮创建新的密码条目
3. **搜索密码** - 使用顶部搜索框进行全文搜索
4. **分类管理** - 为密码设置分类标签进行组织
5. **导入导出** - 支持JSON和CSV格式的数据迁移

## 安全性

- 📁 **本地存储** - 数据存储在用户本地，不会上传到任何服务器
- 🔒 **加密存储** - 敏感数据在数据库中加密存储
- 🚫 **无网络请求** - 第一版本完全离线工作
- 🔐 **主密码保护** - (计划中) 主密码验证访问

## 开发计划

### v1.0 - 基础功能 ✅
- 密码管理核心功能
- 本地数据库存储
- 基础界面和操作

### v1.1 - 增强功能 🔄
- 密码生成器
- 数据安全增强
- 界面优化

### v2.0 - 云端同步 🔄
- 加密云端同步
- 多设备数据共享
- 版本控制

## 贡献指南

欢迎提交Issues和Pull Requests！

1. Fork本项目
2. 创建功能分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 打开Pull Request

## 许可证

本项目采用MIT许可证 - 查看 [LICENSE](LICENSE) 文件了解详情

## 作者

- **FlyingonTemp** - *初始开发* - [GitHub](https://github.com/FlyingonTemp)

## 致谢

- Qt团队提供的优秀框架
- SQLite的可靠数据库引擎
- Material Design的设计理念

---

> 💡 这是一个学习项目，旨在展示Qt/QML的现代应用开发实践。欢迎学习和改进！ 