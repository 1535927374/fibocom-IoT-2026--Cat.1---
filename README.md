<img width="982" height="510" alt="image" src="https://github.com/user-attachments/assets/e354a62b-a466-4f29-9924-a78c7c0587db" /><img width="982" height="510" alt="image" src="https://github.com/user-attachments/assets/b317cbad-d2c8-4238-bdfb-5ea9bc7def17" /># ⚡ 作品名称
> 赛事： 全国大学生物联网设计竞赛
> 奖项： 二等奖
> 年份：202X
> 平台：SC171 / L610
> 团队：团队名称 · 学校名称

## 📖 作品简介
  本项目为广和通 Cat.1 物联云协同智能感知控制终端，是全国大学生物联网设计竞赛获奖作品。终端基于 STM32 主控，搭载广和通 Cat.1 通信模组，实现多传感器环境数据采集、本地逻辑控制与云端数据双向通信。系统可实时采集温湿度、人体感应等环境信息，支持远程下发指令控制外设，将感知数据上传至物联网云平台完成可视化展示、数据存储与超限告警。
  项目兼顾硬件底层驱动、嵌入式程序开发与云端对接，完成从感知、传输到控制的完整物联网链路。整套方案低功耗、部署便捷，可应用于室内环境监测、小型智能安防场景。本仓库存放项目硬件原理图、嵌入式源码、调试文档与云平台配置脚本，方便代码复用、迭代优化与项目成果复盘。

## 🧠 核心功能
- 功能点 1：多传感器数据采集，可实时获取温湿度、人体感应等环境参数
- 功能点 2：基于广和通 Cat.1 模组，实现终端与云平台双向蜂窝通信
- 功能点 3：支持本地逻辑判断，接收云端指令完成外设远程控制
- 功能点 4：云平台实现数据存储、可视化展示，数据超限自动告警
- 功能点 5：低功耗设计，适配室内环境监测、简易智能安防场景

## 🏗️ 系统架构

<img width="629" height="326" alt="图片" src="https://github.com/user-attachments/assets/0177027c-74ad-43c5-9e6a-f60503f91c31" />


## 📂 目录结构
```text
├── README.md               # 项目说明文件
├── docs/                   # 项目文档、设计报告、参考资料
├── hardware/               # 硬件设计资料
│   ├── pcb/                # PCB工程文件
│   ├── mechanical/         # 结构3D模型
│   └── bom.csv             # 物料清单
├── firmware/               # 设备底层固件源码
├── edge_computing/         # 边缘计算与智能算法
│   ├── algorithm/          # 业务算法代码
│   ├── ai_model/           # 模型训练与部署文件
│   └── requirements.txt    # 环境依赖
├── cloud/                  # 云端平台服务
│   ├── iot_platform/       # 设备接入、物模型配置
│   └── web_frontend/       # 数据可视化前端页面
└── tools/                  # 辅助工具与调试脚本
```

## 🚀 快速开始

```bash
git clone https://github.com/Fiborn/仓库名.git
cd 仓库名
# 具体启动命令
```
