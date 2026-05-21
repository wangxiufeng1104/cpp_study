# D-Bus 数据采集系统

演示 D-Bus 在实时数据采集场景中的完整应用：
一个服务端采集数据，两个客户端分别负责控制和数据接收。

## 架构

```
┌──────────────────────────────────────────────────────────────┐
│                   D-Bus Session Bus                          │
│                                                              │
│  ┌──────────────────┐  ┌──────────────┐  ┌──────────────┐    │
│  │ data_acq_server  │  │ data_acq_    │  │ data_acq_    │    │
│  │                  │  │ control      │  │ client       │    │
│  │ • 模拟传感器      │  │              │  │              │    │
│  │ • Start/Stop     │◄─┤ start/stop   │  │              │    │
│  │ • SetFrequency   │◄─┤ freq N       │  │              │    │
│  │                  │  │              │  │              │    │
│  │   DataReady ─────┼──┼──────────────┼─►│ 接收数据      │   │
│  │   (signal)       │  │              │  │ 速率统计      │   │
│  └──────────────────┘  └──────────────┘  └──────────────┘    │
└──────────────────────────────────────────────────────────────┘
```

## D-Bus 接口

**Service:** `com.example.DataAcquisition`
**Object:**  `/com/example/DataAcquisition`
**Interface:** `com.example.DataAcquisition.Control`

| 类型 | 名称 | 参数 | 说明 |
|------|------|------|------|
| Method | `Start` | — | 开始采集 |
| Method | `Stop` | — | 停止采集 |
| Method | `SetFrequency` | `u` freq (1-100) | 设置采样频率 (Hz) |
| Method | `GetStatus` | → `s` | 查询状态 ("running"/"stopped") |
| Signal | `DataReady` | `d` value, `t` timestamp | 实时数据推送 |

## 运行

```bash
# 1. 启动采集服务（终端 1）
./build/bin/data_acq_server

# 2. 启动控制端（终端 2）
./build/bin/data_acq_control
> start          # 开始采集
> freq 20        # 设置 20Hz 采样
> status         # 查看状态
> stop           # 停止采集
> quit

# 3. 启动数据接收端（终端 3）
./build/bin/data_acq_client
```

## 输出示例

**Server:**
```
[server] Data Acquisition Service online
[server]   Service: com.example.DataAcquisition
[server]   Default freq: 10 Hz
[server] Acquisition STARTED (freq=10 Hz)
[server] DataReady: value=52.134, ts=1716285600123456
[server] Frequency set to 20 Hz
[server] Acquisition STOPPED
```

**Control:**
```
[control] Data Acquisition Controller
> start
[control] Start() OK
> freq 20
[control] SetFrequency(20) OK
> status
[control] Status: running
> stop
[control] Stop() OK
```

**Data Client:**
```
[client] Listening for DataReady signals...
[client] #10  val=      52.134  ts=1716285600123456
[client] #20  val=      73.241  ts=1716285600623456
[client] ── rate: 10 samples/sec | total=20
[client] ── rate: 20 samples/sec | total=40
```

## 核心技术点

- **Method Call**: 控制端通过 `Start/Stop/SetFrequency` 同步调用控制采集器状态
- **Signal**: 服务端通过 `DataReady` 信号广播实时数据，数据端通过 match rule 订阅
- **事件循环**: 服务端用 20ms 轮询 + `std::chrono` 计时实现精确采样间隔控制
- **速率验证**: 数据客户端每秒统计接收数据点数，可验证采样频率是否符合设定
