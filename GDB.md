# GDB 使用笔记

> GDB（GNU Debugger）是 Linux 下最常用的命令行源代码级调试器。它让你在程序运行时观察内部状态、确认事实、定位根因，而不是只靠 `printf` 和猜测。

## 目录

- [1. GDB 是什么](#1-gdb-是什么)
- [2. 什么时候需要 GDB](#2-什么时候需要-gdb)
- [3. GDB 的四大核心能力](#3-gdb-的四大核心能力)
- [4. 准备工作：安装与带调试信息编译](#4-准备工作安装与带调试信息编译)
- [5. 常用命令速查](#5-常用命令速查)
- [6. 常见问题与排错](#6-常见问题与排错)
- [7. 总结](#7-总结)

## 1. GDB 是什么

GDB 是 Linux 环境下最核心的命令行源代码级调试工具，主要面向 C/C++，也支持 Go、Rust、Fortran 等编译型语言；GDB 本身可以用 Python 脚本扩展。

它的核心价值在于：让程序在运行时“停下来”，观察内部状态，从而定位逻辑错误、内存问题和崩溃原因。和 `printf` 相比，GDB 不需要修改代码、重新编译，能直接查看调用栈、变量、内存和线程状态。

> 调试的本质是“提出假设，再用运行时事实验证或推翻”。GDB 提供的，就是获取事实的手段。

## 2. 什么时候需要 GDB

如果你已经明确知道错在哪一行、为什么错、怎么改，而且改完验证通过，那就不需要 GDB。GDB 不是“必须走的流程”，它只是调试工具之一。

但“知道错在哪里”和“知道运行时为什么错”经常不是一回事。以下场景值得使用 GDB：

| 场景 | GDB 能做什么 |
| --- | --- |
| 想确认判断，而不是猜 | 查看调用栈、实参、局部变量，用事实确认 |
| 崩溃位置不是根因 | 崩溃在 `strcpy`，根因可能是更早的越界写；`bt`、watchpoint、内存检查能找出“谁把它弄坏的” |
| 问题不能稳定复现 | attach 到进程，查看线程栈、锁状态、变量值；结合 core dump 事后分析 |
| 多线程/并发问题 | 查看所有线程栈，配合条件断点观察交错执行，排查竞态、死锁、数据竞争 |
| 优化后的代码和源码对不上 | `-O2` 后变量被优化掉、行号跳转、函数内联；通过汇编、寄存器、实际执行路径判断 |
| 需要条件断点/数据断点 | 只在 `i == 999` 时停，或监控“谁改了 `g_state`” |
| 调用链复杂 | 用一个错误返回值反推执行路径，`bt` 加单步快速还原 |
| 验证修复 | 断点处检查变量、函数返回值、线程状态是否符合预期 |
| 不能改代码/重新编译 | 线上进程、嵌入式、客户环境；加日志还可能改变时序，干扰并发 bug |
* 多线程：多线程就是一个进程内有多个执行流，它们共享进程资源，但各自有独立栈和寄存器；能提高并发和利用多核，但会引入竞态、死锁和调试复杂性。

一句话：**GDB 不是用来“找你知道的错”，而是用来确认运行时事实、寻找根因、处理难复现/并发/内存/优化/线上问题，以及验证修复。** 错误简单明确时直接改；当“读代码 + printf + 猜测”不够可靠时，再上 GDB。

## 3. GDB 的四大核心能力

| 能力 | 说明 |
| --- | --- |
| 控制执行 | 让程序在关心的地方停下来，而不是一口气跑完 |
| 单步追踪 | 逐行执行，或步入函数内部，精确观察每一步的影响 |
| 状态检查 | 查看变量、内存、寄存器、函数调用栈 |
| 动态修改 | 运行时改变量值、强制函数返回、调用函数，以验证不同条件 |

调试的基本循环就是：**停 → 看 → 改 → 继续**。

## 4. 准备工作：安装与带调试信息编译

### 4.1 安装 GDB

```bash
# Debian / Ubuntu
sudo apt update && sudo apt install gdb

# Fedora / RHEL / CentOS
sudo dnf install gdb
```

### 4.2 编译时必须加 `-g`

GDB 需要可执行文件中包含调试信息（行号、变量名、类型、源码路径）。没有这些信息，GDB 只能看到汇编，无法按源码和变量调试。

Linux 下 `gcc/g++` **默认既不生成调试信息，也不开启优化**（等价于 `-O0`），它并不是严格意义上的 release 构建；release 通常还会加 `-O2/-O3`、`-DNDEBUG` 等。

```bash
gcc -g -O0 -Wall -Wextra -o sum_to sum_to.c
```

| 选项 | 作用 |
| --- | --- |
| `-g` | 生成 DWARF 调试信息：行号、变量名、类型、源码路径 |
| `-O0` | 关闭优化，调试首选；变量不会被优化掉或重排 |
| `-Wall -Wextra` | 打开常用警告，尽早暴露问题 |

### 4.3 示例程序

保存为 `sum_to.c`：

```c
#include <stdio.h>
int sum_to(int n)
{
    int s = 0;
    for (int i = 0; i <= n; i++)
    {
        s += i;
    }
    return s;
}
int main(void)
{
    printf("sum_to(3)=%d\n", sum_to(3));
    return 0;
}
```

```bash
gcc -g -O0 -Wall -Wextra -o sum_to sum_to.c
./sum_to
# sum_to(3)=6
```

## 5. 常用命令速查

> 说明：缩写栏为 `—` 表示没有常用缩写。GDB 支持命令缩写，只要不产生歧义即可，如 `b`、`n`、`s`、`c`、`p`、`bt`、`q`。

### 5.1 启动、运行与退出

| 命令 | 缩写 | 说明 |
| --- | --- | --- |
| `gdb <可执行文件>` | — | 启动 GDB 并加载程序 |
| `gdb <程序> <core>` | — | 加载程序与 core 文件，用于事后分析崩溃 |
| `gdb -p <PID>` | — | 附加到正在运行的进程 |
| `run [参数...]` | `r` | 开始运行程序，参数传给被调试程序 |
| `start` | — | 运行并停在 `main` 的第一条语句 |
| `starti` | — | 停在程序的第一条机器指令（常用于无符号/汇编调试） |
| `set args <参数...>` | — | 设置程序参数；`show args` 查看当前参数 |
| `attach <PID>` | — | 附加到进程（等价于 `gdb -p`） |
| `detach` | — | 与进程分离，让它继续运行 |
| `kill` | `k` | 终止被调试程序，但保留 GDB 会话 |
| `quit` | `q` | 退出 GDB（若程序仍在运行会询问是否终止） |

> 程序运行过程中按 `Ctrl-C` 可以中断程序，回到 `(gdb)` 提示符。

### 5.2 断点与观察点

| 命令 | 缩写 | 说明 |
| --- | --- | --- |
| `break <位置>` | `b` | 下断点；位置可以是函数名（`b sum_to`）、行号（`b 7`）、文件:行号（`b sum_to.c:7`） |
| `break <位置> if <条件>` | `b` | 条件断点，如 `b sum_to.c:7 if i == 2` |
| `tbreak <位置>` | `tb` | 临时断点，命中一次后自动删除 |
| `rbreak <正则>` | — | 对匹配正则的函数批量下断点 |
| `condition <编号> <条件>` | — | 给已有断点添加/修改条件，如 `condition 1 i == 999` |
| `ignore <编号> <次数>` | — | 忽略前 N 次命中，如 `ignore 1 999` |
| `info breakpoints` | `i b` | 查看所有断点/观察点、启用状态和命中次数 |
| `disable <编号...>` | `dis` | 禁用断点（保留定义） |
| `enable <编号...>` | `en` | 重新启用断点 |
| `delete [编号...]` | `d` | 删除断点；不带编号删除全部（会确认） |
| `clear <位置>` | — | 删除指定位置的断点 |
| `watch <表达式>` | — | 观察点：表达式的值变化时停下，如 `watch s`、`watch g_state` |
| `rwatch <表达式>` | — | 读观察点：被读取时停下 |
| `awatch <表达式>` | — | 读写观察点：被读或写时停下 |
| `info watchpoints` | — | 查看观察点 |

> **提示**：watchpoint 依赖硬件/内存监视机制，数量有限；局部变量离开作用域后，对应的 watchpoint 会自动删除。

### 5.3 控制执行

| 命令 | 缩写 | 说明 |
| --- | --- | --- |
| `continue` | `c` | 继续运行，直到下一个断点或程序结束 |
| `next` | `n` | 单步执行下一行；遇到函数调用时**步过**，不进入函数 |
| `step` | `s` | 单步执行下一行；遇到函数调用时**步入**函数 |
| `nexti` | `ni` | 单步执行一条机器指令（汇编级步过） |
| `stepi` | `si` | 单步执行一条机器指令（汇编级步入） |
| `until [位置]` | `u` | 运行到指定位置；不带参数时运行到当前行之后的一行，常用来快速跳过循环 |
| `finish` | `fin` | 运行到当前函数返回，并打印返回值 |
| `return [值]` | — | 强制当前函数立即返回，可指定返回值 |
| `jump <位置>` | `j` | 跳到指定行继续执行（不改变栈帧，慎用） |
| `signal <信号>` | — | 向程序发送信号，如 `signal SIGINT` |
| `Ctrl-C` | — | 中断正在运行的程序 |
| `kill` | `k` | 终止程序 |

### 5.4 查看源码、栈和类型

| 命令 | 缩写 | 说明 |
| --- | --- | --- |
| `list [位置]` | `l` | 显示源码，默认每次显示 10 行（围绕当前行）；`l -` 回看，`l sum_to` 查看函数 |
| `backtrace [n]` | `bt` | 打印调用栈；`bt full` 显示各帧局部变量，`bt 5` 只显示 5 层 |
| `frame <n>` | `f` | 切换到第 n 层栈帧，之后用 `p` 查看该帧变量 |
| `up [n]` / `down [n]` | — | 在调用栈中上/下移动 n 层 |
| `info frame` | `i f` | 当前栈帧的详细信息 |
| `info args` | `i args` | 当前函数的实参 |
| `info locals` | `i locals` | 当前函数的局部变量 |
| `info registers` | `i r` | 所有寄存器；`i r rip rsp rbp` 查看指定寄存器 |
| `disassemble [/m] [函数]` | `disas` | 反汇编；`/m` 同时显示源码 |
| `whatis <表达式>` | — | 查看表达式的类型名 |
| `ptype <类型/变量>` | `pt` | 查看类型定义（结构体成员、typedef 等） |
| `info functions [正则]` | — | 列出函数 |
| `info variables [正则]` | — | 列出全局/静态变量 |

> `where` 是 `backtrace` 的别名，作用和 `bt` 相同。

### 5.5 查看与修改数据

| 命令 | 缩写 | 说明 |
| --- | --- | --- |
| `print <表达式>` | `p` | 打印变量/表达式，如 `p sum`、`p *ptr`、`p &var`、`p arr[2]@3` |
| `print/<格式> <表达式>` | — | 指定格式：`p/x` 十六进制、`p/d` 十进制、`p/c` 字符、`p/s` 字符串、`p/t` 二进制 |
| `print <函数>(<参数>)` | — | 调用函数并打印返回值，如 `p sum_to(5)`（可能改变程序状态） |
| `x/<数量><格式><单位> <地址>` | — | 查看内存，如 `x/4xb &s`、`x/8dw arr`、`x/s str`、`x/10i $rip` |
| `display <表达式>` | — | 每次程序停下自动打印；`display/i $pc` 显示当前指令 |
| `undisplay <编号>` | — | 取消自动显示 |
| `info display` | — | 查看所有 display 项 |
| `set var <变量> = <值>` | — | 运行时改变量，如 `set var i = 0` |
| `set {<类型>}<地址> = <值>` | — | 直接写内存，如 `set {int}0x7fffffffe3a0 = 42`（确认地址合法） |
| `call <函数>(<参数>)` | — | 调用函数，如 `call sum_to(5)` |
| `set $<名字> = <值>` | — | 定义 GDB 便利变量（不写入被调试程序） |
| `show convenience` | — | 查看便利变量 |

`x` 命令的格式字母：`x` 十六进制、`d` 十进制、`u` 无符号、`o` 八进制、`t` 二进制、`a` 地址、`c` 字符、`s` 字符串、`i` 指令；单位：`b` 字节、`h` 半字（2 字节）、`w` 字（4 字节）、`g` 8 字节。例如 `x/16xb arr` 表示从 `arr` 开始查看 16 个字节。

`p arr[2]@3` 中的 `@` 表示把从 `arr[2]` 开始的连续 3 个元素当成数组显示。

### 5.6 线程与进程

| 命令 | 缩写 | 说明 |
| --- | --- | --- |
| `info threads` | `i threads` | 列出所有线程，`*` 表示当前线程 |
| `thread <id>` | — | 切换到指定线程 |
| `thread apply all bt` | — | 对所有线程打印调用栈（排查死锁/卡死常用） |
| `thread apply all bt full` | — | 带局部变量的完整栈 |
| `set scheduler-locking on/off` | — | 调试时锁定/放开线程调度 |
| `info proc mappings` | — | 查看进程内存映射 |
| `gcore <文件>` | — | 把运行中的进程转储为 core 文件，不终止进程 |

### 5.7 其他实用命令

| 命令 | 缩写 | 说明 |
| --- | --- | --- |
| `set pagination off` | — | 关闭分页，适合脚本或记录输出 |
| `set logging enabled on` | — | 把 GDB 输出写入 `gdb.txt`；`set logging file <文件>` 指定文件（旧版本用 `set logging on`） |
| `directory <路径>` | `dir` | 添加源码搜索路径 |
| `set substitute-path <旧> <新>` | — | 源码路径重映射（编译机和调试机路径不同） |
| `handle <信号> <动作>` | — | 设置信号处理，如 `handle SIGPIPE nostop noprint pass` |
| `catch <事件>` | — | 捕获事件：`catch syscall`、`catch throw`、`catch fork` |
| `layout <src/asm/split/regs>` | — | TUI 布局；`Ctrl-x a` 开关 TUI |
| `tui reg general` | — | TUI 中显示通用寄存器 |
| `set disassembly-flavor intel` | — | 反汇编使用 Intel 语法（默认 AT&T） |
| `record` / `reverse-continue` | `rc` | 记录执行并反向调试（需要平台支持） |
| `help <命令>` | `h` | 查看内置帮助 |


## 6. 常见问题与排错

| 现象 | 原因 | 解决 |
| --- | --- | --- |
| `No debugging symbols found in ./prog` | 编译时没加 `-g` | 用 `gcc -g -O0` 重新编译 |
| `No symbol table is loaded. Use the "file" command.` | 没有加载带符号的程序，或在调试纯汇编 | 用 `file ./prog` 加载；确认程序带 `-g` |
| 变量显示 `<optimized out>` | 编译优化把变量优化掉了 | 用 `-O0` 或 `-Og` 重新编译；必须调试优化版时看汇编/寄存器 |
| 源码行号错位、找不到源文件 | 编译路径和当前路径不同，或源码被改动 | `directory <路径>`、`set substitute-path`；用 `info source` 查看编译目录，必要时重新编译 |
| `ptrace: Operation not permitted` | 权限不足或 `ptrace_scope` 限制 | 用同一用户运行/附加，或临时 `sudo sysctl -w kernel.yama.ptrace_scope=0`（注意安全影响） |
| 程序崩溃但没有 core 文件 | `ulimit -c 0`，或 `core_pattern` 交给别的程序处理 | 当前 shell 执行 `ulimit -c unlimited`；查看 `cat /proc/sys/kernel/core_pattern`；也可用独立的 `gcore <PID>` 命令主动转储 |
| `Cannot access memory at address ...` | 指针为空、悬空或越界 | 先 `p ptr`、`info proc mappings` 确认地址范围 |
| `Program received signal SIGSEGV` | 段错误（不是 GDB 报错，而是程序崩溃） | `bt` 看崩溃位置，`frame N` 切栈帧，`info locals`、`p`、`x` 检查非法访问 |
| 启动时提示 debuginfod 下载调试信息 | 系统 GDB 默认启用 debuginfod | 在 `~/.gdbinit` 中加入 `set debuginfod enabled off` |
| Fedora/CentOS 提示 `Missing separate debuginfos` | 缺少系统库的调试符号包 | 安装对应的 `debuginfo-install` 包；调试自己的代码可以先忽略 |


## 7. 总结

GDB 的使用可以浓缩为一个循环：

1. 用 `gcc -g -O0` 编译，确保有调试信息。
2. 在可疑位置下断点：`b 函数`、`b 文件:行号`、条件断点。
3. `run` 复现问题。
4. 停下来后看现场：`bt`、`info args`、`info locals`、`p`、`x`、`display`。
5. 判断是“值不对”还是“流程不对”，必要时用 `watch`、单步、切换栈帧。
6. 修改后重新编译，再用断点或输出验证。

如果只记一组最小命令集：

```text
gcc -g -O0 -o prog prog.c
gdb ./prog
(gdb) b 函数名或行号
(gdb) r
(gdb) n / s
(gdb) p 变量
(gdb) bt
(gdb) c
(gdb) q
```

记住：GDB 的价值不在于“会多少命令”，而在于**在正确的时机停下来，看到可信的运行时事实**。
                                                                           