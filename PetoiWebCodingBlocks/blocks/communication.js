// 添加全局变量
let deviceIP = '';
let deviceModel = '';

// 设置IP地址的函数
function setDeviceIP(ip)
{
  deviceIP = ip;
}

// 获取IP地址的函数
function getDeviceIP()
{
  return deviceIP;
}

// 设置型号的函数
function setDeviceModel(model)
{
  deviceModel = model;
}

// 获取型号的函数
function getDeviceModel()
{
  return deviceModel;
}

// 连接机器人函数实现 - WebSocket版本
async function makeConnection(ip, timeout = TIMEOUT_CONFIG.WEB_REQUEST.CONNECTION_TIMEOUT) {
  try {
    if (window.client) {
      // 如果已经有连接，先断开
      window.client.disconnect();
      window.client = null; // 清除全局客户端实例
    }
    // 创建WebSocket客户端
    const client = new PetoiAsyncClient(`ws://${ip}:81`);
    // window.client = client;
    // 尝试连接
    await client.connect();
    // 发送问号命令测试连接
    const model = await client.sendCommand('?');
    
    // 更严格地检查响应内容，特别识别模拟数据
    if (model && model.length > 0 && model.trim() !== '?' && model.trim() !== '' && model.trim() !== 'PetoiModel-v1.0') {
      setDeviceIP(ip);
      setDeviceModel(model);
      // 设置全局客户端实例
      window.client = client;
      client.startHeartbeat();
      return true;
    } else {
      if (model.trim() === 'PetoiModel-v1.0') {
        alert(getText("connectionFailedMock") + '\n\n' + getText("programExecutionStopped"));
      } else {
        alert(getText("connectionFailedCheck") + '\n\n' + getText("programExecutionStopped"));
      }
      return false;
    }
  } catch (err) {
    client.disconnect();
    // 显示友好的错误信息，并明确说明程序已中断
    if (err.message.includes('timeout') || err.message.includes('超时')) {
      alert(getText("connectionTimeout").replace("{ip}", ip) + '\n\n' + getText("programExecutionStopped"));
    } else if (err.message.includes('Failed to fetch') || err.message.includes('Connection reset') || err.message.includes('ERR_CONNECTION_RESET')) {
      alert(getText("deviceConnectionLost").replace("{ip}", ip) + '\n\n' + getText("checkDeviceAndNetwork") + '\n\n' + getText("programExecutionStopped"));
    } else if (err.message.includes('Network Error') || err.message.includes('网络')) {
      alert(getText("networkError").replace("{ip}", ip) + '\n\n' + getText("programExecutionStopped"));
    } else {
      alert(getText("connectionErrorDetails").replace("{error}", err.message) + '\n\n' + getText("programExecutionStopped"));
    }
    return false;
  }
}

// 关闭连接函数实现
async function closeConnection() {
  try {
    if (window.client) {
      await window.client.disconnect();
      // 清除全局客户端实例
      window.client = null;
      return true;
    }
    return false;
  } catch (err) {
    console.error(getText("closeConnectionError"), err);
    return false;
  }
}

// 全局异步客户端类定义
// PetoiAsyncClient 类已移动到 petoi_async_client.js 文件中

function webRequest(command, timeout = TIMEOUT_CONFIG.WEB_REQUEST.DEFAULT_TIMEOUT, needResponse = true, displayCommand = null, bypassStopCheck = false) {
  return new Promise(async (resolve, reject) =>
    {
      try
      {
        // 检查停止标志（除非明确绕过）
        if (!bypassStopCheck && typeof stopExecution !== 'undefined' && stopExecution) {
          reject(new Error("程序执行被用户停止"));
          return;
        }
        
        // 使用全局的 WebSocket 客户端实例
        if (!window.client) {
          reject(new Error(getText("noConnectionEstablished")));
          return;
        }
        
        // 如果showSentCommands开关激活，打印发送的命令
        if (typeof showSentCommands !== 'undefined' && showSentCommands) {
          // 使用displayCommand参数或默认处理
          let commandToDisplay = displayCommand || command;
          if (!displayCommand && command.startsWith("b64:")) {
            try {
              const decoded = decodeCommand(command);
              if (decoded && decoded.token && decoded.params) {
                commandToDisplay = `${decoded.token} ${decoded.params.join(" ")}`;
              }
            } catch (error) {
              // 如果解码失败，保持原命令
              commandToDisplay = command;
            }
          }
          console.log(getText("sendingCommand") + commandToDisplay);
        }
        
        let result = await window.client.sendCommand(command, timeout);
        if (Array.isArray(result) && result.length == 1) {
          result = result[0];
        }
        
        // 根据 needResponse 参数决定是否返回结果
        // 注意：即使showSentCommands激活，我们也需要返回实际结果给传感器积木块使用
        resolve(needResponse ? result : true);
      } catch (error) {
        console.error(getText("httpRequestError"), error);
        reject(error);
      }
    });
}

function webBatchRequest(commands, timeout = TIMEOUT_CONFIG.WEB_REQUEST.BATCH_REQUEST_TIMEOUT, needResponse = true)
{
  return new Promise(async (resolve, reject) => {
    try
    {
      // 使用全局的 WebSocket 客户端实例
      if (!window.client) {
        reject(new Error(getText("noConnectionEstablished")));
        return;
      }
      
      // 如果showSentCommands开关激活，打印发送的命令
      if (typeof showSentCommands !== 'undefined' && showSentCommands) {
        // 解码base64命令并显示可读格式
        const displayCommands = commands.map(cmd => {
          if (cmd.startsWith("b64:")) {
            try {
              const decoded = decodeCommand(cmd);
              if (decoded && decoded.token && decoded.params) {
                return `${decoded.token} ${decoded.params.join(" ")}`;
              }
            } catch (error) {
              // 如果解码失败，保持原命令
            }
          }
          return cmd;
        });
        console.log(getText("sendingCommand") + displayCommands.join(', '));
      }
      
      const result = await window.client.sendCommand(commands, timeout);
      resolve(needResponse ? result : true);
    } catch (error)
    {
      console.error(getText("webBatchRequestError"), error);
      reject(error);
    }
  });
}

// 添加来自websocket的事件监听
function addWebSocketEventListeners(eventName, callback)
{
   if (!window.client) {
    return;
   }
   window.client.eventTarget.addEventListener(eventName, async (event) => {
     try {
       await callback(event);
     } catch (error) {
       console.error(getText("messageProcessingError"), error);
     }
   });
}

function delay(ms) {
  return new Promise(resolve => setTimeout(resolve, ms));
}

// 辅助函数：根据showSentCommands状态决定是否打印webRequest结果
function logWebRequestResult(result) {
  // 如果showSentCommands激活，则不打印webRequest的返回值
  if (typeof showSentCommands !== 'undefined' && showSentCommands) {
    return;
  }
  console.log(result);
}

// 控制台输入相关函数
let consoleInputPromise = null;
let consoleInputResolve = null;
let currentInputContainer = null; // 跟踪当前的输入容器

// 创建控制台输入界面
function createConsoleInput(prompt) {
  return new Promise((resolve) => {
    // 如果已经有输入等待，先取消之前的
    if (consoleInputPromise) {
      if (currentInputContainer) {
        try {
          currentInputContainer.remove();
        } catch (e) {
          // 静默处理错误
        }
        currentInputContainer = null;
      }
      consoleInputResolve('');
      consoleInputPromise = null;
    }
    
    consoleInputPromise = true;
    consoleInputResolve = resolve;
    
    // 检查程序是否被停止
    if (typeof stopExecution !== 'undefined' && stopExecution) {
      resolve('');
      return;
    }
    
    // 创建输入界面
    const consoleLog = document.getElementById('consoleLog');
    if (!consoleLog) {
      resolve('');
      return;
    }
    
    const inputContainer = document.createElement('div');
    inputContainer.className = 'console-input-container';
    
    const promptSpan = document.createElement('span');
    promptSpan.textContent = prompt;
    promptSpan.style.cssText = `
      color: #FFA500;
      margin-right: 10px;
      font-family: 'Consolas', monospace;
    `;
    
    const inputField = document.createElement('input');
    inputField.type = 'text';
    inputField.className = 'console-input-field';
    
    const submitBtn = document.createElement('button');
    submitBtn.textContent = 'Enter';
    
    inputContainer.appendChild(promptSpan);
    inputContainer.appendChild(inputField);
    inputContainer.appendChild(submitBtn);
    
    // 保存对当前输入容器的引用
    currentInputContainer = inputContainer;
    
    consoleLog.appendChild(inputContainer);
    
    // 延迟聚焦，确保DOM完全渲染
    setTimeout(() => {
      if (inputField && document.contains(inputField)) {
        inputField.focus();
        
        // 设置输入框属性，防止被意外修改
        inputField.setAttribute('data-console-input', 'true');
        inputField.setAttribute('autocomplete', 'off');
        inputField.setAttribute('spellcheck', 'false');
        
        // 确保输入框可编辑
        inputField.removeAttribute('disabled');
        inputField.removeAttribute('readonly');
        inputField.contentEditable = false;
        
        // 监听输入框属性变化
        const observer = new MutationObserver((mutations) => {
          mutations.forEach((mutation) => {
            if (mutation.type === 'attributes') {
              const target = mutation.target;
              if (target.disabled || target.readOnly) {
                target.disabled = false;
                target.readOnly = false;
              }
            }
          });
        });
        
        observer.observe(inputField, {
          attributes: true,
          attributeFilter: ['disabled', 'readonly']
        });
        
        // 防止输入框被其他代码干扰
        const originalFocus = inputField.focus;
        const originalBlur = inputField.blur;
        
        // 重写focus方法，确保聚焦成功
        inputField.focus = function() {
          try {
            originalFocus.call(this);
            // 确保输入框可编辑
            this.disabled = false;
            this.readOnly = false;
            this.contentEditable = false;
          } catch (e) {
            // 静默处理错误
          }
        };
        
        // 重写blur方法，防止意外失去焦点
        inputField.blur = function() {
          // 只有在提交时才允许失去焦点
          if (!consoleInputPromise) {
            originalBlur.call(this);
          } else {
            this.focus();
          }
        };
      }
    }, 100);
    
    // 处理输入提交
    const handleSubmit = () => {
      const value = inputField.value;
      
      // 清理状态
      try {
        if (consoleLog.contains(inputContainer)) {
          consoleLog.removeChild(inputContainer);
        }
      } catch (e) {
        // 静默处理错误
      }
      
      currentInputContainer = null;
      consoleInputPromise = null;
      consoleInputResolve(value);
    };
    
    // 回车键提交
    inputField.addEventListener('keypress', (e) => {
      if (e.key === 'Enter') {
        e.preventDefault(); // 防止默认行为
        handleSubmit();
      }
    });
    
    // 点击按钮提交
    submitBtn.addEventListener('click', (e) => {
      e.preventDefault(); // 防止默认行为
      handleSubmit();
    });
    
    // 防止输入框失去焦点时被意外清理
    inputField.addEventListener('blur', (e) => {
      // 延迟检查，避免在提交过程中触发
      setTimeout(() => {
        if (consoleInputPromise && document.contains(inputField)) {
          // 尝试重新聚焦
          try {
            inputField.focus();
            // 如果重新聚焦失败，再次尝试
            setTimeout(() => {
              if (document.contains(inputField) && document.activeElement !== inputField) {
                inputField.focus();
              }
            }, 50);
          } catch (focusError) {
            // 静默处理错误
          }
        }
      }, 100);
    });
    
    // 防止输入框被意外禁用
    inputField.addEventListener('input', (e) => {
      if (e.target.disabled || e.target.readOnly) {
        e.target.disabled = false;
        e.target.readOnly = false;
      }
    });
    
    // 添加日志条目显示提示
    const logEntry = document.createElement('div');
    logEntry.className = 'log-entry';
    logEntry.style.cssText = `
      color: #FFA500;
      font-family: 'Consolas', monospace;
      margin: 5px 0;
    `;
    
    // 使用与其他console log一致的timestamp格式
    const now = new Date();
    const hours = String(now.getHours()).padStart(2, '0');
    const minutes = String(now.getMinutes()).padStart(2, '0');
    const seconds = String(now.getSeconds()).padStart(2, '0');
    const milliseconds = String(now.getMilliseconds()).padStart(3, '0');
    const timestamp = `[${hours}:${minutes}:${seconds}.${milliseconds}]`;
    logEntry.innerHTML = `<span class="timestamp" style="color: #888; font-family: 'Consolas', monospace;">${timestamp}</span> ${prompt}`;
    consoleLog.appendChild(logEntry);
  });
}

// 控制台输入函数
async function consoleInput(prompt) {
  // 防止重复调用
  if (consoleInputPromise) {
    clearConsoleInput();
    // 等待一小段时间确保清理完成
    await new Promise(resolve => setTimeout(resolve, 50));
  }
  
  return await createConsoleInput(prompt);
}

// 将函数暴露到全局作用域
window.consoleInput = consoleInput;
window.createConsoleInput = createConsoleInput;
window.clearConsoleInput = clearConsoleInput;

// 清理控制台输入界面
function clearConsoleInput() {
  if (consoleInputPromise) {
    // 移除输入界面
    if (currentInputContainer) {
      try {
        if (currentInputContainer.parentNode) {
          currentInputContainer.parentNode.removeChild(currentInputContainer);
        }
      } catch (e) {
        // 静默处理错误
      }
      currentInputContainer = null;
    } else {
      // 备用清理方法
      const consoleLog = document.getElementById('consoleLog');
      if (consoleLog) {
        const inputContainer = consoleLog.querySelector('.console-input-container');
        if (inputContainer) {
          try {
            consoleLog.removeChild(inputContainer);
          } catch (e) {
            // 静默处理错误
          }
        }
      }
    }
    
    // 取消等待
    if (consoleInputResolve) {
      consoleInputResolve('');
    }
    consoleInputPromise = null;
    consoleInputResolve = null;
  }
}
