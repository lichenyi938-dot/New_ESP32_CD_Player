# 编译与烧录说明 (简体中文)

推荐环境:
- ESP-IDF v4.4 或 v5.x
- Python 3.8+
- 已配置的 xtensa-esp32s3 交叉编译工具链
- esptool.py 用于烧录

1) 将源码（original_ok/）和 patches/ 中的改动合并到一个项目目录下（例如 New_ESP32_CD_Player_project）
2) 进入项目目录，设置目标：
   ```
   idf.py set-target esp32s3
   ```
3) 打开 menuconfig，启用关键选项：
   - Component config -> Bluetooth -> Enable Bluetooth
   - Component config -> ESP USB Host -> Enable USB Host
   - Component config -> SPI RAM/PSRAM -> Enable if board has PSRAM
   根据需要调整堆栈和日志等级。
4) 编译:
   ```
   idf.py build
   ```
   成功后生成位于 build/ 的固件文件（app.bin 或分段镜像）

5) 烧录（示例）:
   ```
   esptool.py --chip esp32s3 --port COM3 --baud 460800 write_flash          0x0 build/bootloader/bootloader.bin 0x8000 build/partition_table/partition-table.bin          0x10000 build/your_app.bin
   ```

如果你在编译或烧录过程中遇到任何错误，把终端输出贴给我，我会帮你分析并给出修改建议。
