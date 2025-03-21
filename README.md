# 文件压缩与解压工具 - 基于哈夫曼编码

## 简介

本程序是一款基于 **哈夫曼编码** 实现的文件压缩与解压工具，同时支持 **数据加密功能**。程序内置两种加密方式（偏移量加密和异或+密钥加密），利用 **小根堆优化构建哈夫曼树** 提高了压缩效率，并使用高效算法解压文件。请按照本手册完成安装和使用。

---

## 系统要求

### 操作系统

- **Ubuntu 22.04**（其他 Linux 发行版也可，但请确保安装必要依赖）

### 必备软件

1. **Zenity**（用于图形对话框操作）
2. **GCC 11.4** 或更高版本（如果需要自行编译）
3. **CMake 3.22** 或更高版本（用于构建软件）

### 硬件要求

- **建议配置**：2GB 及以上内存  
- **推荐**：处理器性能较好者可获得更高的压缩/解压速度。

---

## 安装步骤

### 1. 获取程序源代码

```bash
git clone https://github.com/ArcherSore/HUST-CSE-ProgramDesign-2025Spring.git
```

### 2. 构建程序

进入程序根目录后，执行以下命令：

```bash
mkdir build
cd build
cmake ..
make
```

编译完成后，会生成可执行文件 **ProgramDesign**，位于 `bin` 目录下。

---

## 使用说明

### **启动程序**

启动程序后，会弹出初始操作界面，通过 **Zenity 窗口** 提示用户选择 **“压缩文件”** 或 **“解压文件”** 选项。

---

### **压缩文件操作**

1. **选择文件**  
   - 在“压缩文件”模式下，程序将弹出文件选择对话框，请选择需要压缩的文件。

2. **填写信息**  
   - 程序将提示输入 **“发送人”** 和 **“接收人”** 信息。请认真填写，以确保后续解压时信息验证无误。

3. **选择加密方式（可选）**  
   - **偏移量加密**：自动采用固定偏移值对数据进行简单混淆。  
   - **异或+密钥加密**：输入自定义密钥，保证密钥安全（切勿丢失），以便正确解密。

4. **压缩构成**  
   - 系统会自动构建 **哈夫曼树**（内置使用小根堆优化构建方法，对比传统堆排序更高效），进行数据压缩。
   - 压缩完成后，程序会显示“文件压缩成功”提示，并将生成的压缩文件保存为特定后缀（例如：`.hfm`）。

---

### **解压文件操作**

1. **选择待解压文件**  
   - 在“解压文件”模式下，通过文件选择对话框选择压缩文件（例如：`.hfm` 文件）。  
   - 默认情况下只显示 `.hfm` 文件，如果你的文件后缀不是 `.hfm`，需要在界面右上角搜索文件名。

2. **信息验证**  
   - 程序会自动读取文件头（包含“发送人与接收人信息”），验证数据完整性。
   - 请确保信息与压缩时填写一致。

3. **解密操作（如使用）**  
   - 如果压缩文件已加密，系统会提示输入相应的解密密钥，确保密钥与加密时一致。

4. **数据恢复**  
   - 系统会自动读取编码表，并利用 **Trie 字典树** 或 **哈希映射** 方法进行解码，还原出原始文件。
   - 解压完成后，程序会提示“解压成功”，并将恢复的文件保存至 `bin` 目录下。

---

## 注意事项

1. **加密密钥安全**：  
   - 如果选择了“异或+密钥加密”方式，请确保密钥妥善保管，否则文件将无法正确解密。

2. **文件完整性**：  
   - 请勿随意修改生成的 `.hfm` 文件头信息，否则可能导致解压失败。

3. **依赖安装**：  
   - 在其他 Linux 发行版上使用本程序时，请确保安装了 **Zenity**、**GCC** 和 **CMake**。

---

Enjoy! 🎉
