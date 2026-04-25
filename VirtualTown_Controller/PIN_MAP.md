# VirtualTown — 元件 ↔ 引脚 ↔ 触发条件 速查表

> 配套硬件：MEGA2560 Sensor Shield（每个数字脚都拆成 **G / V / S** 三针）。
> 配套程序：`VirtualTown_Controller.ino`。
>
> 4 个状态由这 5 个按键 / 字符触发：
>
> | 触发 | 状态 |
> |---|---|
> | 按键 `1`（或键盘 `1`） | **Morning** |
> | 按键 `2`（或键盘 `2`） | **Nature** |
> | 按键 `3`（或键盘 `3`） | **Festival** |
> | 按键 `0` 或 `POWER`（或键盘 `0` / `p`） | **Night** |

---

## 第 1 份：弱电模块（USB 供电就够，今天就接）

| 元件 | Sensor Shield 上的 3-pin 组 | 元件引脚 → 接到 | 哪个状态下"响应" | 注意事项 |
|---|---|---|---|---|
| **IR 接收头 (VS1838B)** | **D11 那一组** | OUT→**S**, GND→**G**, VCC→**V** | 一直在工作（接收任意按键并打印 `[IR] code = 0x...`） | 引脚顺序看模块小板上的丝印 (`OUT/G/V` 或 `S/-/+`)，不要按位置死记。接反了不会烧，但收不到信号。 |
| **城镇 LED** | **D7 那一组** | LED **长脚 (+)** → 220Ω → **S** ；LED **短脚 (–)** → **G** ；**V 不接** | Morning / Nature / Festival 时 **亮**；Night 时 **灭** | **必须串 220Ω**，否则烧 LED 和板子。LED 是有方向的，反了不亮。 |
| **无源蜂鸣器 / 8Ω 喇叭** | **D8 那一组** | `+` (长脚) → **S** ；`–` (短脚) → **G** ；**V 不接** | Morning：250 ms 高频"叮"<br>Festival：循环旋律<br>Nature / Night：静音 | 必须用**无源**蜂鸣器；有源的只会一直"嗡"一声不变调。喇叭功率 ≤ 0.5 W 时可以直插，更大的串 100Ω 限流。 |
| **LCD 1602 I2C** | **右上角 4-pin 区** `G V SDA SCL` | LCD `GND` → **G** ；`VCC` → **V** ；`SDA` → **SDA** ；`SCL` → **SCL** | 每次状态切换都会自动刷新两行文字 | I2C 地址通常 **0x27**；如果一片漆黑/纯白，先调 LCD 背面**蓝色电位器**（对比度），再去怀疑地址，最后才怀疑接线。 |

> ✅ 这 4 个东西全部接好后，主程序的 4 个状态里**除了风扇和水泵**之外的所有现象都能验证。完全可以先跑这一步。

---

## 第 2 份：强电模块（必须 MOSFET + 外部电源，按图来）

> ⚠ **绝对不要把风扇/水泵直接插 Sensor Shield 的 V 针**！Sensor Shield 的 V 是 Arduino 自己的 5V (≤500 mA)，电机启动会让 Mega 复位甚至烧 USB 口。

### 2.1 风扇 (R130)

| 项目 | 接到哪里 | 备注 |
|---|---|---|
| 风扇 红线 (+) | **外部 5V 电源 (+)** | **不是** Sensor Shield 的 V |
| 风扇 黑线 (–) | **MOSFET 的 D 脚** (Drain) | |
| 1N4007 二极管 | 跨在风扇红线 ↔ 黑线之间，**带白环的一端朝红线** | 续流保护，必须装 |
| MOSFET (IRLZ44N) **G** 脚 | **Sensor Shield D5 组的 S** | 这就是控制信号 |
| MOSFET (IRLZ44N) **G** 脚 | 同时再接一个 **10kΩ 电阻到 GND**（接 Sensor Shield D5 组的 G 即可） | 下拉电阻，防止上电瞬间风扇乱转 |
| MOSFET (IRLZ44N) **S** 脚 | **共地点**（Sensor Shield 任意 G 针 **和** 外部电源 GND 用一根线短接到一起） | **共地是必须的！** |
| Sensor Shield D5 组的 **V** | **不接** | |

**响应规则**：

| 状态 | D5 输出 (PWM) | 风扇 |
|---|---|---|
| Morning | 0 | 不转 |
| Nature | 160 (≈63% 占空比) | 中速 |
| Festival | 160 | 中速 |
| Night | 0 | 不转 |

### 2.2 水泵 (JT-DC3W 小型潜水泵)

电路结构和风扇**完全一样**，只把信号引脚从 D5 换成 **D6**，配套用第二个 MOSFET、第二个二极管、第二个 10kΩ：

| 项目 | 接到哪里 |
|---|---|
| 水泵 红线 (+) | **外部 5V 电源 (+)** （和风扇可以共用同一组电源，只要电源能给到 ≥1.5 A） |
| 水泵 黑线 (–) | 第 2 个 MOSFET 的 **D** 脚 |
| 1N4007 | 跨在水泵两端，带环朝 (+) |
| 第 2 个 MOSFET **G** | **Sensor Shield D6 组的 S** |
| 第 2 个 MOSFET **G** | 串 10kΩ 到 GND (D6 组的 G) |
| 第 2 个 MOSFET **S** | 共地点 |
| Sensor Shield D6 组的 **V** | **不接** |

**响应规则**：

| 状态 | D6 输出 | 水泵 |
|---|---|---|
| Morning | LOW | 关 |
| Nature | HIGH | 开 |
| Festival | HIGH | 开 |
| Night | LOW | 关 |

> ⚠ **水泵第一次通电前必须先泡进水里**！干转 30 秒以上会烧绕组。

---

## 一张总表（贴在你桌上）

| Sensor Shield 引脚 | 谁用 | 信号方向 | 在哪几个状态会"动" |
|---|---|---|---|
| **D5** | 风扇（经 MOSFET） | OUT (PWM) | Nature ✓ / Festival ✓ |
| **D6** | 水泵（经 MOSFET） | OUT (HIGH=开) | Nature ✓ / Festival ✓ |
| **D7** | 城镇 LED（串 220Ω） | OUT (HIGH=亮) | Morning ✓ / Nature ✓ / Festival ✓ |
| **D8** | 无源蜂鸣器 / 喇叭 | OUT (`tone()`) | Morning（叮）/ Festival（旋律） |
| **D11** | IR 接收头 | IN | 任何时候，按键时收到信号 |
| **SDA (20)** | LCD 数据线 | I2C | 每次状态切换 |
| **SCL (21)** | LCD 时钟线 | I2C | 每次状态切换 |
| **5V (顶上 V)** | LCD / IR / 蜂鸣器供电 | — | — |
| **GND (顶上 G)** | 公共地 + 必须和外部电源 GND 短接 | — | — |

---

## 推荐接线顺序（拿这份当 checklist 一条一条勾）

按这个顺序做，**每接完一项立刻跑对应测试**，错了好排查：

- [ ] **第 1 步**：插 USB，烧 `tests/12_virtual_signal_pipeline`，键盘 1/2/3/0 看串口能切状态。
- [ ] **第 2 步**：接 LCD 到 SDA/SCL → 烧 `tests/01_i2c_scanner` 确认地址 → 烧 `tests/02_lcd_hello` 看到字 → 再烧回 `12_virtual_signal_pipeline`，键盘切状态时 LCD 应该跟着变。
- [ ] **第 3 步**：接 LED 到 D7（串 220Ω）→ 烧 `tests/03_led_blink` 闪一下 → 再回 `12_virtual_signal_pipeline`，键盘 1/2/3 时 LED 亮、键盘 0 时 LED 灭。
- [ ] **第 4 步**：接蜂鸣器到 D8 → 烧 `tests/04_buzzer_tone` 听到 do-re-mi → 再回 `12_virtual_signal_pipeline`，键盘 1=叮一声、3=循环旋律。
- [ ] **第 5 步**：接 IR 接收头到 D11 → 烧 `tests/05_ir_dump` 抄下你的 5 个真按键 raw 码。
- [ ] **第 6 步**：把抄到的 5 个码填进 `VirtualTown_Controller.ino` 的 `IR_KEY_*`，烧主程序，**真遥控按键**应当和键盘同字符表现完全一致。
- [ ] **第 7 步**（强电）：按 2.1 节接好风扇 + MOSFET + 1N4007 + 10kΩ + 外部电源 + **共地** → **先不通外部电源**，烧 `tests/06_motor_pwm`，万用表测 MOSFET G 脚电压有跳变 → 再通外部电源，风扇按 OFF→慢→中→快 循环。
- [ ] **第 8 步**：按 2.2 节同样的方法接水泵到 D6（**水泵泡水里**）→ 烧 `tests/07_pump_test`，水泵 3 秒开 3 秒关。
- [ ] **第 9 步**：再次烧 `VirtualTown_Controller.ino`，全套验收。

---

## 几条最容易踩的坑

| 坑 | 现象 | 正解 |
|---|---|---|
| 把电机接到 Sensor Shield 的 V | Mega 一通电就重启 / USB 烧坏 | 必须外部 5V 电源 |
| 忘了共地 | MOSFET 不导通，电机不转 | Arduino GND ↔ 外部电源 GND 必须用一根线短接 |
| MOSFET 栅极没下拉 | 上电瞬间风扇/水泵乱转 | G 脚一定要串 10kΩ 到 GND |
| 没装续流二极管 | 跑几次后 MOSFET 突然烧穿 | 1N4007 必须装，**带环的一端朝 +5V** |
| LED 没串电阻 | LED 烧 / Arduino 引脚也可能受损 | 220Ω 必须串 |
| 用了有源蜂鸣器 | 一直"嗡"一声、Festival 旋律不变调 | 换无源蜂鸣器 |
| LCD 选错地址 | 屏幕完全没字（背光可能亮） | 跑 `01_i2c_scanner` 看到底是 0x27 还是 0x3F |
| 串口键盘按 1 没反应 | 串口监视器右下角行尾选了 "No line ending" | 改成 **Newline** |
