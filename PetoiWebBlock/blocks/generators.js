/**
 * JavaScript代码生成器 - 为所有自定义积木生成JavaScript代码
 */

// 辅助函数：包装异步操作，添加停止检查
function wrapAsyncOperation(operation) {
    return `checkStopExecution();
await (async function() {
  ${operation}
  return true;
})()`;
}

// 代码生成:发送步态动作命令

// 使用统一的超时配置
const COMMAND_TIMEOUT_MAX = TIMEOUT_CONFIG.COMMAND.DEFAULT_TIMEOUT; // 默认命令超时
const LONG_COMMAND_TIMEOUT = TIMEOUT_CONFIG.COMMAND.LONG_COMMAND_TIMEOUT; // 长时间命令超时
const ACROBATIC_MOVES_TIMEOUT = TIMEOUT_CONFIG.COMMAND.ACROBATIC_MOVES_TIMEOUT; // 杂技动作超时
const JOINT_QUERY_TIMEOUT = TIMEOUT_CONFIG.COMMAND.JOINT_QUERY_TIMEOUT; // 关节查询超时

Blockly.JavaScript.forBlock["gait"] = function (block) {
    const cmd = block.getFieldValue("COMMAND");
    const delay = block.getFieldValue("DELAY");
    const delayMs = Math.round(delay * 1000);
    let code = wrapAsyncOperation(`const result = await webRequest("${cmd}", 20000, true); if (result !== null) console.log(result);`) + '\n';
    if (delayMs > 0) {
        // 对于长时间延时，分段检查停止标志
        if (delayMs > 100) {
            code += `await (async () => {
  const checkInterval = 100; // 每100ms检查一次
  const totalChecks = Math.ceil(${delayMs} / checkInterval);
  for (let i = 0; i < totalChecks; i++) {
    checkStopExecution();
    await new Promise(resolve => setTimeout(resolve, Math.min(checkInterval, ${delayMs} - i * checkInterval)));
  }
})();\n`;
        } else {
            code += `checkStopExecution();\nawait new Promise(resolve => setTimeout(resolve, ${delayMs}));\n`;
        }
    }
    return code;
};

// 代码生成:发送姿势动作命令
Blockly.JavaScript.forBlock["posture"] = function (block) {
    const cmd = block.getFieldValue("COMMAND");
    const delay = block.getFieldValue("DELAY");
    const delayMs = Math.round(delay * 1000);
    
    let code = wrapAsyncOperation(`const result = await webRequest("${cmd}", 10000, true); if (result !== null) console.log(result);`) + '\n';
    if (delayMs > 0) {
        // 对于长时间延时，分段检查停止标志
        if (delayMs > 100) {
            code += `await (async () => {
  const checkInterval = 100; // 每100ms检查一次
  const totalChecks = Math.ceil(${delayMs} / checkInterval);
  for (let i = 0; i < totalChecks; i++) {
    checkStopExecution();
    await new Promise(resolve => setTimeout(resolve, Math.min(checkInterval, ${delayMs} - i * checkInterval)));
  }
})();\n`;
        } else {
            code += `checkStopExecution();\nawait new Promise(resolve => setTimeout(resolve, ${delayMs}));\n`;
        }
    }
    return code;
};

// 代码生成:发送杂技动作命令
Blockly.JavaScript.forBlock["acrobatic_moves"] = function (block) {
    const cmd = block.getFieldValue("COMMAND");
    const delay = block.getFieldValue("DELAY");
    const delayMs = Math.round(delay * 1000);
    let code = wrapAsyncOperation(`const result = await webRequest("${cmd}", ${ACROBATIC_MOVES_TIMEOUT}, true); if (result !== null) console.log(result);`) + '\n';
    if (delayMs > 0) {
        // 对于长时间延时，分段检查停止标志
        if (delayMs > 100) {
            code += `await (async () => {
  const checkInterval = 100; // 每100ms检查一次
  const totalChecks = Math.ceil(${delayMs} / checkInterval);
  for (let i = 0; i < totalChecks; i++) {
    checkStopExecution();
    await new Promise(resolve => setTimeout(resolve, Math.min(checkInterval, ${delayMs} - i * checkInterval)));
  }
})();\n`;
        } else {
            code += `checkStopExecution();\nawait new Promise(resolve => setTimeout(resolve, ${delayMs}));\n`;
        }
    }
    return code;
};

// 代码生成:延时代码生成器
Blockly.JavaScript.forBlock["delay_ms"] = function (block) {
    const delay = block.getFieldValue("DELAY");
    const delayMs = Math.round(delay * 1000); // 将秒转换为毫秒
    let code = `checkStopExecution();\nconsole.log(getText("delayMessage").replace("{delay}", ${delay}));\n`;
    if (delayMs > 0) {
        // 对于长时间延时，分段检查停止标志
        if (delayMs > 100) {
            code += `await (async () => {
  const checkInterval = 100; // 每100ms检查一次
  const totalChecks = Math.ceil(${delayMs} / checkInterval);
  for (let i = 0; i < totalChecks; i++) {
    checkStopExecution();
    await new Promise(resolve => setTimeout(resolve, Math.min(checkInterval, ${delayMs} - i * checkInterval)));
  }
})();\n`;
        } else {
            code += `checkStopExecution();\nawait new Promise(resolve => setTimeout(resolve, ${delayMs}));\n`;
        }
    }
    return code;
};

// 代码生成:陀螺仪控制代码生成器
Blockly.JavaScript.forBlock["gyro_control"] = function (block) {
    const state = block.getFieldValue("STATE");
    const value = state === "1" ? "U" : "u";
    const command = encodeCommand("g", [value]);
    return wrapAsyncOperation(`const result = await webRequest("${command}", 5000, true); if (result !== null) console.log(result);`) + '\n';
};

// 代码生成:获取传感器输入代码生成器
Blockly.JavaScript.forBlock["get_sensor_input"] = function (block) {
    var sensor = block.getFieldValue("SENSOR");
    return [
        `(async () => { checkStopExecution(); return parseInt(await webRequest("${sensor}", 5000, true)) || 0; })()`,
        Blockly.JavaScript.ORDER_FUNCTION_CALL,
    ];
};

// 代码生成:发送自定义命令代码生成器
Blockly.JavaScript.forBlock["send_custom_command"] = function (block) {
    const command = Blockly.JavaScript.valueToCode(
        block,
        "COMMAND",
        Blockly.JavaScript.ORDER_ATOMIC
    );
    const delay = block.getFieldValue("DELAY");
    const delayMs = Math.round(delay * 1000);
    let code = wrapAsyncOperation(`const result = await webRequest(${command}, ${LONG_COMMAND_TIMEOUT}, true); if (result !== null) console.log(result);`) + '\n';
    if (delayMs > 0) {
        // 对于长时间延时，分段检查停止标志
        if (delayMs > 100) {
            code += `await (async () => {
  const checkInterval = 100; // 每100ms检查一次
  const totalChecks = Math.ceil(${delayMs} / checkInterval);
  for (let i = 0; i < totalChecks; i++) {
    checkStopExecution();
    await new Promise(resolve => setTimeout(resolve, Math.min(checkInterval, ${delayMs} - i * checkInterval)));
  }
})();\n`;
        } else {
            code += `checkStopExecution();\nawait new Promise(resolve => setTimeout(resolve, ${delayMs}));\n`;
        }
    }
    return code;
};

// 代码生成:控制台输出变量代码生成器
Blockly.JavaScript.forBlock["console_log_variable"] = function (block) {
    const variable =
        Blockly.JavaScript.valueToCode(
            block,
            "VARIABLE",
            Blockly.JavaScript.ORDER_NONE
        ) || '""';
    return `console.log(${variable});\n`;
};

// 代码生成:播放音符代码生成器
Blockly.JavaScript.forBlock["play_note"] = function (block) {
    const note = block.getFieldValue("NOTE");
    const duration = block.getFieldValue("DURATION");
    return wrapAsyncOperation(`const result = await webRequest("b ${note} ${duration}", 5000, true); if (result !== null) console.log(result);`) + '\n';
};

// 代码生成:播放旋律代码生成器
Blockly.JavaScript.forBlock["play_melody"] = function (block) {
    const statements = Blockly.JavaScript.statementToCode(block, "MELODY");
    // 将语句转换为命令字符串
    const params = statements
        .split("\n")
        .filter((line) => line.trim())
        .map((line) => {
            // 从每行代码中提取音符和持续时间
            const match = line.match(/b\s+(\d+)\s+(\d+)/);
            if (match) {
                return [parseInt(`${match[1]}`), parseInt(`${match[2]}`)];
            }
            return [];
        })
        .filter((item) => item.length == 2);
    const cmdParams = params.flat();
    
    // 生成base64编码的实际命令
    let encodeCmd = encodeCommand("B", cmdParams);
    
    // 生成可读的显示格式
    let displayCmd = `B ${cmdParams.join(" ")}`;
    
    const delay = block.getFieldValue("DELAY");
    const delayMs = Math.ceil(delay * 1000);
    let code = wrapAsyncOperation(`const result = await webRequest("${encodeCmd}", ${LONG_COMMAND_TIMEOUT}, true, "${displayCmd}"); if (result !== null) console.log(result);`) + '\n';
    if (delayMs > 0) {
        // 对于长时间延时，分段检查停止标志
        if (delayMs > 100) {
            code += `await (async () => {
  const checkInterval = 100; // 每100ms检查一次
  const totalChecks = Math.ceil(${delayMs} / checkInterval);
  for (let i = 0; i < totalChecks; i++) {
    checkStopExecution();
    await new Promise(resolve => setTimeout(resolve, Math.min(checkInterval, ${delayMs} - i * checkInterval)));
  }
})();\n`;
        } else {
            code += `checkStopExecution();\nawait new Promise(resolve => setTimeout(resolve, ${delayMs}));\n`;
        }
    }
    return code;
};

javascript.javascriptGenerator.forBlock["set_joints_angle_seq"] = function (
    block
) {
    const token = "m";
    const variableText = Blockly.JavaScript.valueToCode(
        block,
        "VARIABLE",
        Blockly.JavaScript.ORDER_ATOMIC
    );
    const delay = block.getFieldValue("DELAY");
    let code = `
checkStopExecution();
await (async function() {
  const command = await encodeMoveCommand("${token}", ${variableText});
  await webRequest(command, ${COMMAND_TIMEOUT_MAX}, true);
  return true;
})()
`
    const delayMs = Math.ceil(delay * 1000);
    if (delayMs > 0) {
        // 对于长时间延时，分段检查停止标志
        if (delayMs > 100) {
            code += `await (async () => {
  const checkInterval = 100; // 每100ms检查一次
  const totalChecks = Math.ceil(${delayMs} / checkInterval);
  for (let i = 0; i < totalChecks; i++) {
    checkStopExecution();
    await new Promise(resolve => setTimeout(resolve, Math.min(checkInterval, ${delayMs} - i * checkInterval)));
  }
})();\n`;
        } else {
            code += `checkStopExecution();\nawait new Promise(resolve => setTimeout(resolve, ${delayMs}));\n`;
        }
    }
    return code;
};

javascript.javascriptGenerator.forBlock["set_joints_angle_sim"] = function (
    block
) {
    const token = "i";
    const delay = block.getFieldValue("DELAY");
    const variableText = Blockly.JavaScript.valueToCode(
        block,
        "VARIABLE",
        Blockly.JavaScript.ORDER_ATOMIC
    );
    let code = `
checkStopExecution();
await (async function() {
  const command = await encodeMoveCommand("${token}", ${variableText});
  await webRequest(command, ${COMMAND_TIMEOUT_MAX}, true);
  return true;
})()
`
    const delayMs = Math.ceil(delay * 1000);
    if (delayMs > 0) {
        // 对于长时间延时，分段检查停止标志
        if (delayMs > 100) {
            code += `await (async () => {
  const checkInterval = 100; // 每100ms检查一次
  const totalChecks = Math.ceil(${delayMs} / checkInterval);
  for (let i = 0; i < totalChecks; i++) {
    checkStopExecution();
    await new Promise(resolve => setTimeout(resolve, Math.min(checkInterval, ${delayMs} - i * checkInterval)));
  }
})();\n`;
        } else {
            code += `checkStopExecution();\nawait new Promise(resolve => setTimeout(resolve, ${delayMs}));\n`;
        }
    }
    return code;
};

javascript.javascriptGenerator.forBlock["set_joints_angle_sim_raw"] = function (
    block
) {
    const token = "L";
    const variableText = Blockly.JavaScript.valueToCode(
        block,
        "VARIABLE",
        Blockly.JavaScript.ORDER_ATOMIC
    );
    const variable = eval(variableText).filter((item) => item !== null);
    if (variable.length == 0) {
        return `console.log("set_joints_angle_sim: variable is empty");\n`;
    } else {
        let angleParams = [];
        if (Array.isArray(variable[0])) {
            // variable is array of [[jointId, angle], [jointId, angle], ...]
            angleParams = variable.flat();
        } else if (Number.isInteger(variable[0])) {
            // variable is array of [jointId, angle, jointId, angle, ...]
            angleParams = variable;
        }

        const delay = block.getFieldValue("DELAY");
        const delayMs = Math.ceil(delay * 1000);
        const command = encodeCommand(token, angleParams);
        let code = wrapAsyncOperation(`const result = await webRequest("${command}", 30000, true); if (result !== null) console.log(result);`) + '\n';
        if (delayMs > 0) {
            // 对于长时间延时，分段检查停止标志
            if (delayMs > 100) {
                code += `await (async () => {
  const checkInterval = 100; // 每100ms检查一次
  const totalChecks = Math.ceil(${delayMs} / checkInterval);
  for (let i = 0; i < totalChecks; i++) {
    checkStopExecution();
    await new Promise(resolve => setTimeout(resolve, Math.min(checkInterval, ${delayMs} - i * checkInterval)));
  }
})();\n`;
            } else {
                code += `checkStopExecution();\nawait new Promise(resolve => setTimeout(resolve, ${delayMs}));\n`;
            }
        }
        return code;
    }
};

javascript.javascriptGenerator.forBlock["joints_angle_frame_raw"] = function (
    block
) {
    const variable = block.getFieldValue("VARIABLE");
    return [`[${variable}]`, Blockly.JavaScript.ORDER_ATOMIC];
};

// 代码生成:设置马达角度代码生成器
javascript.javascriptGenerator.forBlock["set_joint_angle"] = function (block) {
    const variableText = Blockly.JavaScript.valueToCode(
        block,
        "VARIABLE",
        Blockly.JavaScript.ORDER_ATOMIC
    );
    const token = "m";
    let code = `
checkStopExecution();
await (async function() {
  const command = await encodeMoveCommand("${token}", ${variableText});
  await webRequest(command, ${COMMAND_TIMEOUT_MAX}, true);
  return true;
})()
`
    const delay = block.getFieldValue("DELAY");
    const delayMs = Math.ceil(delay * 1000);
    if (delayMs > 0) {
        // 对于长时间延时，分段检查停止标志
        if (delayMs > 100) {
            code += `await (async () => {
  const checkInterval = 100; // 每100ms检查一次
  const totalChecks = Math.ceil(${delayMs} / checkInterval);
  for (let i = 0; i < totalChecks; i++) {
    checkStopExecution();
    await new Promise(resolve => setTimeout(resolve, Math.min(checkInterval, ${delayMs} - i * checkInterval)));
  }
})();\n`;
        } else {
            code += `checkStopExecution();\nawait new Promise(resolve => setTimeout(resolve, ${delayMs}));\n`;
        }
    }
    return code;
};

javascript.javascriptGenerator.forBlock["joint_absolute_angle_value"] =
    function (block) {
        const jointId = block.getFieldValue("JOINT");
        const angle = Blockly.JavaScript.valueToCode(
            block,
            "ANGLE",
            Blockly.JavaScript.ORDER_ATOMIC
        );
        return [`[${jointId}, ${angle}]`, Blockly.JavaScript.ORDER_ATOMIC];
    };


javascript.javascriptGenerator.forBlock["joint_relative_angle_value"] =
    function (block) {
        const jointId = block.getFieldValue("JOINT");
        const angleSign = block.getFieldValue("ANGLE_SIGN");
        const angle = Blockly.JavaScript.valueToCode(
            block,
            "ANGLE",
            Blockly.JavaScript.ORDER_ATOMIC
        );
        return [
            `[${jointId}, ${angleSign}, ${angle}]`,
            Blockly.JavaScript.ORDER_ATOMIC,
        ];
    };

// 代码生成:获取关节角度的代码生成器
javascript.javascriptGenerator.forBlock["get_joint_angle"] = function (block) {
    const jointId = block.getFieldValue("JOINT");
    const command = encodeCommand("j", [jointId]);
    return [
        `(async () => { checkStopExecution(); return parseInt(await webRequest("${command}", 5000, true)) || 0; })()`,
        Blockly.JavaScript.ORDER_FUNCTION_CALL,
    ];
};

// 代码生成:获取所有关节角度的代码生成器
javascript.javascriptGenerator.forBlock["get_all_joint_angles"] = function (
    block
) {
    const command = "j";
    let code = `
await (async function() {
  checkStopExecution();
  const rawResult = await webRequest("${command}", 5000, true);
  const result = parseAllJointsResult(rawResult);
  return result;
})()
`;
    return [code, Blockly.JavaScript.ORDER_FUNCTION_CALL];
};

//机械臂动作积木的代码生成器
javascript.javascriptGenerator.forBlock["arm_action"] = function (block) {
    const cmd = block.getFieldValue("COMMAND");
    const delay = block.getFieldValue("DELAY");
    const delayMs = Math.round(delay * 1000);
    let code = wrapAsyncOperation(`const result = await webRequest("${cmd}", ${LONG_COMMAND_TIMEOUT}, true); if (result !== null) console.log(result);`) + '\n';
    if (delayMs > 0) {
        // 对于长时间延时，分段检查停止标志
        if (delayMs > 100) {
            code += `await (async () => {
  const checkInterval = 100; // 每100ms检查一次
  const totalChecks = Math.ceil(${delayMs} / checkInterval);
  for (let i = 0; i < totalChecks; i++) {
    checkStopExecution();
    await new Promise(resolve => setTimeout(resolve, Math.min(checkInterval, ${delayMs} - i * checkInterval)));
  }
})();\n`;
        } else {
            code += `checkStopExecution();\nawait new Promise(resolve => setTimeout(resolve, ${delayMs}));\n`;
        }
    }
    return code;
};

// 代码生成:执行技能文件
javascript.javascriptGenerator.forBlock["action_skill_file"] = function (
    block
) {
    const filename = block.getFieldValue("FILENAME");
    // 延时单位为秒, 需要转换为毫秒整数
    const delay = parseInt(block.getFieldValue("DELAY") * 1000);
    const skillData = window.uploadedSkills.find(
        (skill) => skill.name === filename
    );
    if (!skillData) {
        return `console.log("Skill file not found: ${filename}");\n`;
    }
    const skillContent = skillData.content;
    const token = skillContent.token;
    const list = skillContent.data.flat();
    const cmd = encodeCommand(token, list);
    let code = wrapAsyncOperation(`const result = await webRequest("${cmd}", ${LONG_COMMAND_TIMEOUT}, true); if (result !== null) console.log(result);`) + '\n';
    if (delay > 0) {
        // 对于长时间延时，分段检查停止标志
        if (delay > 100) {
            code += `await (async () => {
  const checkInterval = 100; // 每100ms检查一次
  const totalChecks = Math.ceil(${delay} / checkInterval);
  for (let i = 0; i < totalChecks; i++) {
    checkStopExecution();
    await new Promise(resolve => setTimeout(resolve, Math.min(checkInterval, ${delay} - i * checkInterval)));
  }
})();\n`;
        } else {
            code += `checkStopExecution();\nawait new Promise(resolve => setTimeout(resolve, ${delay}));\n`;
        }
    }
    return code;
};

// 连接机器人代码生成
javascript.javascriptGenerator.forBlock["make_connection"] = function (block) {
    const ip = block.getFieldValue("IP_ADDRESS");
    return `
try {
  const connectionResult = await makeConnection("${ip}");
  if(connectionResult) {
    deviceIP = "${ip}";
    console.log(getText("connectedToDevice") + deviceIP);
  } else {
    console.log("连接失败，后续操作可能无法正常执行");
  }
} catch (error) {
  console.error("连接错误:", error.message);
}\n`;
};

// 代码生成:设置模拟输出积木
Blockly.JavaScript.forBlock["set_analog_output"] = function (
    block
) {
    const pin = block.getFieldValue("PIN");
    const value = Blockly.JavaScript.valueToCode(block, "VALUE", Blockly.JavaScript.ORDER_ATOMIC) || "128";
    return wrapAsyncOperation(`const analogValue = ${value}; const command = encodeCommand("Wa", ["${pin}", analogValue]); const result = await webRequest(command, 5000, true); if (result !== null) console.log(result);`) + '\n';
};

// 代码生成:设置数字输出的代码
Blockly.JavaScript.forBlock["set_digital_output"] = function (
    block
) {
    const pin = block.getFieldValue("PIN");
    const value = block.getFieldValue("STATE");
    const command = encodeCommand("Wd", [pin, value]);
    return wrapAsyncOperation(`const result = await webRequest("${command}", 5000, true); if (result !== null) console.log(result);`) + '\n';
};

// 代码生成:获取数字输入代码生成器 - 只在showDebug下自动打印
Blockly.JavaScript.forBlock["get_digital_input"] = function (block) {
    const pin = block.getFieldValue("PIN");
    const command = encodeCommand("Rd", [pin]);
    let code = `await (async function() {
    checkStopExecution();
    const rawResult = await webRequest("${command}", 5000, true);
    const result = parseSingleResult(rawResult);
    // 只在showDebug模式下打印结果
    if (typeof showDebug !== 'undefined' && showDebug) {
      console.log(result);
    }
    return result;
  })()`;
    return [code, Blockly.JavaScript.ORDER_FUNCTION_CALL];
};

// 代码生成:获取模拟输入代码生成器 - 只在showDebug下自动打印
Blockly.JavaScript.forBlock["get_analog_input"] = function (block) {
    const pin = block.getFieldValue("PIN");
    const command = encodeCommand("Ra", [pin]);
    let code = `await (async function() {
    checkStopExecution();
    const rawResult = await webRequest("${command}", 5000, true);
    const result = parseSingleResult(rawResult);
    // 只在showDebug模式下打印结果
    if (typeof showDebug !== 'undefined' && showDebug) {
      console.log(result);
    }
    return result;
  })()`;
    return [code, Blockly.JavaScript.ORDER_FUNCTION_CALL];
};

// 代码生成:控制台输入代码生成器
Blockly.JavaScript.forBlock["console_input"] = function (block) {
    const prompt = block.getFieldValue("PROMPT");
    let code = `await (async function() {
    checkStopExecution();
    // 检查是否使用默认提示文本，如果是则使用当前语言的翻译
    const promptText = "${prompt}";
    const finalPrompt = (promptText === getText("consoleInputDefaultPrompt") || 
                        promptText === "Please input:" || 
                        promptText === "请输入:" || 
                        promptText === "入力してください:") ? 
                       getText("consoleInputDefaultPrompt") : promptText;
    const result = await window.consoleInput(finalPrompt);
    return result;
  })()`;
    return [code, Blockly.JavaScript.ORDER_FUNCTION_CALL];
};

// 代码生成:获取超声波传感器距离积木 - 只在showDebug下自动打印
Blockly.JavaScript.forBlock["getUltrasonicDistance"] = function (
    block
) {
    const trPin = block.getFieldValue("TRPIN");
    const ecPinValue = block.getFieldValue("ECPIN");
    const ecPin = ecPinValue === "-1" ? trPin : ecPinValue;
    const command = encodeCommand("XU", [trPin, ecPin]);
    let code = `await (async function() {
    checkStopExecution();
    const rawResult = await webRequest("${command}", 5000, true);
    const result = parseSingleResult(rawResult);
    // 只在showDebug模式下打印结果
    if (typeof showDebug !== 'undefined' && showDebug) {
      console.log(result);
    }
    return result;
  })()`;
    return [code, Blockly.JavaScript.ORDER_FUNCTION_CALL];
};

// 代码生成:读取摄像头坐标积木
// {
//   "type": "event_cam",
//   "x": -20.5,      // 相对于中心点的x偏移
//   "y": 15.0,       // 相对于中心点的y偏移
//   "width": 50,     // 目标宽度
//   "height": 50,    // 目标高度
//   "timestamp": 1234567890
// }
Blockly.JavaScript.forBlock["getCameraCoordinate"] = function (
    block
) {
    let code = `
await (async function() {
  checkStopExecution();
  await webRequest("XC", 5000, true);
  checkStopExecution();
  const rawResult = await webRequest("XCp", 5000, true);
  const result = parseCameraCoordinateResult(rawResult);
    return result;
})()
`;
    return [code, Blockly.JavaScript.ORDER_FUNCTION_CALL];
};

function encodeCommand(token, params) {
    if (token.charCodeAt(0) >= 65 && token.charCodeAt(0) <= 90) {
        // 大写字母开头的指令，直接按字节发送
        let byteArray = [];
        
        // 添加token字符
        for (let i = 0; i < token.length; i++) {
            byteArray.push(token.charCodeAt(i));
        }
        
        // 添加参数
        for (let i = 0; i < params.length; i++) {
            // 保证负数转成补码
            byteArray.push(params[i] & 0xff);
        }
        
        // 大写字母开头的指令在末尾添加'~'字符（ASCII 126）
        byteArray.push(126);
        
        // 返回字节数组标识符和数组
        return "bytes:" + JSON.stringify(byteArray);
    } else {
        // 小写字母开头的指令，按原有方式处理
        if (params.length > 0) {
            return `${token}${params.join(" ")}`;
        } else {
            return token;
        }
    }
}

function decodeCommand(content) {
    // 解码base64编码的命令
    if (content.startsWith("b64:")) {
        const base64Data = content.substring(4); // 去掉"b64:"前缀
        const bufferText = atob(base64Data);
        const buffer = new Uint8Array(bufferText.length);
        for (let i = 0; i < bufferText.length; i++) {
            buffer[i] = bufferText.charCodeAt(i);
        }

        // 读取token（第一个字符）
        const token = bufferText.charAt(0);
        const params = new Int8Array(buffer.buffer, 1, buffer.length - 1);
        return {
            token: token,
            params: params,
        };
    }
    const command = content.split(" ");
    // 如果不是base64编码，返回原始内容
    return {
        token: content.charAt(0),
        params: command.slice(1).map((item) => parseInt(item)),
    };
}

function parseSingleResult(rawResult) {
    // 检查rawResult是否为null或undefined
    if (!rawResult) {
        console.warn('parseSingleResult: rawResult is null or undefined');
        return 0;
    }
    
    // 如果rawResult已经是数字，直接返回
    if (typeof rawResult === 'number') {
        return rawResult;
    }
    
    // 首先尝试提取=号后的数字
    if (typeof rawResult === 'string' && rawResult.includes("=")) {
        const lines = rawResult.split("\\\\n");
        for (let i = 0; i < lines.length; i++) {
            if (lines[i] && lines[i].trim() === "=" && i + 1 < lines.length) {
                const num = parseInt(lines[i + 1].trim());
                if (!isNaN(num)) {
                    return num;
                }
            }
        }
    }

    // 尝试从单行格式中提取数字，如"4094 R"
    if (typeof rawResult === 'string') {
        const words = rawResult.trim().split(/\s+/);
        for (const word of words) {
            const num = parseInt(word);
            if (!isNaN(num)) {
                return num;
            }
        }
    }

    return 0;
}

// 解析摄像头坐标
// =
//-23.00 20.00 size = 42 56
//X
function parseCameraCoordinateResult(rawResult) {
    // 检查rawResult是否为null或undefined
    if (!rawResult) {
        console.warn('parseCameraCoordinateResult: rawResult is null or undefined');
        return [];
    }
    
    const lines = rawResult.split("\n");
    if (lines.length >= 3 && lines[2] && lines[2].includes("X")) {
        const args = lines[0].split("\t");
        const x = parseFloat(args[0]);
        const y = parseFloat(args[1]);
        const width = parseFloat(args[4]);
        const height = parseFloat(args[5]);
        return [x, y, width, height];
    }
    return [];
}

// rawResult is string like "0\t1\t2\t3\t4\t5\t6\t7\t8\t9\t10\t11\t12\t13\t14\t15\t\n0,\t0,\t0,\t0,\t0,\t0,\t0,\t0,\t30,\t30,\t30,\t30,\t30,\t30,\t30,\t30,\t\nj\n"
function parseAllJointsResult(rawResult) {
    // 检查rawResult是否为null或undefined
    if (!rawResult) {
        console.warn('parseAllJointsResult: rawResult is null or undefined');
        return [];
    }
    
    const lines = rawResult.split("\n");
    if (lines.length >= 3 && lines[2] && lines[2].includes("j")) {
        const indexs = lines[0]
            .split("\t")
            .filter((item) => item.length > 0)
            .map((num) => parseInt(num));
        const angles = lines[1]
            .split(",\t")
            .filter((item) => item.length > 0)
            .map((num) => parseInt(num));
        return angles;
    }
    return [];
}

function delay(ms) {
    return new Promise((resolve) => setTimeout(resolve, ms));
}

function generateRelativeMoveSimCode(joints, params) {
    let status = Array.from(joints);
    let joinIndexs = new Set();
    for (let i = 0; i < params.length; i++) {
        const args = params[i];
        if (args.length == 3) {
            const jointId = args[0];
            const angleSign = args[1];
            const angle = args[2];
            const updatedAngle = status[jointId] + angleSign * angle;
            status[jointId] = Math.max(Math.min(updatedAngle, 125), -125);
            joinIndexs.add(jointId);
        } else if (args.length == 2) {
            const jointId = args[0];
            const angle = args[1];
            status[jointId] = angle;
            joinIndexs.add(jointId);
        }
    }
    // map array [angle0, angle1, ...] to [index0, angle0, index1, angle1, ...]
    let result = [];
    joinIndexs.forEach((index) => {
        result.push(index, status[index]);
    });
    return result;
}

function generateRelativeMoveSeqCode(joints, params) {
    let status = Array.from(joints);
    let angleParams = [];
    params.forEach((args) => {
        const jointId = args[0];
        if (args.length == 3) {
            const angleSign = args[1];
            const angle = args[2];
            const updatedAngle = status[jointId] + angleSign * angle;
            status[jointId] = Math.max(Math.min(updatedAngle, 125), -125);
        } else if (args.length == 2) {
            const angle = args[1];
            status[jointId] = angle;
        }
        angleParams.push(jointId, status[jointId]);
    });
    return angleParams;
}

async function encodeMoveCommand(token, params) {
    if (Array.isArray(params) && params.length > 0) {
        let joints = Array(16).fill(0);
        let jointArgs = params.filter((item) => item !== null);
        if (Number.isInteger(jointArgs[0])) {
            jointArgs = [jointArgs];
        }
        const hasRelative = jointArgs.some((item) => item.length == 3);
        if (hasRelative) {
            const rawResult = await webRequest("j", JOINT_QUERY_TIMEOUT, true); // 10 seconds for joint query
            const result = parseAllJointsResult(rawResult);
            joints = result;
        }
        let command = "";
        // m: move seq
        if (token.toLowerCase() == "m") {
            const cmdArgs = generateRelativeMoveSeqCode(joints, jointArgs);
            command = encodeCommand(token, cmdArgs);
        } else {
            const cmdArgs = generateRelativeMoveSimCode(joints, jointArgs);
            command = encodeCommand(token, cmdArgs);
        }
        return command;
    } else {
        return token;
    }
}

// HTTP请求函数，用于在生成的代码中使用 - 仅供模拟测试
function mockwebRequest(ip, command, returnResult = false) {
    // 在命令前添加标识前缀，用于调试，但不改变原始命令行为
    const debugCommand = "[MOCK]" + command;
    // console.log(getText("mockRequest") + `${debugCommand} -> ${ip}`);

    // 针对不同命令返回不同模拟值
    if (returnResult) {
        // 模拟设备型号查询
        if (command === "?") {
            // console.warn(getText("usingMockwebRequest"));
            return "PetoiModel-v1.0";
        }

        // 模拟传感器、数字和模拟输入的响应
        if (
            command.startsWith("Ra") ||
            command.startsWith("Rd") ||
            command.startsWith("i ") ||
            command.includes(" ?")
        ) {
            return "123";
        }
    }

    return returnResult ? "0" : true; // 默认返回值
}

// 循环积木块的代码生成器 - 添加停止检查
Blockly.JavaScript.forBlock["controls_repeat_ext"] = function(block) {
    const repeats = Blockly.JavaScript.valueToCode(block, 'TIMES', Blockly.JavaScript.ORDER_ATOMIC) || '0';
    const branch = Blockly.JavaScript.statementToCode(block, 'DO');
    const code = `
for (let i = 0; i < ${repeats}; i++) {
  await checkStopExecutionInLoop();
  ${branch}
}`;
    return code;
};

Blockly.JavaScript.forBlock["controls_whileUntil"] = function(block) {
    const until = block.getFieldValue('MODE') === 'UNTIL';
    const argument0 = Blockly.JavaScript.valueToCode(block, 'BOOL', Blockly.JavaScript.ORDER_NONE) || 'false';
    const branch = Blockly.JavaScript.statementToCode(block, 'DO');
    const code = `
while (${until ? '!' : ''}(${argument0})) {
  await checkStopExecutionInLoop();
  ${branch}
}`;
    return code;
};

Blockly.JavaScript.forBlock["controls_for"] = function(block) {
    const variable0 = Blockly.JavaScript.variableDB_.getName(block.getFieldValue('VAR'), Blockly.Variables.NAME_TYPE);
    const argument0 = Blockly.JavaScript.valueToCode(block, 'FROM', Blockly.JavaScript.ORDER_NONE) || '0';
    const argument1 = Blockly.JavaScript.valueToCode(block, 'TO', Blockly.JavaScript.ORDER_NONE) || '0';
    const increment = Blockly.JavaScript.valueToCode(block, 'BY', Blockly.JavaScript.ORDER_NONE) || '1';
    const branch = Blockly.JavaScript.statementToCode(block, 'DO');
    const code = `
for (let ${variable0} = ${argument0}; ${variable0} <= ${argument1}; ${variable0} += ${increment}) {
  await checkStopExecutionInLoop();
  ${branch}
}`;
    return code;
};

Blockly.JavaScript.forBlock["controls_forEach"] = function(block) {
    const variable0 = Blockly.JavaScript.variableDB_.getName(block.getFieldValue('VAR'), Blockly.Variables.NAME_TYPE);
    const argument0 = Blockly.JavaScript.valueToCode(block, 'LIST', Blockly.JavaScript.ORDER_NONE) || '[]';
    const branch = Blockly.JavaScript.statementToCode(block, 'DO');
    const code = `
for (const ${variable0} of ${argument0}) {
  await checkStopExecutionInLoop();
  ${branch}
}`;
    return code;
};

// 代码生成:随机数积木块
Blockly.JavaScript.forBlock["math_random"] = function(block) {
    const from = block.getFieldValue("FROM");
    const to = block.getFieldValue("TO");
    const type = block.getFieldValue("TYPE");
    
    let code;
    if (type === "Integer") {
        // 生成整数随机数
        code = `Math.floor(Math.random() * (${to} - ${from} + 1)) + ${from}`;
    } else {
        // 生成小数随机数
        code = `Math.random() * (${to} - ${from}) + ${from}`;
    }
    
    return [code, Blockly.JavaScript.ORDER_FUNCTION_CALL];
};
