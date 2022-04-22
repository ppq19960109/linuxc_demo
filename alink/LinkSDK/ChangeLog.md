# 文件说明

记录`Link SDK-4.x`的更新历史

# 更新内容

+ 2020-05-06: SDK 4.0.0版本正式发布
+ 2021-04-20: SDK 4.1.0版本正式发布
  +  新增安全隧道和远程调试功能
  +  新增AT模组驱动框架,支持模组快速适配
  +  mbedtls安全层抽象
  +  支持单报文多url的OTA
  +  新增基于mqtt的动态注册功能
  +  支持MQTT 5.0协议

# 模块状态


| 模块名                                      | 更新时间    | Commit ID
|---------------------------------------------|-------------|---------------------------------------------
| 核心模块(core)                              | 2021-10-28  | 2e0e64173ab049dd9979395dc95abf6a85385ae2
| (components/mqtt_upload)                    | 2022-03-15  | 0947d5ecbe5d8fccbdc22374f24cfe44abd58aae
| 远程隧道模块(components/remote-access)      | zhijian     | cc16a128987d983a21ac1c11ffd9a88b905e764f
| 子设备管理模块(components/subdev)           | 2022-03-24  | b546d3dfa57c2ea9fc32f08ef99f1e7238c05b4b
| 基于MQTT的动态注册(components/dynreg-mqtt)  | 2021-10-14  | 3ec260703f401c925fbef6b6905de5eac4da4663
| 物模型模块(components/data-model)           | 2021-09-06  | 1d9270de816f7ff0f60c0b2a53d08ca4da8bab66
| 时间同步模块(components/ntp)                | 2021-09-16  | cb96f929c231ad8ee8c48dcf82167f3f6eb66dad
| 设备诊断模块(components/diag)               | 2021-10-18  | 18897e1421952e4eda11e82a61f573654f2bcc69
| 设备影子模块(components/shadow)             | 2021-09-22  | 8e36b636c72b38817382a5ca6f4ea80483b398b6
| 设备日志模块(components/logpost)            | 2021-09-15  | d0e41935909d0c7f593f9225e119f7698db67b2d
| 固件升级模块(components/ota)                | 2021-11-10  | 13f8cc22568375d132be9c3e1ad26a57f199f294
| 任务管理模块(components/task)               | 2021-10-14  | 1fe061f31ad0e6b1616472335cad7e2f67761915
| 动态注册(components/dynreg)                 | 2020-07-30  | 190e3bed4080eeb07c4f9e907cb7c3d966dfab53
| 设备标签模块(components/devinfo)            | 2021-09-15  | 9fe181e1c6e537410a7fe843db5c4af782f8061a
| 引导服务模块(components/bootstrap)          | 2020-07-30  | 566135cde8a63c2b5944877ea8c8189c0712b4f7



