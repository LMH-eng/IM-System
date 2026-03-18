# IM-System

基于 Qt + WinSocket + MySQL 的即时通讯系统

## 项目结构

```
IM-System/
├── Client/     # Qt 客户端
└── Server/     # C++ 服务端
```

## 技术栈

### 客户端
- Qt 5.12
- C++11
- TCP Socket

### 服务端
- WinSocket
- MySQL
- C++11

## 功能模块

| 功能 | 说明 |
|------|------|
| 用户注册 | 手机号注册，密码加密存储 |
| 用户登录 | 登录验证，在线状态管理 |
| 好友管理 | 添加好友，好友列表 |
| 即时聊天 | 点对点消息转发 |
| 心跳机制 | 在线检测，异常断线处理 |

## 架构设计

采用三层架构：
- **业务层 (Kernel)**: 处理业务逻辑
- **中介者层 (Mediator)**: 解耦业务层和网络层
- **网络层 (Net)**: 封装 Socket 通信

## 编译运行

### 客户端
1. 安装 Qt 5.12
2. 打开 `Client/IMCllient.pro`
3. 编译运行

### 服务端
1. 安装 Visual Studio 2022
2. 配置 MySQL 环境
3. 打开 `Server/imserver.vcxproj`
4. 编译运行

## 数据库配置

服务端需要 MySQL 数据库，创建 `t_user` 表：

```sql
CREATE TABLE t_user (
    id INT PRIMARY KEY AUTO_INCREMENT,
    tel VARCHAR(20),
    pass VARCHAR(50),
    name VARCHAR(50),
    iconid INT,
    feeling VARCHAR(100)
);
```

## 作者

LMH-eng
