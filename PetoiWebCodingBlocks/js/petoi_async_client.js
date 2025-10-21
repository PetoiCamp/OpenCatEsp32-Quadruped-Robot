/**
 * PetoiWebBlock WebSocket客户端
 * 实现实时双向通信
 */


class PetoiAsyncClient
{
    constructor(baseUrl = null)
    {
        this.baseUrl = baseUrl || `ws://${window.location.hostname}:81`;
        this.taskTimeout = TIMEOUT_CONFIG.COMMAND.DEFAULT_TIMEOUT; // 使用配置的默认超时
        this.ws = null;
        this.connected = false;
        this.pendingTasks = new Map();
        this.reconnectAttempts = 0;
        this.maxReconnectAttempts = TIMEOUT_CONFIG.WEBSOCKET.MAX_RECONNECT_ATTEMPTS;
        this.reconnectDelay = TIMEOUT_CONFIG.WEBSOCKET.RECONNECT_DELAY; // 使用配置的重连延迟
        this.heartbeatInterval = null;
        this.heartbeatTimeout = null;
        this.heartbeatIntervalMs = TIMEOUT_CONFIG.WEBSOCKET.HEARTBEAT_INTERVAL;  // 使用配置的心跳间隔
        this.heartbeatTimeoutMs = TIMEOUT_CONFIG.WEBSOCKET.HEARTBEAT_TIMEOUT;  // 使用配置的心跳超时
        this.lastHeartbeatTime = 0;       // 记录最后一次心跳时间
        this.heartbeatPaused = false;     // 跟踪心跳是否被暂停
        this.eventTarget = new EventTarget();
        this.clientId = Date.now().toString(); // 唯一客户端ID
        
        // 连接健康检查
        this.healthCheckInterval = null;
        this.healthCheckIntervalMs = TIMEOUT_CONFIG.WEBSOCKET.HEALTH_CHECK_INTERVAL; // 使用配置的健康检查间隔
        this.autoReconnect = true; // 自动重连开关
        this.connectionTimeout = TIMEOUT_CONFIG.WEBSOCKET.CONNECTION_TIMEOUT; // 使用配置的连接超时
        
        // 连接状态监控
        this.lastActivityTime = 0; // 最后活动时间
        this.connectionStartTime = 0; // 连接开始时间
    }

    /**
     * 启动心跳
     */
    startHeartbeat()
    {
        // 清除可能存在的旧心跳
        this.stopHeartbeat();

        // 设置心跳定时器
        this.heartbeatInterval = setInterval(() => {
            if (this.connected && this.ws && this.ws.readyState === WebSocket.OPEN) {
                this.sendHeartbeat();
                
                // 设置心跳超时
                this.heartbeatTimeout = setTimeout(() => {
                    if (typeof showDebug !== 'undefined' && showDebug) {
                        console.log(getText('heartbeatTimeout'));
                    }
                    this.handleConnectionFailure(getText('heartbeatTimeout'));
                }, this.heartbeatTimeoutMs);
            }
        }, this.heartbeatIntervalMs);
        
        // 立即发送第一个心跳
        setTimeout(() => {
            if (this.connected) {
                this.sendHeartbeat();
            }
        }, 1000);
    }

    /**
     * 停止心跳
     */
    stopHeartbeat()
    {
        if (this.heartbeatInterval) {
            clearInterval(this.heartbeatInterval);
            this.heartbeatInterval = null;
            
            // 只在调试模式下显示心跳停止信息
            if (typeof showDebug !== 'undefined' && showDebug) {
                console.log(getText('heartbeatStopped'));
            }
        }
        if (this.heartbeatTimeout) {
            clearTimeout(this.heartbeatTimeout);
            this.heartbeatTimeout = null;
        }
    }

    /**
     * 启动连接健康检查
     */
    startHealthCheck()
    {
        this.stopHealthCheck();
        
        this.healthCheckInterval = setInterval(() => {
            if (this.connected && this.ws) {
                this.checkConnectionHealth();
            }
        }, this.healthCheckIntervalMs);
    }

    /**
     * 停止连接健康检查
     */
    stopHealthCheck()
    {
        if (this.healthCheckInterval) {
            clearInterval(this.healthCheckInterval);
            this.healthCheckInterval = null;
        }
    }

    /**
     * 检查连接健康状态
     */
    checkConnectionHealth()
    {
        if (!this.connected || !this.ws) {
            return false;
        }

        // 检查WebSocket状态
        if (this.ws.readyState !== WebSocket.OPEN) {
            if (typeof showDebug !== 'undefined' && showDebug) {
                console.log('WebSocket not in OPEN state, attempting reconnect...');
            }
            this.handleConnectionFailure('WebSocket not open');
            return false;
        }

        // 检查最后活动时间
        const now = Date.now();
        if (now - this.lastActivityTime > this.heartbeatTimeoutMs * 2) {
            if (typeof showDebug !== 'undefined' && showDebug) {
                console.log('No activity detected, attempting reconnect...');
            }
            this.handleConnectionFailure('No activity detected');
            return false;
        }

        return true;
    }

    /**
     * 处理连接失败
     */
    handleConnectionFailure(reason)
    {
        if (typeof showDebug !== 'undefined' && showDebug) {
            console.log(getText('connectionFailure').replace('{reason}', reason));
        }
        this.connected = false;
        this.stopHeartbeat();
        this.stopHealthCheck();
        
        if (this.autoReconnect) {
            this.handleReconnect();
        }
        
        // 触发连接失败事件
        this.eventTarget.dispatchEvent(new CustomEvent('connectionFailed', {
            detail: { reason: reason }
        }));
    }

    /**
     * 连接到WebSocket服务器
     */
    async connect()
    {
        return new Promise((resolve, reject) => {
            // 如果已有连接，先断开
            if (this.ws) {
                this.ws.close();
                this.ws = null;
            }

            this.connectionStartTime = Date.now();
            
            try {
                this.ws = new WebSocket(this.baseUrl);

                // 设置连接超时
                const connectionTimeout = setTimeout(() => {
                    if (this.ws && this.ws.readyState === WebSocket.CONNECTING) {
                        // 只在调试模式下显示连接超时信息
                        if (typeof showDebug !== 'undefined' && showDebug) {
                            console.log('Connection timeout');
                        }
                        this.ws.close();
                        reject(new Error('Connection timeout'));
                    }
                }, this.connectionTimeout);

                this.ws.onopen = () => {
                    clearTimeout(connectionTimeout);
                    this.connected = true;
                    this.reconnectAttempts = 0;
                    this.lastActivityTime = Date.now();
                    
                    // 只在调试模式下显示连接成功信息
                    if (typeof showDebug !== 'undefined' && showDebug) {
                        console.log(getText('websocketConnected'));
                    }
                    
                    // 启动心跳和健康检查
                    this.startHeartbeat();
                    this.startHealthCheck();
                    
                    resolve();
                };

                this.ws.onclose = (event) => {
                    clearTimeout(connectionTimeout);
                    this.connected = false;
                    this.stopHeartbeat();
                    this.stopHealthCheck();
                    
                    // 只在调试模式下显示连接关闭信息
                    if (typeof showDebug !== 'undefined' && showDebug) {
                        console.log(getText('websocketClosed'), event.code, event.reason);
                    }
                    
                    // 如果不是正常关闭，尝试重连
                    if (event.code !== 1000 && this.autoReconnect) {
                        this.handleConnectionFailure('Connection closed unexpectedly');
                    }
                };

                this.ws.onerror = (error) => {
                    clearTimeout(connectionTimeout);
                    console.error(getText('websocketError'), error);
                    reject(error);
                };

                this.ws.onmessage = (event) => {
                    this.lastActivityTime = Date.now();
                    this.handleMessage(event.data);
                };
                
            } catch (error) {
                clearTimeout(connectionTimeout);
                // 只在调试模式下显示WebSocket创建失败错误
                if (typeof showDebug !== 'undefined' && showDebug) {
                    console.error('Failed to create WebSocket connection:', error);
                }
                reject(error);
            }
        });
    }

    /**
     * 处理重连
     */
    handleReconnect()
    {
        if (this.reconnectAttempts < this.maxReconnectAttempts) {
            this.reconnectAttempts++;
            const delay = this.reconnectDelay * Math.pow(2, this.reconnectAttempts - 1); // 指数退避
            if (typeof showDebug !== 'undefined' && showDebug) {
                console.log(getText('reconnectAttempt').replace('{current}', this.reconnectAttempts).replace('{max}', this.maxReconnectAttempts) + ` (delay: ${delay}ms)`);
            }
            
            setTimeout(() => {
                this.connect().catch(error => {
                    if (typeof showDebug !== 'undefined' && showDebug) {
                        console.error('Reconnect failed:', error);
                    }
                    if (this.reconnectAttempts < this.maxReconnectAttempts) {
                        this.handleReconnect();
                    } else {
                        if (typeof showDebug !== 'undefined' && showDebug) {
                            console.error(getText('maxReconnectAttempts'));
                        }
                        this.eventTarget.dispatchEvent(new CustomEvent('maxReconnectAttemptsReached'));
                    }
                });
            }, delay);
        } else {
            if (typeof showDebug !== 'undefined' && showDebug) {
                console.error(getText('maxReconnectAttempts'));
            }
            this.eventTarget.dispatchEvent(new CustomEvent('maxReconnectAttemptsReached'));
        }
    }

    /**
     * 发送心跳消息
     */
    sendHeartbeat()
    {
        if (this.ws && this.connected && this.ws.readyState === WebSocket.OPEN) {
            const now = Date.now();
            const timeSinceLastHeartbeat = now - this.lastHeartbeatTime;
            
            // 只在调试模式下显示心跳信息
            if (typeof showDebug !== 'undefined' && showDebug) {
                console.log(getText('heartbeatSent').replace('{time}', timeSinceLastHeartbeat));
            }
            
            const heartbeatMessage = {
                type: 'heartbeat',
                timestamp: now
            };
            
            try {
                this.ws.send(JSON.stringify(heartbeatMessage));
                this.lastHeartbeatTime = now;
                this.lastActivityTime = now;
            } catch (error) {
                // 只在调试模式下显示心跳发送失败错误
                if (typeof showDebug !== 'undefined' && showDebug) {
                    console.error('Failed to send heartbeat:', error);
                }
                this.handleConnectionFailure('Heartbeat send failed');
            }
        }
    }

    /**
     * 处理接收到的消息
     */
    handleMessage(data)
    {
        try {
            // 只在调试模式下显示消息处理信息
            if (typeof showDebug !== 'undefined' && showDebug) {
                console.log('handleMessage', data);
            }
            
            // 检查data是否为null或undefined
            if (!data) {
                if (typeof showDebug !== 'undefined' && showDebug) {
                    console.warn('Received null or undefined data in handleMessage');
                }
                return;
            }
            
            // 清理数据中的特殊字符
            const cleanData = data.replace(/[\r\n\t\f\v]/g, ' ').trim();
            const message = JSON.parse(cleanData);
            
            // 只在调试模式下显示消息类型
            if (typeof showDebug !== 'undefined' && showDebug) {
                console.log('message type', message.type);
            }
            
            // 更新最后活动时间
            this.lastActivityTime = Date.now();
            
            // 处理心跳响应
            if (message.type === 'heartbeat') {
                const now = Date.now();
                const latency = now - this.lastHeartbeatTime;
                
                // 只在调试模式下显示心跳信息
                if (typeof showDebug !== 'undefined' && showDebug) {
                    console.log(getText('heartbeatResponse').replace('{latency}', latency));
                }
                
                if (this.heartbeatTimeout) {
                    clearTimeout(this.heartbeatTimeout);
                    this.heartbeatTimeout = null;
                }
                return;
            }

            // 处理连接成功响应
            if (message.type === 'connected') {
                // 只在调试模式下显示连接确认信息
                if (typeof showDebug !== 'undefined' && showDebug) {
                    console.log(getText('connectionConfirmed'));
                }
                return;
            }

            // 处理错误消息
            if (message.error) {
                console.error(getText('serverError'), message.error);
                this.eventTarget.dispatchEvent(new CustomEvent('serverError', {
                    detail: { error: message.error }
                }));
                return;
            }

            // 处理任务相关消息
            if (message.taskId && this.pendingTasks.has(message.taskId)) {
                const task = this.pendingTasks.get(message.taskId);
                
                switch (message.status) {
                    case 'running':
                        task.onProgress && task.onProgress(message);
                        break;
                    case 'completed':
                        if (message.results && message.results.length > 0) {
                            task.resolve(message.results);
                        } else {
                            task.reject(new Error("response no results"));
                        }
                        this.pendingTasks.delete(message.taskId);
                        break;
                    case 'error':
                        task.reject(new Error(message.error || 'Task failed'));
                        this.pendingTasks.delete(message.taskId);
                        break;
                }
                return;
            }

            if (message.event) {
                this.eventTarget.dispatchEvent(new CustomEvent(message.event, message));
            }
        } catch (error) {
            // 只在调试模式下显示消息处理错误
            if (typeof showDebug !== 'undefined' && showDebug) {
                console.error(getText('messageProcessingError'), error);
                console.error(getText('rawData'), data);
            }
        }
    }

    /**
     * 获取命令的超时时间
     * @param {string|Array} command - 命令或命令数组
     * @returns {number} - 超时时间（毫秒）
     */
    getCommandTimeout(command) {
        const commands = Array.isArray(command) ? command : [command];
        
        // 检查是否为传感器读取命令
        const isSensorCommand = commands.some(cmd => 
            cmd.includes('get_digital_input') || 
            cmd.includes('get_analog_input') || 
            cmd.includes('get_sensor_input') ||
            cmd.includes('get_ultrasonic_distance') ||
            cmd.includes('get_joint_angle') ||
            cmd.includes('get_all_joint_angles')
        );
        
        // 检查是否为复杂动作命令
        const isComplexAction = commands.some(cmd => 
            cmd.includes('acrobatic_moves') ||
            cmd.includes('high_difficulty_action') ||
            cmd.includes('complex_sequence') ||
            cmd.includes('clap') ||
            cmd.includes('kclap') ||
            cmd.includes('pee') ||
            cmd.includes('kpee') ||
            cmd.includes('hunt') ||
            cmd.includes('khunt') ||
            cmd.startsWith('b64:Q') ||  // 音乐播放命令 (B命令base64编码)
            cmd.startsWith('b64:S') ||  // 复杂动作序列命令 (K命令base64编码)
            (cmd.length > 50)           // 长命令通常是复杂操作
        );
        
        // 检查是否为音乐播放命令（需要更长超时）
        const isMusicCommand = commands.some(cmd => 
            cmd.startsWith('b64:Q') ||
            (cmd.startsWith('B ') && cmd.split(' ').length > 10)
        );
        
        if (isSensorCommand) {
            return TIMEOUT_CONFIG.COMMAND.SENSOR_READ_TIMEOUT; // 传感器读取超时
        } else if (isMusicCommand) {
            return TIMEOUT_CONFIG.COMMAND.MUSIC_COMMAND_TIMEOUT; // 音乐播放超时
        } else if (isComplexAction) {
            return TIMEOUT_CONFIG.COMMAND.COMPLEX_ACTION_TIMEOUT; // 复杂动作超时
        } else {
            return TIMEOUT_CONFIG.COMMAND.SIMPLE_COMMAND_TIMEOUT; // 普通命令超时
        }
    }

    /**
     * 发送命令
     * @param {string|Array} command - 单个命令或命令数组
     * @param {number} timeout - 超时时间（毫秒），如果不指定则自动判断
     * @returns {Promise} - 返回命令执行完成后的结果
     */
    async sendCommand(command, timeout = null)
    {
        // 检查连接状态
        if (!this.connected || !this.ws || this.ws.readyState !== WebSocket.OPEN) {
            // 尝试重新连接
            if (this.autoReconnect) {
                // 只在调试模式下显示重连信息
                if (typeof showDebug !== 'undefined' && showDebug) {
                    console.log('Connection lost, attempting reconnect before sending command...');
                }
                try {
                    await this.connect();
                } catch (error) {
                    throw new Error(getText('notConnected') + ' (reconnect failed)');
                }
            } else {
                throw new Error(getText('notConnected'));
            }
        }

        // 将单个命令转换为命令数组
        const commands = Array.isArray(command) ? command : [command];

        if (commands.length === 0) {
            throw new Error('Invalid command');
        }

        // 如果没有指定超时时间，则自动判断
        const actualTimeout = timeout || this.getCommandTimeout(commands);

        // 暂停心跳检测（避免长时间命令执行时心跳超时）
        const wasHeartbeatActive = this.heartbeatInterval !== null;
        if (wasHeartbeatActive && actualTimeout > 15000) { // 只对超过15秒的命令暂停心跳
            this.stopHeartbeat();
            this.heartbeatPaused = true;
        }

        // 检查是否包含陀螺仪禁用命令
        const hasGyroDisableCommand = commands.some(cmd => {
            const cmdLower = cmd.toLowerCase();
            return cmdLower.includes('gu') || cmdLower.includes('gb') || cmdLower.includes('gf');
        });

        return new Promise((resolve, reject) => {
            const taskId = Date.now().toString();
            const message = {
                type: 'command',
                taskId: taskId,
                commands: commands,
                timestamp: Date.now()
            };

            const timeoutId = setTimeout(() => {
                this.pendingTasks.delete(taskId);
                // 恢复心跳检测
                if (this.heartbeatPaused) {
                    this.startHeartbeat();
                    this.heartbeatPaused = false;
                }
                reject(new Error(getText('taskTimeout').replace('{timeout}', actualTimeout)));
            }, actualTimeout);

            // 添加停止标志检查定时器（每100ms检查一次）
            const stopCheckInterval = setInterval(() => {
                if (typeof stopExecution !== 'undefined' && stopExecution) {
                    clearTimeout(timeoutId);
                    clearInterval(stopCheckInterval);
                    this.pendingTasks.delete(taskId);
                    reject(new Error("程序执行被用户停止"));
                }
            }, 100);

            this.pendingTasks.set(taskId, {
                resolve: async (result) => {
                    clearTimeout(timeoutId);
                    clearInterval(stopCheckInterval);
                    this.pendingTasks.delete(taskId);
                    
                    // 如果执行了陀螺仪禁用命令，等待100ms让主机端关闭gyro任务
                    if (hasGyroDisableCommand) {
                        if (typeof showDebug !== 'undefined' && showDebug) {
                            console.log('Gyroscope disable command detected, waiting 100ms for task shutdown...');
                        }
                        await this.delay(200);
                    }
                    
                    // 恢复心跳检测
                    if (this.heartbeatPaused) {
                        this.startHeartbeat();
                        this.heartbeatPaused = false;
                    }
                    if (Array.isArray(command)) {
                        resolve(result.map(item => parseInt(item) || 0));
                    } else {
                        resolve(result[0]);
                    }
                },
                reject: (error) => {
                    clearTimeout(timeoutId);
                    clearInterval(stopCheckInterval);
                    this.pendingTasks.delete(taskId);
                    // 恢复心跳检测
                    if (this.heartbeatPaused) {
                        this.startHeartbeat();
                        this.heartbeatPaused = false;
                    }
                    reject(error);
                }
            });
            
            const messageStr = JSON.stringify(message);
            // 只在debug模式下打印完整的JSON消息
            if (typeof showDebug !== 'undefined' && showDebug) {
                console.log('send message', messageStr);
            }
            
            try {
                this.ws.send(messageStr);
                this.lastActivityTime = Date.now();
            } catch (error) {
                clearTimeout(timeoutId);
                clearInterval(stopCheckInterval);
                this.pendingTasks.delete(taskId);
                // 恢复心跳检测（即使发送失败也要恢复）
                if (this.heartbeatPaused) {
                    this.startHeartbeat();
                    this.heartbeatPaused = false;
                }
                this.handleConnectionFailure('Send command failed');
                reject(error);
            }
        });
    }

    /**
     * 关闭连接
     */
    disconnect()
    {
        this.autoReconnect = false; // 禁用自动重连
        this.stopHeartbeat(); // 停止心跳
        this.stopHealthCheck(); // 停止健康检查
        
        if (this.ws)
        {
            this.ws.close(1000, 'Client disconnect'); // 正常关闭
            this.ws = null;
            this.connected = false;
        }
    }

    /**
     * 延迟函数
     */
    delay(ms) {
        return new Promise(resolve => setTimeout(resolve, ms));
    }

    /**
     * 获取连接状态
     */
    getConnectionStatus() {
        return {
            connected: this.connected,
            readyState: this.ws ? this.ws.readyState : null,
            reconnectAttempts: this.reconnectAttempts,
            lastActivityTime: this.lastActivityTime,
            connectionDuration: this.connectionStartTime ? Date.now() - this.connectionStartTime : 0
        };
    }

    /**
     * 启用/禁用自动重连
     */
    setAutoReconnect(enabled) {
        this.autoReconnect = enabled;
    }
}

// 导出类
if (typeof module !== 'undefined' && module.exports) {
    module.exports = PetoiAsyncClient;
}
