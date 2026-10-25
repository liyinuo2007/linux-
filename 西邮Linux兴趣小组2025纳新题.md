# 西邮 Linux 兴趣小组 2025 纳新面试题题解

> 来源：西邮 Linux 兴趣小组官方仓库 `xiyou-linuxer/website-2024` 中的 `docs/archive/interview-2025.md`
> 原文链接：https://github.com/xiyou-linuxer/website-2024/blob/main/docs/archive/interview-2025.md
> 官网渲染页：https://xiyoulinux.com/archive/interview-2025
> 发布日期：2025-10-09 19:29:08
- 本题目只作为西邮 Linux 兴趣小组 2025 纳新面试的有限参考。
- 为节省版面，本试题的程序源码省去了 `#include` 指令。（电子版已为 `main()` 补上 `return 0;`）
- 本试题中的程序源码仅用于考察 C 语言基础，不应当作为 C 语言「代码风格」的范例。
- 所有题目编译并运行于 **x86_64 GNU/Linux** 环境。

## 题目总结

这套题几乎全是 C 语言的"底层基本功"：二进制拆解（企鹅题）、printf 的返回值与嵌套调用、`\0` 和字符串函数、浮点数的二进制表示、异或位运算、`argc/argv` 与整数溢出、`const` 与指针的权限、数组指针 / 指针数组 / 函数指针、全局变量与 `static` 局部变量、宏展开的括号陷阱、动态内存与结构体、多级指针、`union` 与柔性数组、结构体内存对齐。最后第 13 题是选做的 GNU/Linux 基础题（`cd`、目录操作）。

## 0. 拼命的企鹅

一只企鹅在爬山，每隔一段路都会遇到一块石头。第 1 块石头重量是 `a`，每往上走一段路，石头重量就会变成上一段的平方。企鹅可以选择把某些石头捡起来，最后把捡到的石头重量相乘。

它怎样捡石头，才能得到重量乘积恰好是 `a` 的 `b` 次方的石头？（比如 `b = 173` 时， 要捡哪些石头？）

### 题目分析

* 石头重量排下来是：第 1 块 a、第 2 块 a²、第 3 块 a⁴、第 4 块 a⁸……**第 n 块的指数是 2 的 (n-1) 次方**。
* 捡石头就是把重量相乘，而"相乘"在指数上就是"相加"，所以要让**捡到的石头指数加起来等于 b**。
* 指数全是 2 的幂，于是问题变成"把 b 拆成 2 的幂之和"——这正是 b 的**二进制**。二进制哪一位是 1，就捡对应的那块石头。

### 解题步骤

1. b = 173 转二进制：**173 = 10101101₂**（验算：128 + 32 + 8 + 4 + 1 = 173）。
2. 从右往左数，1 落在第 1、3、4、6、8 位 → 捡**第 1、3、4、6、8 块**石头。
3. 对应的指数是 2⁰、2²、2³、2⁵、2⁷，即 1 + 4 + 8 + 32 + 128 = 173，对上了。


### 题目反思

* 本题考察"把一道文字题翻译成二进制"：石头指数是 2 的幂，所以不用暴力枚举组合，转个进制就行。
* 第 n 块石头对应的是 **2^(n-1)**。

## 1. 西邮Linux欢迎你啊

请解释以下代码的运行结果。

```c
int main() {
    if (printf("Hi guys! ") || printf("Xiyou Linux ")) {
        printf("%d\n", printf("Welcome to Xiyou Linux 2%d", printf("")));
    }
    return 0;
}
```

### 题目分析

* **printf 的返回值是"实际打印了几个字符"**，不是输出的内容本身。
* `||` 有短路特性：左边为真，右边直接跳过，不执行。
* 嵌套的 printf 是"从最里层往外打印"：里面的先写出，外面那层的返回值再作输出。

### 解题步骤

1. `printf("Hi guys! ")` 打印 9 个字符（"Hi guys! " 含空格）→ 返回 **9**，非 0 即真 → `||` 短路，`printf("Xiyou Linux ")` **一句都不执行**。
2. 进 if，只有一句 printf，从里往外拆：
   * 最里层 `printf("")`：空字符串，打印 0 个字符 → 返回 **0**；
   * 中间层 `printf("Welcome to Xiyou Linux 2%d", 0)`：打印 "Welcome to Xiyou Linux 20"，共 **25** 个字符 → 返回 25；
   * 最外层 `printf("%d\n", 25)`：把 25 接在后面打印出来，再换行。
3. 屏幕上的顺序是先中间层、后最外层，连起来就是：

```
Hi guys! Welcome to Xiyou Linux 2025
```

### 题目反思

* 本题考察 printf 的返回值语义 + `||` 短路 + 嵌套 printf 的输出顺序。
* 那句 `printf("Xiyou Linux ")` 是来骗人的：它看起来一定会执行，其实被短路了，一个字都不会出现。

## 2. 可以和 `\0` 组一辈子字符串吗？

你能找到成功打印的语句吗？你能看出每个 `if` 中函数的返回值吗？这些函数各有什么特点？

```c
int main() {
    char p1[] = { 'W', 'e', 'l', 'c', 'o', 'm', 'e', '\0' };
    char p2[] = "Join us\0", p4[] = "xiyou_linux_group";
    const char* p3 = "Xiyou Linux Group\0\n2025\0";
    if (strcmp(p1, p2)) {
        printf("%s to %s!\n", p1, p2);
    }
    if (strlen(p3) > sizeof(p3)) {
        printf("%s", p3);
    }
    if (sizeof(p1) == sizeof(p2)) {
        printf("%s", p4);
    }
    return 0;
}
```

### 题目分析

* 三个函数不同：
  * `strcmp` 比**内容**：一样返回 0，不一样返回第一个不同字符的 ASCII 差（前减后）。
  * `strlen` **数内存**：从开头数到第一个 `\0` 停下，不算 `\0` 本身。
  * `sizeof` 量**变量类型占多少字节**：数组量整个数组，指针就量 8 字节。
* 字符串中间可以塞 `\0`，但所有"按字符串看"的函数都在**第一个 `\0`** 就停下了。

### 解题步骤

先把三个数实测出来：`sizeof(p1) = 8`、`sizeof(p2) = 9`、`sizeof(p3) = 8`（指针）、`strlen(p3) = 17`。

1. `strcmp(p1, p2)`：'W' 和 'J' 第一个字符就不同 → 87 − 74 = **13**（非 0 即真）→ 打印 `Welcome to Join us!`（p2 打印到第一个 `\0` 就停，"us" 后面的东西看不见）。
2. `strlen(p3) > sizeof(p3)`：strlen 数到第一个 `\0` → "Xiyou Linux Group" 共 **17** 个字符；sizeof(p3) 是**指针**的 8 字节。17 > 8 为真 → 打印 `p3`，`%s` 同样在第一个 `\0` 停 → 输出 `Xiyou Linux Group`。
3. `sizeof(p1) == sizeof(p2)`：8 == 9，**假** → 什么都不打印。

实测输出：

```
Welcome to Join us!
Xiyou Linux Group
```

### 题目反思

* 本题考察 sizeof / strlen / strcmp 的区别。
* 最容易数错的是 p2：`"Join us\0"` 是 "Join us"（7）+ 显式 `\0`（1）+ **编译器补的结尾 `\0`（1）** = 9 个字节。
* `strlen(p3) > sizeof(p3)` 这种"自己比自己"看着像恒假，其实一个是 17、一个是 8，能成立。

## 3. 数学没问题，浮点数有鬼

这个程序的输出是什么？解释为什么会这样？

```c
int main() {
    float a1 = 0.3, b1 = 6e-1, sum1 = 0.9;
    printf("a1 + b1 %s sum1\n", (a1 + b1 == sum1) ? "==" : "!=");
    float a2 = 0x0.3p0, b2 = 0x6p-4, sum2 = 0x0.9p0;
    printf("a2 + b2 %s sum2\n", (a2 + b2 == sum2) ? "==" : "!=");
    return 0;
}
```

### 题目分析

* `float` 只有 23 位小数位，0.3、0.6、0.9 这类十进制小数转成二进制是**无限循环**的，存进来就已经不准了。
* 十六进制浮点 `0x…p…` 的底数是 2：16 = 2⁴，**一位十六进制正好对应 4 位二进制**，所以只要位数够就能精确表示，不丢精度。

### 解题步骤

1. `a1 + b1 == sum1`：把实测值打印到小数点后 20 位看看差在哪：
   * a1 + b1 = **0.90000003576278686523**
   * sum1 = **0.89999997615814208984**
   * 不一样 → 输出 `!=`。
2. `a2 + b2 == sum2`：算一下真值——
   * `0x0.3p0` = 3/16 = 0.1875
   * `0x6p-4` = 6/16 = 0.375
   * `0x0.9p0` = 9/16 = 0.5625
   * 0.1875 + 0.375 = 0.5625，全是精确值 → 输出 `==`。

实测输出：

```
a1 + b1 != sum1
a2 + b2 == sum2
```

### 题目反思

* 本题考察"浮点数为什么不能直接 `==`"：二进制表示本身就存不准。
* 浮点数比较用 `fabs(a - b) < 1e-6` 这种"允许误差"的写法，而不是 `==`。

## 4. 不合群的数字

在一个数组中，所有数字都出现了偶数次，只有两个数字出现了奇数次，请聪明的你帮我看看以下的代码是如何找到这两个数字的呢？

```c
void findUndercoverIDs(int nums[], int size) {
    int xorAll = 0,id_a = 0,id_b = 0;
    for (int i = 0; i < size; i++) {
        xorAll ^= nums[i];
    }
    int diffBit = xorAll & -xorAll;
    for (int i = 0; i < size; i++) {
        if(nums[i] & diffBit){
            id_a ^= nums[i];
        } else {
            id_b ^= nums[i];
        }
    }
    printf("These nums are %d %d\n", id_a, id_b);
}
```

### 题目分析

* 异或的两条性质：`x ^ x = 0`（一样的数互相抵消）、`x ^ 0 = x`；而且异或满足交换律、结合律，顺序随便换。
* 所以**全体异或**一遍：出现偶数次的全部抵消，剩下的就是**两个目标数的异或**（记作 a ^ b）。
* `xorAll & -xorAll`：只保留最低位那个 1，拿到的是"两个目标数不同的最低那一位"。用这一位就能把数组分成两组，**两个目标数必然一组一个**。
* 每组内部再异或，成对的又抵消，各自剩下来的就是答案。

### 解题步骤

拿一组数据试跑：`{1, 2, 3, 4, 5, 1, 2, 3, 4, 5, 6, 7}`（6 和 7 是那两个"不合群"的）。

1. 全体异或 → 6 ^ 7 = 1（其余全抵消）。
2. `diffBit = 1 & -1 = 1`（最低位不一样）。
3. 分组：7（111）最低位是 1 → 进 id_a；6（110）最低位是 0 → 进 id_b；其他成对的各自抵消。
4. 实测输出：

```
These nums are 7 6
```


### 题目反思

* 一个目标数时全体异或就完事；两个目标数就靠"找最低不同位"分两组，各破各的。
* `-x` 在补码里是"按位取反加 1"，所以 `x & -x` 正好能抠出最低位的 1。

## 5. 会一直循环吗？

你了解 `argc` 和 `argv` 吗，程序的输出是什么？为什么会这样？

```c
int main(int argc, char* argv[]) {
    printf("argc = %d\n", argc);
    while (argc++ > 0) {
        if(argc < 0){
            printf("argv[0] %s\n", argv[0]);
            break;
        }
    }
    printf("argc = %d\n", argc);
    return 0;
}
```

### 题目分析

* `argc` 是命令行参数的个数，**程序名也算一个**，所以最小是 1；`argv[0]` 就是程序自己。
* `while (argc++ > 0)` 是"先拿旧值比较，再自增"，所以每轮 argc 都 +1——它其实在数数。
* int 是有符号 32 位，最大 2147483647，**再加 1 就绕到最小 -2147483648**（补码回绕）。

### 解题步骤

1. 进来 argc = 1，先打印 `argc = 1`。
2. 一轮轮 +1：2、3、…、2147483647，每轮都比 0 大，继续。
3. argc = 2147483647 时比较通过 → 自增变成 **-2147483648** → `argc < 0` 成立 → 打印 argv[0]，break。
4. 最后再把 argc 打印一遍。

实测输出：

```
argc = 1
argv[0] ./q5
argc = -2147483648
```

### 题目反思

* 表面上问"会一直循环吗"，实际考两件事：argc/argv 是什么 + 有符号整数溢出。
* argc 涨到最大值再 +1 属于**有符号溢出**，标准里是未定义行为，只是实际表现成"回绕"。无符号的回绕是标准明文规定的。
* `argv[0]` 打印出来就是敲的命令（这里是 `./q5`），程序名也算在 argc 里。

## 6. `const` 与指针：谁能动，谁不能动？

```c
struct P {
    int x;
    const int y;
};

int main() {
    struct P p1 = { 10, 20 }, p2 = { 30, 40 };
    const struct P p3 = { 50, 60 };
    struct P* const ptr1 = &p1;
    const struct P* ptr2 = &p2;
    const struct P* const ptr3 = &p3;
    return 0;
}
```

说说下列操作是否合法，并解释原因：

`ptr1->x = 100`, `ptr2->x = 300`, `ptr3->x = 500`

`ptr1->y = 200`, `ptr1 = &p2`, `ptr2->y = 400`

`ptr2 = &p1`, `ptr3->y = 600`, `ptr3 = &p1`

### 题目分析

* 结构体里 `x` 是普通成员，能改；**`y` 从定义起就是 `const int`**，从哪条路走都改不了。
* 三个指针的区别全在 const 的位置：
  * `struct P* const ptr1`：const 在 `*` **右边** → 锁的是"指针自己"，不能换指向，指向的内容能改；
  * `const struct P* ptr2`：const 在 `*` **左边** → 锁的是"指向的内容"，通过它整个结构体都是只读的，但指针自己能换指向；
  * `const struct P* const ptr3`：两边都有 → 指向和内容全锁死。

> 读法：**从变量名往左读**。`struct P * const ptr1` = "ptr1 是个 const 的指针，指向 struct P"；`const struct P * ptr2` = "ptr2 是个指针，指向 const 的 struct P"。

### 解题步骤


| 操作 | 结果 | 
|---|---|
| `ptr1->x = 111` | ✅ 合法 
| `ptr2->x = 333` | ❌ 
| `ptr3->x = 555` | ❌ 
| `ptr1->y = 222` | ❌ 
| `ptr1 = &p2` | ❌ 
| `ptr2->y = 444` | ❌ 
| `ptr2 = &p1` | ✅ 合法 
| `ptr3->y = 666` | ❌ 
| `ptr3 = &p1` | ❌ 
结论：九条里**只有第 1 条和第 7 条合法**。

### 题目反思

* 两句话记牢：**const 在 `*` 左边锁内容，在 `*` 右边锁指针**；**成员自己带 const（`const int y`），从哪条路都改不了**。



## 7. 指针！数组!

在主函数中定义如下变量:

```c
int main() {
    int a[3] = { 2, 4, 8 };
    int(*b)[3] = &a;
    int* c[3] = { a, a + 1, a + 2 };
    int (*f1(int))(int*, int);
    return 0;
}
```

说说这几个表达式的输出分别是什么？

`a`, `*b`, `*b + 1`, `b`, `b + 1`, `* (*b + 1)`, `c`, `sizeof(a)`, `sizeof(b)`, `sizeof(&a)`, `sizeof(f1)`

### 题目分析

* `int(*b)[3] = &a;` → **数组指针**：b 是个指针，指向"3 个 int 的数组"，一步跨 12 字节。
* `int* c[3]` → **指针数组**：c 是个数组，里面装了 3 个 `int*`；数组名 c 参与运算退化成 `&c[0]`，类型是 `int**`。
* `int (*f1(int))(int*, int);` 是个**函数声明**：f1 收一个 int，返回一个"收两个参数、返回 int 的函数指针"。

### 解题步骤

先把每个表达式的**类型**想清楚，结果自然就出来了（地址每次运行都不一样，看差值和值）：

| 表达式 | 是什么 | 输出|
|---|---|---|
| `a` | 首元素地址 `&a[0]`（int*） | 
| `*b` | 数组 a 本身，参与运算退化成 `&a[0]` | 0x…ea74（和 a 一样） |
| `*b + 1` | 第二个元素的地址 `&a[1]` | 0x…ea78（+4 字节） |
| `b` | "整个数组的地址"（值和 a 相同，类型不同） | 0x…ea74 |
| `b + 1` | 跳过**整个数组** | 0x…ea80（+12 字节） |
| `*(*b + 1)` | `a[1]` 的值 | **4** |
| `c` | 指针数组首元素的地址（int**） | 一个地址，跟上面几个没有必然关系 |
| `sizeof(a)` | 3 × 4 | **12** |
| `sizeof(b)` | 指针大小 | **8** |
| `sizeof(&a)` | "数组的地址"也是指针 | **8** |
| `sizeof(f1)` | 函数类型不能 sizeof | gcc 默认（gnu17）给 **1**；`-std=c17 -pedantic-errors` 会直接报错|

### 题目反思

* 本题考察"地址一样、类型不一样，+1 跨的就完全不一样"：`int*` 一步 4 字节，`int(*)[3]` 一步 12 字节。
* 数组指针和指针数组最容易记混，看括号优先跟谁：`int (*b)[3]` 括号先跟 `*b`，所以 b 是"指针"；`int *c[3]` 里 `[]` 优先，c 是"数组"。
* `sizeof(f1)` 严格讲是标准不支持的（不能对函数类型取 sizeof）。

## 8. 全局还是局部！！！

观察程序输出，思考为什么？

```c
int g;
int func() {
    static int j = 98;
    j += g;
    return j;
}

int main() {
    g += 3;
    char arr[6] = {};
    arr[1] = func();
    arr[0] = func();
    arr[2] = arr[3] = func() + 1;
    arr[4] = func() + 1;
    printf("%s linux\n",arr);
    return 0;
}
```

### 题目分析

* 全局变量 `g` 不初始化默认是 0，`g += 3` 之后是 3。
* `static int j = 98` 写在函数里：**只初始化一次**，函数返回也不销毁，下次进来还接着上次的值用（存在静态存储区，不在栈上）。
* `char arr[6] = {}` 是全零初始化，后面往 arr[0]~arr[4] 里填的是 func() 的返回值——也就是 ASCII 码数字。

### 解题步骤

func 每次调用都做 `j += 3`（g 已经是 3）：

1. 第 1 次：j = 98 + 3 = **101** → `arr[1] = 101` = 'e'
2. 第 2 次：j = 101 + 3 = **104** → `arr[0] = 104` = 'h'
3. 第 3 次：j = 104 + 3 = **107**，`arr[2] = arr[3] = 107 + 1 = 108` = 'l'
4. 第 4 次：j = 107 + 3 = **110**，`arr[4] = 110 + 1 = 111` = 'o'
5. arr[5] 还是 0，正好当字符串的结束符。

从 arr[0] 开始读正好是 "hello"，实测输出：

```
hello linux
```

### 题目反思

* static 局部变量的定位：**作用域还是函数内部，寿命却和全局变量一样**，而且只初始化一次。
* 链式赋值 `arr[2] = arr[3] = func() + 1` 里 func() 只调用**一次**，算出 108 后两个位置都赋 108。

## 9. 宏函数指针

观察程序结果，说说程序运行的过程：

```c
#define CALL_MAIN(main, x) (*(int (*)(int))*main)(x);
#define DOUBLE(x) 2 * x
int (*registry[1])(int);
int main(int argc) {
    if (argc > 2e3) return 0;
    printf("%d ", argc + 1);
    *registry = (int(*)(int))main;
    CALL_MAIN(registry, DOUBLE(argc + 1));
    return 0;
}
```

### 题目分析

* `registry` 是函数指针数组，程序把自己的 main 地址存了进去 → 变成了**自己调自己**的递归。
* `CALL_MAIN(main, x)` 展开成 `(*(int (*)(int))*main)(x);`——把 registry 里存的函数指针取出来调。
* `DOUBLE(x) 2 * x` **没给参数加括号**：`DOUBLE(argc + 1)` 展开是 `2 * argc + 1`，不是 2 × (argc + 1)！

### 解题步骤

1. 启动时 argc = 1：`1 > 2000` 不成立 → 打印 `argc + 1` = **2**。
2. 调用自己时传的参数是 `2 * argc + 1` = 2×1+1 = **3**（不是 4）→ 下一轮 argc = 3，打印 **4**。
3. 参数一路是 7、15、31、63、127、255、511、1023、2047：
   * argc = 1023 时打印 **1024**，然后调 main(2047)；
   * argc = 2047 > 2000 → 直接 return，结束。

实测输出：

```
2 4 8 16 32 64 128 256 512 1024 
```

### 题目反思

* `DOUBLE` 少一对括号，参数就只跟到 `2 *` 后面的第一个东西，展开成 `2 * argc + 1`。宏要写成 `#define DOUBLE(x) (2 * (x))` 才安全。
* "函数也是数据"：函数名就是地址，可以存进数组、也能取出来调用。
* main 递归调用自己是完全合法的，这题就是用它做循环，把"数到多少停"藏在参数里。

## 10. 拼接 排序 去重

本题要求你编写以下函数，不能改动 `main` 函数里的代码。实现对 `arr1` 和 `arr2` 的拼接、排序和去重。你需要自行定义 `result` 结构体并使用 `malloc` 手动开辟内存。

```c
int main() {
    int arr1[] = { 6, 1, 2, 1, 9, 1, 3, 2, 6, 2 };
    int arr2[] = { 4, 2, 2, 1, 6, 2 };
    int len1 = sizeof(arr1) / sizeof(arr1[0]);
    int len2 = sizeof(arr2) / sizeof(arr2[0]);

    struct result result;
    your_concat(arr1, len1, arr2, len2, result);
    print_result(result);
    your_sort(result);
    print_result(result);
    your_dedup(result);
    print_result(result);
    free(result.arr);
    return 0;
}
```

### 题目分析

* 使用"数据指针 + 长度"：`struct result { int *arr; int len; };`
* 三步思路：
  * **拼接**：`malloc` 一块装得下两个

```c
int main() {
    int arr1[] = { 6, 1, 2, 1, 9, 1, 3, 2, 6, 2 };
    int arr2[] = { 4, 2, 2, 1, 6, 2 };
    int len1 = sizeof(arr1) / sizeof(arr1[0]);
    int len2 = sizeof(arr2) / sizeof(arr2[0]);
数组的内存，把 arr1、arr2 依次拷进去（记得判 malloc 失败）。
  * **排序**：冒泡排序，相邻两个比大小交换。
  * **去重**：排序后相同的数一定挨在一起，只要"只保留和前一个不同的值"就行，一趟搞定。
* 题目说不能改 main，且 main 里是值传递 (…, result)。C语言值传递会拷贝副本，函数内修改的 malloc 指针和 len 传不回 main，导致 main 里 free 野指针崩溃。


### 解题步骤

* main 里把传参改成 `&result`（就加一个 & 的事），函数都收 `struct result*`：

```c
your_concat(arr1, len1, arr2, len2, &result);   // ← 只加了 &
your_sort(&result);
your_dedup(&result);
```

实测输出：

```
6 1 2 1 9 1 3 2 6 2 4 2 2 1 6 2 
1 1 1 1 2 2 2 2 2 2 3 4 6 6 6 9 
1 2 3 4 6 9 
```


### 题目反思

* 核心一句话：**函数想改外面的变量，必须拿到地址**（和"值传递 vs 地址传递"是一回事）；传结构体本身也是传副本。
* malloc 了要 free，判空也要做；去重先排序。

## 11. 指针魔法

用你智慧的眼睛，透过这指针魔法的表象，看清其本质：

```c
void magic(int(*pa)[6], int** pp) {
    **pp += (*pa)[2];
    *pp = (*pa) + 5;
    **pp -= (*pa)[0];
    *pp = (*pa) + ((*(*pa + 3) & 1) ? 3 : 1);
    *(*pp) += *(*pp - 1);
    *pp = (*pa) + 2;
}
int main() {
    int a[6] = { 2, 4, 6, 8, 10, 12 };
    int* p = a + 1,** pp = &p;
    magic(&a, pp);
    printf("%d %d\n%d %d %d\n%d %d\n",*p,**pp,a[1],a[2],a[3],a[5],p-a);
    return 0;
}
```

### 题目分析

* 初始状态：a = {2, 4, 6, 8, 10, 12}，p = &a[1]，pp = &p（二级指针）。
* 读法：`pa` 是"数组 a 的地址"，`(*pa)[i]` 就是 a[i]；`*pp` 是 p，`**pp` 是 p 指着的那个数。
* 一行一行跟着走，重点 **p 每次指到哪**。

### 解题步骤

1. `**pp += (*pa)[2]` → a[1] += a[2] → a[1] = 4 + 6 = **10**
2. `*pp = (*pa) + 5` → p 改指 **a[5]**
3. `**pp -= (*pa)[0]` → a[5] -= a[0] → a[5] = 12 − 2 = **10**
4. `*pp = (*pa) + ((*(*pa + 3) & 1) ? 3 : 1)` → a[3] = 8，8 & 1 = 0（偶数）→ 取 1 → p 改指 **a[1]**
5. `*(*pp) += *(*pp - 1)` → a[1] += a[0] → a[1] = 10 + 2 = **12**
6. `*pp = (*pa) + 2` → p 改指 **a[2]**

最后：a = {2, 12, 6, 8, 10, 10}，p = &a[2]。输出：

```
6 6
12 6 8
10 2
```

（第一行 `*p`、`**pp`；第二行 a[1]、a[2]、a[3]；第三行 a[5]、`p - a`。）

### 题目反思

* 每行都要重新确认 p 指到哪儿、动的是哪个数。
* 第 4 行判断奇偶用的是 **a[3]**（不是 a[1]）；第 5 行 `*(*pp - 1)` 是"p 前面那个元素"，不是"pp 减一"。
* `p - a` 是两个指针相减，结果是**元素个数**（2），不是字节差（8）。

## 12. 奇怪的循环

你能看明白这个程序怎样运行吗？试着理解这个程序吧！

```c
union data {
    void**** p;
    char arr[20];
};
typedef struct node {
    int a;
    union data b;
    void (*use)(struct node* n);
    char string[0];
} Node;
void func2(Node* node);

void func1(Node* node) {
    node->use = func2;
    printf("%s\n", node->string);
}
void func2(Node* node) {
    node->use = func1;
    printf("%d\n", ++(node->a));
}
int main() {
    const char* s = "Your journey begins here!";
    Node* P = (Node*)malloc(sizeof(Node) + (strlen(s) + 1) * sizeof(char));
    strcpy(P->string, s);
    P->use = func1;
    P->a = sizeof(Node) * 50 + sizeof(union data);
    while (P->a < 2028) {
        P->use(P);12 };
    int* p = a + 1,** pp = &p;
    magic(&a, pp);
    printf("%d %d\n%d %d %d\n%d %d\n",*p,**pp,a[1],a[2],a[3],a[5],p-a);
    return 0;
    }
    free(P);
    return 0;
}
```

### 题目分析

* `union data`：最大成员是 `char arr[20]`（20 字节），但里面还有个 8 字节的指针，得按 8 对齐 → 大小是 **24**（20 向上取到 8 的倍数）。
* `Node` 的布局：int a（4）+ 填充 4 + union（24）+ 函数指针（8）+ `char string[0]`（0）→ 一共 **40** 字节。
* `char string[0]` 是**柔性数组**：本身不占空间，但 malloc 的时候要多申请一段给它存字符串。
* `use` 是函数指针，在 func1 / func2 之间来回切换，实现两个函数**交替执行**。

### 解题步骤

1. 先算大小：`sizeof(union data) = 24`，`sizeof(Node) = 40`，初值 `P->a = 40 × 50 + 24 = 2024`。
2. 循环条件 `P->a < 2028`，每轮流程：
   * func1：把 use 换成 func2，打印 `string`；
   * func2：把 use 换成 func1，a 自增后打印。
3. 所以输出是"字符串、数字、字符串、数字……"交替，数字从 2025 涨到 2028，条件不成立后退出。

输出：

```
sizeof(union data)=24 sizeof(Node)=40
a start = 2024
Your journey begins here!
2025
Your journey begins here!
2026
Your journey begins here!
2027
Your journey begins here!
2028
```

### 题目反思

* 这题是把 union、柔性数组、函数指针、结构体对齐全揉进了 20 行里。**读的时候先算清楚"内存里长什么样"，再跟执行流程**。
* union 大小不是 20 而是 24（对齐到12 };
    int* p = a + 1,** pp = &p;
    magic(&a, pp);
    printf("%d %d\n%d %d %d\n%d %d\n",*p,**pp,a[1],a[2],a[3],a[5],p-a);
    return 0; 8）；Node 也不是 36 而是 40（int 后面有 4 字节填充，让 union 对齐到 8）。
* 柔性数组 `char string[0]` 自己不占空间，但 malloc 时必须把字符串长度算进去，不然 strcpy 会写到别人家的内存上。

## 13. GNU/Linux (选做)

注：嘿！你或许对 Linux 命令不是很熟悉，甚至你没听说过 Linux。但别担心，这是选做题，了解 Linux 是加分项，但不了解也不扣分哦！

1. 你知道 `cd` 命令的用法与 `/` `~` `-` 这些符号的含义吗？
2. 你知道 Linux 系统如何创建和删除一个目录吗？
3. 请问你还懂得哪些与 GNU/Linux 相关的知识呢？

### 题目分析

三道小题分别对应：`cd` 的用法和三个特殊符号、目录的创建与删除、以及"平时还摸过什么"。选做题，会多少写多少。
12 };
    int* p = a + 1,** pp = &p;
    magic(&a, pp);
    printf("%d %d\n%d %d %d\n%d %d\n",*p,**pp,a[1],a[2],a[3],a[5],p-a);
    return 0;
### 解题步骤

1. `cd` 与三个符号：

```bash
cd /       # 切到根目录
cd ~       # 切到家目录
cd -       # 切回"上一次待的目录"
```

2. 创建 / 删除目录：

```bash
mkdir 实验目录                                # 建一个目录
touch 实验目录/a.txt 实验目录/b.md             # 往里放两个文件
ls 实验目录                                   # 看看：a.txt  b.md
rmdir 实验目录                                # 报错：Directory not empty
rm -r 实验目录                                # 连里面的文件一起删掉
```

`rmdir` 只能删**空目录**；非空目录要用 `rm -r`（r = recursive，递归删除）。

3. 还了解的一些 GNU/Linux 知识：
   * 文件权限 rwx 与 `chmod`（比如 755），以及所有者/所属组的概念；
   * 包管理：Ubuntu 用 `apt`（换源改 `/etc/apt/sources.list`）；
   * 进程相关：`ps aux` 看进程、`kill` 结束进程；
   * 管道 `|` 和重定向 `>` `>>`；
   * 目录结构：配置在 `/etc`、软件在 `/usr`、自己的东西放 `/home`、日志在 `/var`；
   * 发行版：Ubuntu、Debian、Fedora、Arch、Kali 。

### 题目反思

* `rmdir` 和 `rm -r` 的区别值得自己踩一次：一个只收拾空目录，一个连窝端。
* 安全提醒：`rm` 删掉的东西**不进回收站**；`rm -rf` 是"不问了直接删"，敲之前一定看清路径。

---

