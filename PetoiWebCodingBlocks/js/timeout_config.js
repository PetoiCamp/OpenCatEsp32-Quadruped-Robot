/**
 * 网页通信超时配置
 * 统一管理所有与网页通信相关的超时时间设置
 */

const TIMEOUT_CONFIG = {
    // WebSocket 连接相关超时 (毫秒)
    WEBSOCKET: {
        CONNECTION_TIMEOUT: 10000,      // WebSocket连接超时：10秒
        HEARTBEAT_INTERVAL: 5000,       // 心跳发送间隔：5秒
        HEARTBEAT_TIMEOUT: 20000,       // 心跳响应超时：20秒
        HEALTH_CHECK_INTERVAL: 10000,   // 连接健康检查间隔：10秒
        RECONNECT_DELAY: 500,           // 重连延迟：0.5秒
        MAX_RECONNECT_ATTEMPTS: 5       // 最大重连次数
    },

    // 命令执行相关超时 (毫秒)
    COMMAND: {
        DEFAULT_TIMEOUT: 30000,         // 默认命令超时：30秒
        SENSOR_READ_TIMEOUT: 5000,      // 传感器读取：5秒
        SIMPLE_COMMAND_TIMEOUT: 20000,  // 普通命令：20秒
        COMPLEX_ACTION_TIMEOUT: 30000,  // 复杂动作：30秒
        MUSIC_COMMAND_TIMEOUT: 120000,  // 音乐播放：2分钟
        ACROBATIC_MOVES_TIMEOUT: 30000, // 杂技动作：30秒
        LONG_COMMAND_TIMEOUT: 60000,    // 长时间命令：60秒
        JOINT_QUERY_TIMEOUT: 5000       // 关节状态查询：5秒
    },

    // 网页请求相关超时 (毫秒)
    WEB_REQUEST: {
        DEFAULT_TIMEOUT: 15000,         // 默认网页请求超时：15秒
        CONNECTION_TIMEOUT: 2000,       // 连接建立超时：2秒
        BATCH_REQUEST_TIMEOUT: 15000    // 批量请求超时：15秒
    },

    // 服务器端超时设置 (毫秒)
    SERVER: {
        TASK_EXECUTION_TIMEOUT: 30000,  // 任务执行超时：30秒
        HEARTBEAT_CHECK_INTERVAL: 10000,// 心跳检查间隔：10秒
        HEARTBEAT_TIMEOUT: 25000,       // 服务器端心跳超时：25秒
        HEALTH_CHECK_INTERVAL: 10000    // 健康检查间隔：10秒
    },

    // 串口操作相关超时 (毫秒)
    SERIAL: {
        PORT_CLOSE_TIMEOUT: 3000,       // 端口关闭超时：3秒
        READER_CANCEL_TIMEOUT: 3000,    // 读取器取消超时：3秒
        WRITER_CLOSE_TIMEOUT: 3000      // 写入器关闭超时：3秒
    },

    // 用户界面相关超时 (毫秒)
    UI: {
        VALIDATION_TIMEOUT_WINDOWS: 8000,  // Windows验证超时：8秒
        VALIDATION_TIMEOUT_OTHER: 200,     // 其他系统验证超时：200ms
        IP_CHECK_DELAY: 100,                // IP检查延迟：100ms
        SERIAL_OUTPUT_BUFFER_DELAY: 30      // 串口输出缓冲延迟：30ms
    }
};

// 导出配置对象
if (typeof module !== 'undefined' && module.exports) {
    module.exports = TIMEOUT_CONFIG;
} else if (typeof window !== 'undefined') {
    window.TIMEOUT_CONFIG = TIMEOUT_CONFIG;
} 
