# Virtual Signal Testing — 用"假发射器"先把整个 pipeline 跑通

> 适用场景：队友的红外发射器还没到位，或者你不想被"信号收不到"这个变量耽误，
> 想**先验证"信号 → 状态机 → 执行器"这条链路本身**是不是工作的。
>
> 这套方法在工业界叫 **stub / mock testing**：把不确定的硬件输入用一个
> 你完全可控的输入源（这里是电脑键盘）替代，先把后面的逻辑全部锁死。
> 等真信号到了，只换一行代码就能切换回去。

## 1. 为什么要做"虚拟信号"？

调试时你会遇到这种连环问题：

> "按按键没反应 —— 是遥控器没电？是接收头反了？是 IR 协议不对？是 IR_KEY 码填错？是 switch 写错？是 LED 接反？是 LCD 没初始化？……"

变量太多就排查不出来。**先把"输入端"用一个绝对可靠的东西（键盘 + 串口）替换掉**，剩下问题就只可能在状态机和外设侧，定位速度提高 5-10 倍。

```
真实链路：  [遥控器] -> [IR 头] -> [Arduino 解码] -> [状态机] -> [LED/风扇/...]
                                                      ▲
                                                      │ 都用同一个入口
                                                      │
虚拟链路：  [电脑键盘] -> [串口] -> [Arduino 读字符] -> [状态机] -> [LED/风扇/...]
```

只要状态机入口是同一个，**虚拟链路通了 = 真实链路一定也通**（只剩"红外解码到状态机"这一小段需要单独验）。

## 2. 三种使用方式（任选其一）

### 方式 A — 主程序自带"虚拟魔杖"通道（推荐）

`VirtualTown_Controller.ino` 已经内置了串口虚拟魔杖。**红外和键盘是并存的**：

- 没接 IR 接收器、或者还没填真信号码 → 直接键盘输入触发。
- 队友信号码到位、IR 接好后 → 键盘仍然可用，演示当天可作 backup。

**操作步骤**：

1. 打开 Arduino IDE → 上传 `VirtualTown_Controller.ino`（IR_KEY_* 暂时不动也行）。
2. 打开 **Serial Monitor**，右下角设置：
   - 波特率：**115200**
   - 行尾：**Newline** （或 *Both NL & CR*，**不要选 No line ending**）
3. 在输入框输入下面任意字符 + 回车：

   | 输入 | 触发的状态 | 等价于真按键 |
   |---|---|---|
   | `1` | Morning  | 按键 1 |
   | `2` | Nature   | 按键 2 |
   | `3` | Festival | 按键 3 |
   | `0` | Night    | 按键 0 |
   | `p` 或 `P` | Night | POWER |

4. 串口会回显 `[VirtualWand] -> 1` 之类，并继续按真实状态执行：LCD 切文字、LED 亮、蜂鸣器响、（如已接好）风扇/水泵动。

> **小心**：有的 IDE 默认行尾是 *No line ending*，那串口会一直收不到字符。
> 如果按上面操作没反应，第一件事就是**改右下角行尾设置**。

### 方式 B — 纯虚拟测试 sketch（**最安全**，可以一个外设都不接）

`tests/12_virtual_signal_pipeline/12_virtual_signal_pipeline.ino`

特点：
- **完全不需要红外硬件**。
- **完全不会驱动电机**（只把 D5/D6 当数字输出 toggle 一下，可以用万用表验证）。
- LCD 没接也不会卡死（自动跳过）。

**操作步骤**：
1. 上传 `12_virtual_signal_pipeline.ino`。
2. 串口 115200 + Newline。
3. 输入 `h` 看帮助、`1/2/3/0/p` 切状态、`r` 报告当前状态。
4. 验证四个状态的现象（LED、蜂鸣器、LCD 文字）都正确。

**这一步通过 = 你的状态机、LCD、LED、蜂鸣器、非阻塞旋律全部正确**，
后面只需要把"键盘字符 → enterXxx()" 换成"红外码 → enterXxx()"。

### 方式 C — 进阶：用电脑脚本批量灌信号（自动化回归测试）

等你想做"脚本化测试"时，可以在电脑端用 Python 往串口写字符，模拟一连串
信号自动跑：

```python
# pip install pyserial
import serial, time
s = serial.Serial('COM5', 115200, timeout=1)   # Windows，Linux 用 /dev/ttyACM0
time.sleep(2)
for cmd in ['1', '2', '3', '0', 'p']:
    s.write((cmd + '\n').encode())
    time.sleep(2)
    print(s.read_all().decode(errors='ignore'))
s.close()
```

这就是真正的 **CI/regression test**：每次改代码都跑一遍这个脚本，
能自动验证全部状态切换。

## 3. 拿到队友真信号码后，怎么"切换回真实链路"？

只改一处：`VirtualTown_Controller.ino` 顶部 5 个常量。

```cpp
constexpr uint32_t IR_KEY_1     = 0x........;   // ← 队友信号 1
constexpr uint32_t IR_KEY_2     = 0x........;   // ← 队友信号 2
constexpr uint32_t IR_KEY_3     = 0x........;   // ← 队友信号 3
constexpr uint32_t IR_KEY_0     = 0x........;   // ← 队友信号 0
constexpr uint32_t IR_KEY_POWER = 0x........;   // ← 队友信号 POWER
```

获取这些码的方法：
- **如果队友也用 Arduino + IRremote 做发射器**：让他/她直接告诉你他发出去的
  `decodedRawData` 是多少，原样填进来，**两边字段必须是同一个**才能匹配上。
- **如果队友用市售遥控器**：你这边接好 IR 接收器，烧 `tests/05_ir_dump`，
  让他按 5 个按键，你抄串口打印的 `raw=0x......`。

填完后**重新 Upload 即可**。键盘虚拟通道仍保留，做 backup 用。

## 4. "虚拟 → 真实" 渐进集成 checklist

**严格按这个顺序做**，每一步通过再走下一步：

- [ ] **L0 (纯软件)**：烧 `12_virtual_signal_pipeline`，**只接 USB**，串口键盘
      切 4 个状态，看到 `[STATE] ...` 打印 → 状态机本身 OK。
- [ ] **L1 (+LED+蜂鸣器)**：仍用 `12_virtual_signal_pipeline`，加上 LED(D7)
      和无源蜂鸣器(D8)，键盘切状态时 LED 和蜂鸣器有反应 → 弱电外设 OK。
- [ ] **L2 (+LCD)**：再加上 1602 I2C LCD，键盘切状态时 LCD 文字正确切换
      → I2C / 显示通道 OK。
- [ ] **L3 (主程序+键盘)**：换成 `VirtualTown_Controller.ino`，**还不接 IR**，
      用串口键盘 1/2/3/0/p 触发，所有现象一致 → 主程序状态机 OK。
- [ ] **L4 (+真信号)**：接好 IR 头，烧 `tests/05_ir_dump` 抄真信号码，填回
      `IR_KEY_*`，重新烧主程序，**真实遥控按键** = 键盘相同字符的现象
      → 输入通道 OK。
- [ ] **L5 (+电机)**：按 `HARDWARE_GUIDE.md` 接 MOSFET + 风扇 + 水泵 + 外部
      电源 + 共地，再次跑全流程 → 强电通道 OK。

每一关失败时，**只可能是新加进来的那一层**有问题，定位非常快。

## 5. 常见问题

| 现象 | 原因 | 解决 |
|---|---|---|
| 串口输入字符没反应 | 行尾选了 "No line ending" | 改成 "Newline" |
| 输入 `1` 串口没回显 | Arduino IDE 串口监视器默认本地不回显，只看 Arduino 打印的 `[VirtualWand] -> 1` 即可 | 正常 |
| 主程序串口里既看到 `[IR]` 又看到 `[VirtualWand]` | 两条通道并存 = 设计如此 | 正常 |
| 我想关掉键盘虚拟通道 | 在 `loop()` 里把 `while (Serial.available()...)` 那段注释掉 | 主程序顶部已经有清晰注释 |
| Python 脚本写串口后 Arduino 没反应 | 打开串口会让 Mega 复位 → 等 2 秒 (`time.sleep(2)`) 再写 | 上面例子已加 |
