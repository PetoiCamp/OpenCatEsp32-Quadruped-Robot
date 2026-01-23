# 如何增加ESP32 Arduino主循环栈大小

在代码中直接使用 SET_LOOP_TASK_STACK_SIZE() 宏。ESP32 Arduino 2.0.12 支持此方法，无需修改系统文件。

// Set Arduino loop task stack size to 16KB (default is 8KB)
// This must be called before setup() to take effect
SET_LOOP_TASK_STACK_SIZE(16*1024);  // 16KB stack size

这个方法：
不需要修改系统文件（boards.txt）
不需要修改 boards.txt
代码中直接设置，更清晰
适用于 ESP32 Arduino 2.0.12