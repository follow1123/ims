# Input Mode Switch

* Windows下切换输入法的中英模式


## 必要操作

1. <kbd>Win</kbd>+<kbd>q</kbd> **打开或关闭系统图标**
2. 打开**输入指示**

## 安装

### 直接下载

### 手动编译

* 安装`mingw`环境

```cmd
git clone https://github.com/follow1123/ims.git

cd ims

make
```

## 使用

* `ims.exe` - 获取当前输入法模式，`1`：中文，`2`：英文
* `ims.exe 1` - 切换到英文模式
* `ims.exe 2` - 切换到中文模式
