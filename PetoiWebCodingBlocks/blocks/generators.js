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
    const delayMs = Math.ceil(delay * 1000);

    let code = `
checkStopExecution();
await (async function() {
  const __from = (typeof serialBuffer === 'string')
    ? serialBuffer.length
    : ((typeof window !== 'undefined' && typeof window.serialBuffer === 'string') ? window.serialBuffer.length : undefined);
  await webRequest("${cmd}", 20000, true);
  // 串口模式时等待完成信号：gait 指令一般以 'k' 作为完成标记
  if (!((typeof window !== 'undefined') && window.petoiClient) && typeof waitForSerialTokenLine === 'function') {
    await waitForSerialTokenLine('k', 20000, __from);
  }
  // 收到 token 的时刻（WiFi：webRequest 在收到 k 后 resolve；串口：waitForSerialTokenLine 已返回）
  if (typeof window !== 'undefined') window.__lastTokenReceivedAt = Date.now();
  return true;
})()
`;

    if (delayMs > 0) {
        code += `await (async () => {
  const __tokenAt = (typeof window !== 'undefined' && typeof window.__lastTokenReceivedAt === 'number') ? window.__lastTokenReceivedAt : Date.now();
  const __endAt = __tokenAt + ${delayMs};
  const __checkInterval = 100;
  while (Date.now() < __endAt) {
    checkStopExecution();
    const __wait = Math.min(__checkInterval, __endAt - Date.now());
    if (__wait > 0) await new Promise(r => setTimeout(r, __wait));
  }
})();\n`;
    }
    return code;
};

// 代码生成:发送姿势动作命令（收到 token 后记录时刻并从该时刻开始延时）
Blockly.JavaScript.forBlock["posture"] = function (block) {
    const cmd = block.getFieldValue("COMMAND");
    const delay = block.getFieldValue("DELAY");
    const delayMs = Math.ceil(delay * 1000);

    let code = `
checkStopExecution();
await (async function() {
  const __from = (typeof serialBuffer === 'string')
    ? serialBuffer.length
    : ((typeof window !== 'undefined' && typeof window.serialBuffer === 'string') ? window.serialBuffer.length : undefined);
  await webRequest("${cmd}", 10000, true);
  // 串口模式时等待完成信号：'k...' 返回 'k'；'d'（rest）返回 'd'
  if (!((typeof window !== 'undefined') && window.petoiClient) && typeof waitForSerialTokenLine === 'function') {
    const _tok = '${cmd}'.charAt(0);
    await waitForSerialTokenLine(_tok, 15000, __from);
  }
  if (typeof window !== 'undefined') window.__lastTokenReceivedAt = Date.now();
  return true;
})()
`;

    if (delayMs > 0) {
        code += `await (async () => {
  const __tokenAt = (typeof window !== 'undefined' && typeof window.__lastTokenReceivedAt === 'number') ? window.__lastTokenReceivedAt : Date.now();
  const __endAt = __tokenAt + ${delayMs};
  const __checkInterval = 100;
  while (Date.now() < __endAt) {
    checkStopExecution();
    const __wait = Math.min(__checkInterval, __endAt - Date.now());
    if (__wait > 0) await new Promise(r => setTimeout(r, __wait));
  }
})();\n`;
    }
    return code;
};

// 代码生成:播放音调列表
Blockly.JavaScript.forBlock["play_tone_list"] = function (block) {
    const toneList = block.getFieldValue("TONE_LIST");
    const delay = block.getFieldValue("DELAY");
    const delayMs = Math.round(delay * 1000);
    
    // 解析音调列表
    const tones = toneList.split(',').map(t => t.trim());
    if (tones.length % 2 !== 0) {
        // 如果音调数量不是偶数，添加一个默认时长
        tones.push('4');
    }
    
    // 构建音调数组：[B, tone1, duration1, tone2, duration2, ..., 126]
    // B的ASCII码是66，结束标记是126
    const toneArray = [66]; // 'B'.charCodeAt(0) = 66
    for (let i = 0; i < tones.length; i += 2) {
        const tone = parseInt(tones[i]) || 0;
        const duration = parseInt(tones[i + 1]) || 4;
        toneArray.push(tone, duration);
    }
    toneArray.push(126); // 结束标记
    
    // 使用字节数组格式，但添加更好的错误处理
    const command = `bytes:[${toneArray.join(',')}]`;
    let code = wrapAsyncOperation(`
        // 串口等待：记录发送前缓冲区长度，避免命中旧 token
        if (typeof window !== 'undefined') {
          const __sb = (typeof serialBuffer === 'string')
            ? serialBuffer
            : ((typeof window.serialBuffer === 'string') ? window.serialBuffer : '');
          window.__lastSerialStartIndex = __sb.length;
        }
        try {
            const result = await webRequest("${command}", 15000, true);
        } catch (error) {
            console.error(getText("debugToneListSendFailed"), error);
            // 如果字节数组发送失败，尝试逐个发送音符
            ${generateFallbackNotes(tones)}
        }
    `) + '\n';
    // 串口模式：等到串口回 'B'（音调列表完成）后，再开始计时延时
    code += `if (!((typeof window !== 'undefined') && window.petoiClient) && typeof waitForSerialTokenLine === 'function') {
  const __from = (typeof window !== 'undefined' && typeof window.__lastSerialStartIndex === 'number') ? window.__lastSerialStartIndex : undefined;
  await waitForSerialTokenLine('B', 15000, __from);
  if (typeof window !== 'undefined') window.__lastSerialStartIndex = null;
}\n`;
    
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

// 生成备用音符发送代码的辅助函数
function generateFallbackNotes(tones) {
    let fallbackCode = '';
    for (let i = 0; i < tones.length; i += 2) {
        const tone = parseInt(tones[i]) || 0;
        const duration = parseInt(tones[i + 1]) || 4;
        fallbackCode += `await webRequest("b ${tone} ${duration}", 5000, true);
            `;
    }
    return fallbackCode;
}

// 代码生成:发送杂技动作命令（收到 token 后记录时刻并从该时刻开始延时）
Blockly.JavaScript.forBlock["acrobatic_moves"] = function (block) {
    const cmd = block.getFieldValue("COMMAND");
    const delay = block.getFieldValue("DELAY");
    const delayMs = Math.ceil(delay * 1000);

    let code = `
checkStopExecution();
await (async function() {
  const __from = (typeof serialBuffer === 'string')
    ? serialBuffer.length
    : ((typeof window !== 'undefined' && typeof window.serialBuffer === 'string') ? window.serialBuffer.length : undefined);
  await webRequest("${cmd}", ${ACROBATIC_MOVES_TIMEOUT}, true);
  if (!((typeof window !== 'undefined') && window.petoiClient) && typeof waitForSerialTokenLine === 'function') {
    await waitForSerialTokenLine('k', ${ACROBATIC_MOVES_TIMEOUT}, __from);
  }
  if (typeof window !== 'undefined') window.__lastTokenReceivedAt = Date.now();
  return true;
})()
`;

    if (delayMs > 0) {
        code += `await (async () => {
  const __tokenAt = (typeof window !== 'undefined' && typeof window.__lastTokenReceivedAt === 'number') ? window.__lastTokenReceivedAt : Date.now();
  const __endAt = __tokenAt + ${delayMs};
  const __checkInterval = 100;
  while (Date.now() < __endAt) {
    checkStopExecution();
    const __wait = Math.min(__checkInterval, __endAt - Date.now());
    if (__wait > 0) await new Promise(r => setTimeout(r, __wait));
  }
})();\n`;
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
    const value = state === "1" ? "B" : "b";
    const command = encodeCommand("g", [value]);
    return wrapAsyncOperation(`const result = await webRequest("${command}", 5000, true);`) + '\n';
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
    let code = wrapAsyncOperation(`
      if (typeof window !== 'undefined') {
        const __sb = (typeof serialBuffer === 'string')
          ? serialBuffer
          : ((typeof window.serialBuffer === 'string') ? window.serialBuffer : '');
        window.__lastSerialStartIndex = __sb.length;
      }
      const result = await webRequest(${command}, ${LONG_COMMAND_TIMEOUT}, true);
    `) + '\n';
    // 若自定义命令是 'm'/'k'/'d' 开头，串口模式下等待对应完成标记；否则跳过
    code += `if (!((typeof window !== 'undefined') && window.petoiClient) && typeof waitForSerialTokenLine === 'function') { try { 
  const _c = ${command}; 
  const _t = (typeof _c === 'string' && _c.length>0) ? _c[0] : null; 
  if (_t && ('mkd'.includes(_t))) { 
    const __from = (typeof window !== 'undefined' && typeof window.__lastSerialStartIndex === 'number') ? window.__lastSerialStartIndex : undefined;
    await waitForSerialTokenLine(_t, ${LONG_COMMAND_TIMEOUT}, __from); 
  } 
  if (typeof window !== 'undefined') window.__lastSerialStartIndex = null;
} catch(e) {} }\n`;
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
    const trimmed = ("" + variable).trim();
    if (/\.length\s*$/.test(trimmed)) {
        const baseExpr = trimmed.replace(/\.length\s*$/, "");
        return `await (async () => {\nconst __baseTmp = ${baseExpr};\nconst __baseVal = (__baseTmp && typeof __baseTmp.then === 'function') ? await __baseTmp : __baseTmp;\nconst __len = (Array.isArray(__baseVal) || typeof __baseVal === 'string') ? __baseVal.length : 0;\nconsole.log(__len);\n})();\n`;
    }
    return `await (async () => {\nconst __tmp = ${variable};\nconst __val = (__tmp && typeof __tmp.then === 'function') ? await __tmp : __tmp;\nif (Array.isArray(__val)) {\n  const __text = __val.length === 0\n    ? '[ ]'\n    : '[' + __val.map(v => {\n        if (typeof v === 'number') return Number.isInteger(v) ? String(v) : v.toFixed(2);\n        return String(v);\n      }).join(',') + ']';\n  console.log(__text);\n} else {\n  console.log(__val);\n}\n})();\n`;
};

// 代码生成:播放音符代码生成器
Blockly.JavaScript.forBlock["play_note"] = function (block) {
    const note = block.getFieldValue("NOTE");
    const duration = block.getFieldValue("DURATION");
    return wrapAsyncOperation(`const result = await webRequest("b ${note} ${duration}", 5000, true);`) + '\n';
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
    let code = wrapAsyncOperation(`
      if (typeof window !== 'undefined') {
        const __sb = (typeof serialBuffer === 'string')
          ? serialBuffer
          : ((typeof window.serialBuffer === 'string') ? window.serialBuffer : '');
        window.__lastSerialStartIndex = __sb.length;
      }
      const result = await webRequest("${encodeCmd}", ${LONG_COMMAND_TIMEOUT}, true, "${displayCmd}");
    `) + '\n';
    // 串口模式：等到串口回 'B'（旋律完成）后，再开始计时延时
    code += `if (!((typeof window !== 'undefined') && window.petoiClient) && typeof waitForSerialTokenLine === 'function') { 
  const __from = (typeof window !== 'undefined' && typeof window.__lastSerialStartIndex === 'number') ? window.__lastSerialStartIndex : undefined;
  await waitForSerialTokenLine('B', ${LONG_COMMAND_TIMEOUT}, __from); 
  if (typeof window !== 'undefined') window.__lastSerialStartIndex = null;
}\n`;
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
  const __from = (typeof serialBuffer === 'string') ? serialBuffer.length : ((typeof window !== 'undefined' && typeof window.serialBuffer === 'string') ? window.serialBuffer.length : undefined);
  const command = await encodeMoveCommand("${token}", ${variableText});
  await webRequest(command, ${COMMAND_TIMEOUT_MAX}, true);
  if (!((typeof window !== 'undefined') && window.petoiClient) && typeof waitForSerialTokenLine === 'function') {
    await waitForSerialTokenLine('m', 15000, __from);
  }
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
  const __from = (typeof serialBuffer === 'string') ? serialBuffer.length : ((typeof window !== 'undefined' && typeof window.serialBuffer === 'string') ? window.serialBuffer.length : undefined);
  const command = await encodeMoveCommand("${token}", ${variableText});
  await webRequest(command, ${COMMAND_TIMEOUT_MAX}, true);
  if (!((typeof window !== 'undefined') && window.petoiClient) && typeof waitForSerialTokenLine === 'function') {
    await waitForSerialTokenLine('i', 30000, __from);
  }
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
        let code = wrapAsyncOperation(`const result = await webRequest("${command}", 30000, true);`) + '\n';
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
  const __from = (typeof serialBuffer === 'string') ? serialBuffer.length : ((typeof window !== 'undefined' && typeof window.serialBuffer === 'string') ? window.serialBuffer.length : undefined);
  const command = await encodeMoveCommand("${token}", ${variableText});
  await webRequest(command, ${COMMAND_TIMEOUT_MAX}, true);
  if (!((typeof window !== 'undefined') && window.petoiClient) && typeof waitForSerialTokenLine === 'function') {
    await waitForSerialTokenLine('m', 15000, __from);
  }
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

//机械臂动作积木的代码生成器（收到 token 后记录时刻并从该时刻开始延时）
javascript.javascriptGenerator.forBlock["arm_action"] = function (block) {
    const cmd = block.getFieldValue("COMMAND");
    const delay = block.getFieldValue("DELAY");
    const delayMs = Math.ceil(delay * 1000);

    let code = `
checkStopExecution();
await (async function() {
  const __from = (typeof serialBuffer === 'string')
    ? serialBuffer.length
    : ((typeof window !== 'undefined' && typeof window.serialBuffer === 'string') ? window.serialBuffer.length : undefined);
  await webRequest("${cmd}", ${LONG_COMMAND_TIMEOUT}, true);
  if (!((typeof window !== 'undefined') && window.petoiClient) && typeof waitForSerialTokenLine === 'function') {
    const _tok = '${cmd}'.charAt(0);
    await waitForSerialTokenLine(_tok, ${LONG_COMMAND_TIMEOUT}, __from);
  }
  if (typeof window !== 'undefined') window.__lastTokenReceivedAt = Date.now();
  return true;
})()
`;

    if (delayMs > 0) {
        code += `await (async () => {
  const __tokenAt = (typeof window !== 'undefined' && typeof window.__lastTokenReceivedAt === 'number') ? window.__lastTokenReceivedAt : Date.now();
  const __endAt = __tokenAt + ${delayMs};
  const __checkInterval = 100;
  while (Date.now() < __endAt) {
    checkStopExecution();
    const __wait = Math.min(__checkInterval, __endAt - Date.now());
    if (__wait > 0) await new Promise(r => setTimeout(r, __wait));
  }
})();\n`;
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
    let code = wrapAsyncOperation(`
      if (typeof window !== 'undefined') {
        const __sb = (typeof serialBuffer === 'string')
          ? serialBuffer
          : ((typeof window.serialBuffer === 'string') ? window.serialBuffer : '');
        window.__lastSerialStartIndex = __sb.length;
      }
      const result = await webRequest("${cmd}", ${LONG_COMMAND_TIMEOUT}, true);
    `) + '\n';
    // 串口模式：根据技能文件的token类型等待对应完成标记
    code += `if (!((typeof window !== 'undefined') && window.petoiClient) && typeof waitForSerialTokenLine === 'function') { 
  const __from = (typeof window !== 'undefined' && typeof window.__lastSerialStartIndex === 'number') ? window.__lastSerialStartIndex : undefined;
  await waitForSerialTokenLine('${token}', ${LONG_COMMAND_TIMEOUT}, __from); 
  if (typeof window !== 'undefined') window.__lastSerialStartIndex = null;
}\n`;
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
    window.__gesturePollIntervalMs = 600;
  } else {
    console.log(getText("debugConnectionFailed"));
  }
} catch (error) {
  console.error(getText("debugConnectionError"), error.message);
}\n`;
};

// 代码生成:设置模拟输出积木
Blockly.JavaScript.forBlock["set_analog_output"] = function (
    block
) {
    const pin = block.getFieldValue("PIN");
    const value = Blockly.JavaScript.valueToCode(block, "VALUE", Blockly.JavaScript.ORDER_ATOMIC) || "128";
    return wrapAsyncOperation(`const analogValue = ${value}; const command = encodeCommand("Wa", ["${pin}", analogValue]); const result = await webRequest(command, 5000, true);`) + '\n';
};

// 代码生成:设置数字输出的代码
Blockly.JavaScript.forBlock["set_digital_output"] = function (
    block
) {
    const pin = block.getFieldValue("PIN");
    const value = block.getFieldValue("STATE");
    const command = encodeCommand("Wd", [pin, value]);
    return wrapAsyncOperation(`const result = await webRequest("${command}", 5000, true);`) + '\n';
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

// 代码生成:通用输入积木（digital/analog下拉菜单 + 引脚数字输入框）
Blockly.JavaScript.forBlock["get_input"] = function (block) {
    const type = block.getFieldValue("TYPE");
    const pin = block.getFieldValue("PIN");
    const commandType = type === "digital" ? "Rd" : "Ra";
    const command = encodeCommand(commandType, [pin]);
    let code = `await (async function() {
    checkStopExecution();
    const rawResult = await webRequest("${command}", 5000, true);
    let result = parseSingleResult(rawResult);
    // 对于数字输入，非零数字自动转化为1（或true）
    if ("${type}" === "digital") {
      result = result !== 0 ? 1 : 0;
    }
    // 只在showDebug模式下打印结果
    if (typeof showDebug !== 'undefined' && showDebug) {
      console.log(result);
    }
    return result;
  })()`;
    return [code, Blockly.JavaScript.ORDER_FUNCTION_CALL];
};

// 代码生成:通用输出积木（digital/analog下拉菜单 + 引脚数字输入框 + Value数字输入框）
Blockly.JavaScript.forBlock["set_output"] = function (block) {
    const type = block.getFieldValue("TYPE");
    const pin = block.getFieldValue("PIN");
    const valueCode = Blockly.JavaScript.valueToCode(block, "VALUE", Blockly.JavaScript.ORDER_ATOMIC) || "0";
    const commandType = type === "digital" ? "Wd" : "Wa";
    // 对于数字输出，非零数字自动转化为1（在运行时处理）
    if (type === "digital") {
        return wrapAsyncOperation(`const value = ${valueCode}; const digitalValue = value !== 0 ? 1 : 0; const command = encodeCommand("${commandType}", ["${pin}", digitalValue]); const result = await webRequest(command, 5000, true);`) + '\n';
    } else {
        return wrapAsyncOperation(`const analogValue = ${valueCode}; const command = encodeCommand("${commandType}", ["${pin}", analogValue]); const result = await webRequest(command, 5000, true);`) + '\n';
    }
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
  // 仅在第一次获取坐标前激活相机
  if (typeof window === 'undefined' || !window.__cameraActivated) {
    await webRequest("XCr", 5000, true);
    if (typeof window !== 'undefined') window.__cameraActivated = true;
  }
  checkStopExecution();
  // 首次仅发送一次获取命令，串口返回时由串口读取钩子即时镜像坐标
  if (typeof window === 'undefined' || !window.__cameraPolled) {
    await webRequest("XCP", 5000, true);
    if (typeof window !== 'undefined') window.__cameraPolled = true;
  }
  // 仅在检测到“新的一帧”坐标时返回；否则返回空数组
  const beforeKey = (typeof window !== 'undefined' && window.__lastCameraFrameKey) ? window.__lastCameraFrameKey : '';
  if (typeof window !== 'undefined') { window.__cameraQueryActive = true; window.__mirrorCameraToConsole = false; window.__cameraMirrorDone = false; }
  let coords = [];
  try {
    if (typeof waitForNewCameraCoordinates === 'function') {
      coords = await waitForNewCameraCoordinates(beforeKey, 1000);
    } else {
      coords = [];
    }
  } finally {
    if (typeof window !== 'undefined') { window.__cameraQueryActive = false; window.__mirrorCameraToConsole = false; }
  }
  if (Array.isArray(coords) && coords.length === 4) {
    // 若串口捕获到了该帧的到达时间戳，则用该时间戳在Console先行对齐打印一次
    if (typeof window !== 'undefined' && typeof addConsoleMessageAt === 'function' && window.__lastCameraTs && window.__lastCameraCoords) {
      try { addConsoleMessageAt([[String(coords[0]), String(coords[1]), String(coords[2]), String(coords[3])].join(',')], window.__lastCameraTs); } catch (e) {}
      window.__lastCameraTs = null; window.__lastCameraCoords = null;
    }
    return coords;
  }
  return [];
})()
`;
    return [code, Blockly.JavaScript.ORDER_FUNCTION_CALL];
};

// 新增：手势传感器读取积木的代码生成器
Blockly.JavaScript.forBlock["get_gesture_value"] = function (block) {
    let code = `
await (async function() {
  checkStopExecution();
  // 首次进入手势模式（WiFi 下设备可能响应慢或格式不同，失败不中断任务，仍标记已尝试并继续轮询 XGp）
  if (typeof window === 'undefined' || !window.__gestureActivated) {
    try {
      // 启用手势传感器，但禁用固件自动反应（使用小写 r）
      // XGr~（小写 r + 终止符）= 禁用固件自动反应，让 Blockly 程序控制动作
      // XGR~（大写 R + 终止符）= 启用固件自动反应，固件会自动执行内置动作序列
      // 注意：大写字母命令必须以 ~ 结尾
      await webRequest("XGr~", 5000, true);
      // 等待固件设置生效
      await new Promise(r => setTimeout(r, 200));
    } catch (e) {
      if (typeof console !== 'undefined' && console.warn) console.warn('XGr~ (gesture enable) failed, continuing with XGp poll:', e && e.message ? e.message : e);
    }
    if (typeof window !== 'undefined') window.__gestureActivated = true;
  }
  checkStopExecution();

  // 每次都发送单次查询命令 XGp（小写p），获取当前手势值
  // 注意：WiFi(WebSocket)模式下，手势值会直接作为 webRequest 返回字符串返回；
  // 串口模式下，也可能返回完整响应或占位文本，此时再回退到 serialBuffer 解析。
  //
  // 性能优化（WiFi模式尤其重要）：
  // - 鑺傛祦锛氶粯璁?500ms锛圵iFi 妯″紡闇€杈冮暱闂撮殧閬垮厤闃熷垪绉帇锛夛紱鑺傛祦鏈熼棿绛夊緟骞惰繑鍥?-1锛岄伩鍏嶉噸澶嶄娇鐢ㄧ紦瀛樻墜鍔縗n  // - 瓒呮椂锛歑Gp 缁熶竴 800ms锛堢粰璁惧瓒冲鐨勫鐞嗘椂闂达級
  // - 鍗曢淇濇姢锛氬悓涓€鏃跺埢鍙厑璁镐竴涓?XGp 鍦ㄩ€旓紝淇濇姢鏈熼棿绛夊緟 100ms 鍚庤繑鍥?-1
  // - 鍔ㄤ綔閿侊細鍔ㄤ綔鎵ц鏈熼棿绛夊緟 100ms 鍚庤繑鍥?-1锛岄伩鍏嶅悓涓€鎵嬪娍閲嶅瑙﹀彂
  if (typeof window !== 'undefined') {
    const __now = Date.now();
    
    // 优化策略：允许在动作执行期间检测手势，但会在动作积木中阻止执行新动作
    // 这样可以在动作完成后立即响应下一个手势，提高实时性
    // 注意：不再在这里返回 -1，让手势检测继续进行
    
    const __minInterval = (typeof window.__gesturePollIntervalMs === 'number')
      ? window.__gesturePollIntervalMs
      : 500;
    if (typeof window.__lastGesturePollTs !== 'number') {
      window.__lastGesturePollTs = 0;
    }
    if (__now - window.__lastGesturePollTs < __minInterval) {
      // 节流期间：等待剩余时间，然后返回 -1（避免重复使用缓存的手势值）
      const __waitTime = __minInterval - (__now - window.__lastGesturePollTs);
      if (__waitTime > 0 && __waitTime < 10000) {
        await new Promise(r => setTimeout(r, __waitTime));
      }
      return -1;
    }
    if (window.__gesturePollInFlight) {
      // 单飞保护：等待 100ms 后返回 -1
      await new Promise(r => setTimeout(r, 100));
      return -1;
    }
    window.__gesturePollInFlight = true;
    window.__lastGesturePollTs = __now;
  }

  let rawGesture = null;
  const __xgpTimeout = 1000;
  try {
    // XGp~（大写字母命令需要 ~ 终止符，小写 p = 单次查询手势值）
    rawGesture = await webRequest("XGp~", __xgpTimeout, true);
  } catch (e) {
    // 关键：XGp 偶发超时/断连时不要让程序整体中断，
    // 只把这次读数当作“无手势”，让循环继续，后续 exit_gesture_mode 才有机会执行。
    rawGesture = null;
  } finally {
    if (typeof window !== 'undefined') {
      window.__gesturePollInFlight = false;
    }
  }

  // 解析返回的手势值（优先解析 rawGesture；必要时回退到缓冲区解析）
  let gestureValue = -1;
  try {
    // WiFi 下若注入的 webRequest 未展开数组，此处做兼容：单元素数组取首元素
    if (Array.isArray(rawGesture) && rawGesture.length > 0) {
      rawGesture = rawGesture[0];
    }
    // 1) 优先：从 webRequest 的返回值解析（WiFi模式必走这里）
    if (typeof rawGesture === 'number') {
      gestureValue = rawGesture;
    } else if (typeof rawGesture === 'string' && rawGesture.length > 0) {
      // parseSingleResult 会从字符串中提取首个可解析数字（WiFi 下设备返回 "=\\r\\n数字\\r\\n" 格式）
      if (typeof parseSingleResult === 'function') {
        gestureValue = parseSingleResult(rawGesture);
      } else {
        // 这里用 \\d，避免某些拼接/转义流程把 \d 吃掉导致正则失效
        const m = String(rawGesture).match(/-?\\d+/);
        gestureValue = m ? parseInt(m[0], 10) : -1;
      }
    }

    // 2) 回退：仅串口模式从缓冲区取最新一帧；WiFi 无缓冲区，不等待 200ms 以保持实时性
    const __hasSerialBuffer = (typeof serialBuffer !== 'undefined' && typeof serialBuffer === 'string') || (typeof window !== 'undefined' && typeof window.serialBuffer === 'string');
    if (![0, 1, 2, 3].includes(gestureValue) && __hasSerialBuffer) {
      if (typeof serialBuffer !== 'undefined' && typeof serialBuffer === 'string' && serialBuffer.length > 5000) {
        serialBuffer = serialBuffer.substring(serialBuffer.length - 3000);
      } else if (typeof window !== 'undefined' && typeof window.serialBuffer === 'string' && window.serialBuffer.length > 5000) {
        window.serialBuffer = window.serialBuffer.substring(window.serialBuffer.length - 3000);
      }
      await new Promise(r => setTimeout(r, 200));
      if (typeof getLatestGestureNoWait === 'function') {
        const result = getLatestGestureNoWait();
        if (result && result.value !== null && result.value !== undefined) {
          gestureValue = result.value;
        }
      }
    }
  } catch (e) {
    gestureValue = -1;
  }

  // 只接受 0/1/2/3，其它一律视为无手势
  if (![0, 1, 2, 3].includes(gestureValue)) {
    gestureValue = -1;
  }

  // 缓存上一帧（让WiFi下“跳过轮询”时也能有值可用）
  if (typeof window !== 'undefined') {
    window.__lastGestureValue = gestureValue;
    window.__lastGestureValueTs = Date.now();
  }

  // 返回手势值（0=上, 1=下, 2=左, 3=右, -1=无手势）
  return gestureValue;
})()
`;
    return [code, Blockly.JavaScript.ORDER_FUNCTION_CALL];
};

// 新增：退出手势识别模式积木的代码生成器
Blockly.JavaScript.forBlock["exit_gesture_mode"] = function (block) {
    let code = `
await (async function() {
  checkStopExecution();
  
  // 辅助函数：等待特定响应（用于WebSocket和串口通信）
  async function waitForResponse(expectedValue, timeout = 5000) {
    const startTime = Date.now();
    const checkInterval = 50; // 每50ms检查一次
    
    return new Promise((resolve, reject) => {
      const checkResponse = () => {
        checkStopExecution();
        
        if (Date.now() - startTime > timeout) {
          reject(new Error('等待响应超时'));
          return;
        }
        
        // 获取缓冲区数据（支持WebSocket和串口）
        let buffer = '';
        if (typeof window !== 'undefined' && window.__isSerialMode) {
          // 串口模式
          if (typeof serialBuffer !== 'undefined' && typeof serialBuffer === 'string') {
            buffer = serialBuffer;
          } else if (typeof window !== 'undefined' && typeof window.serialBuffer === 'string') {
            buffer = window.serialBuffer;
          }
        } else {
          // WebSocket模式
          if (typeof serialBuffer !== 'undefined' && typeof serialBuffer === 'string') {
            buffer = serialBuffer;
          }
        }
        
        if (buffer) {
          // 按行分割，查找匹配的响应
          const lines = buffer.split(/[\\r\\n]+/).map(l => l.trim());
          for (let i = lines.length - 1; i >= 0; i--) {
            if (lines[i] === expectedValue) {
              resolve(expectedValue);
              return;
            }
          }
        }
        
        // 继续等待
        setTimeout(checkResponse, checkInterval);
      };
      
      checkResponse();
    });
  }
  
  try {
    // 发送Xg~指令退出手势识别模式（大写字母命令需要 ~ 终止符）
    let response = await webRequest("Xg~", 5000, true);
    await new Promise(resolve => setTimeout(resolve, 100)); // 等待100ms
    
    // 检查响应（WebSocket模式可能直接返回，串口模式需要从缓冲区读取）
    let foundX = false;
    if (response && (typeof response === 'string' || typeof response === 'number')) {
      const responseStr = String(response);
      if (responseStr.includes('X') || responseStr.trim() === 'X') {
        foundX = true;
      }
    }
    
    // 如果WebSocket模式没找到，或者使用串口模式，从缓冲区等待"X"
    if (!foundX) {
      try {
        await waitForResponse('X', 3000);
      } catch (e) {
        // 如果超时，继续执行（可能已经收到响应但没检测到）
      }
    }
    
    // 重置初始化标志，以便下次可以重新初始化
    if (typeof window !== 'undefined') {
      // 统一使用 __gestureActivated（get_gesture_value 里用的就是它）
      window.__gestureActivated = false;
      window.__lastGestureFrameKey = '';
    }
  } catch (error) {
    console.error('退出手势识别模式失败:', error);
  }
})();
`;
    return code;
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
    
    // 首先尝试提取=号后的数字（支持设备返回 "=\r\n数字\r\n" 或 "=\n数字\n" 等格式）
    if (typeof rawResult === 'string' && rawResult.includes("=")) {
        const lines = rawResult.split(/\r?\n/).map(function (s) { return s.trim(); });
        for (let i = 0; i < lines.length; i++) {
            if (lines[i] === "=" && i + 1 < lines.length) {
                const num = parseInt(lines[i + 1], 10);
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
    // 内部通用解析：支持两种格式
    // 1) 旧格式（行0为坐标，行2为'X'，使用Tab分隔）
    // 2) 新格式（行0为'=', 行1为"x y size = w h"，行2为'X'）
    function extractFromText(text) {
        if (!text) return [];
        const norm = String(text).replace(/\r\n/g, "\n");
        const lines = norm.split("\n").map(l => l.trim()).filter(l => l.length > 0);

        // 优先匹配新格式块：
        // =\n坐标行\nX
        // 其中坐标行形如 "-65.00 -2.00 size = 97 138"
        // 取最后一帧匹配（避免切片里有多帧时总拿到旧帧）
        const blockRegex = /=\s*\n([^\n]+)\nX/gi;
        let blockMatch = null;
        let lastMatch = null;
        while ((blockMatch = blockRegex.exec(norm)) !== null) {
            lastMatch = blockMatch;
        }
        if (lastMatch && lastMatch[1]) {
            const mid = lastMatch[1];
            const coordsRegex = /(-?\d+(?:\.\d+)?)\s+(-?\d+(?:\.\d+)?)\s+size\s*=\s*(\d+)\s+(\d+)/i;
            const m = mid.match(coordsRegex);
            if (m) {
                const x = parseFloat(m[1]);
                const y = parseFloat(m[2]);
                const w = parseFloat(m[3]);
                const h = parseFloat(m[4]);
                if ([x, y, w, h].every(v => !Number.isNaN(v))) {
                    return [x, y, w, h];
                }
            }
        }

        // 回退匹配旧格式：行2含X，坐标在行0（Tab分隔，索引0、1、4、5）
        if (lines.length >= 3 && /x/i.test(lines[2])) {
            const args = lines[0].split(/\t+/);
            if (args.length >= 6) {
                const x = parseFloat(args[0]);
                const y = parseFloat(args[1]);
                const width = parseFloat(args[4]);
                const height = parseFloat(args[5]);
                if ([x, y, width, height].every(v => !Number.isNaN(v))) {
                    return [x, y, width, height];
                }
            }
        }
        return [];
    }

    // 1) 尝试解析传入的 rawResult（WebSocket 路径通常返回完整文本）
    let parsed = extractFromText(rawResult);
    if (parsed.length === 4) return parsed;

    // 2) 串口路径下，webRequest("XCP") 可能返回占位文本（如"Command sent via serial"）。
    //    此时从全局串口缓冲区中回退解析最新一帧坐标块（优先使用全局绑定 serialBuffer，其次 window.serialBuffer）。
    try {
        let buf = '';
        if (typeof serialBuffer !== 'undefined' && typeof serialBuffer === 'string') {
            buf = serialBuffer;
        } else if (typeof window !== 'undefined' && typeof window.serialBuffer === 'string') {
            buf = window.serialBuffer;
        }
        if (buf && buf.length > 0) {
            // 仅使用缓冲区末尾部分以提高命中率与性能
            const tail = buf.slice(-2000);
            parsed = extractFromText(tail);
            if (parsed.length === 4) return parsed;
        }
    } catch (e) {
        // 忽略回退解析中的异常
    }

    // 3) 仍未解析到有效数据
    return [];
}

// 轮询等待串口缓冲区中出现一帧坐标数据（= / coords / X）
async function waitForCameraCoordinates(timeoutMs = 1000) {
    const start = Date.now();
    while (Date.now() - start < timeoutMs) {
        const parsed = parseCameraCoordinateResult("");
        if (Array.isArray(parsed) && parsed.length === 4) {
            return parsed;
        }
        await new Promise(r => setTimeout(r, 50));
    }
    return [];
}

// 从串口缓冲区提取最新一帧坐标（尽可能使用最后一帧，避免拿到旧帧）
function getLatestCameraCoordinatesNoWait() {
    try {
        let buf = '';
        if (typeof serialBuffer !== 'undefined' && typeof serialBuffer === 'string') {
            buf = serialBuffer;
        } else if (typeof window !== 'undefined' && typeof window.serialBuffer === 'string') {
            buf = window.serialBuffer;
        }
        if (!buf) return { coords: [], key: '' };

        const norm = String(buf).replace(/\r\n/g, "\n");
        // 优先：三行帧，以 X 为结束标记
        let lastXMatch = null;
        const xRegex = /(^|\n)X(\n|$)/g;
        let m;
        while ((m = xRegex.exec(norm)) !== null) {
            lastXMatch = { index: m.index + (m[1] ? m[1].length : 0) };
        }
        if (lastXMatch) {
            const xIndex = lastXMatch.index;
            const coordsEnd = xIndex; // 坐标行在 X 前一行
            const coordsStart = norm.lastIndexOf('\n', coordsEnd - 1) + 1;
            if (!(coordsStart < 0 || coordsStart >= coordsEnd)) {
                const coordsLine = norm.substring(coordsStart, coordsEnd).trim();
                const eqEnd = coordsStart - 1;
                const eqStart = norm.lastIndexOf('\n', eqEnd - 1) + 1;
                const eqLine = eqStart >= 0 ? norm.substring(eqStart, eqEnd).trim() : '';
                const coordsRegex = /(-?\d+(?:\.\d+)?)\s+(-?\d+(?:\.\d+)?)\s+size\s*=\s*(\d+)\s+(\d+)/i;
                const c = coordsLine.match(coordsRegex);
                if (c) {
                    const x = parseFloat(c[1]);
                    const y = parseFloat(c[2]);
                    const w = parseFloat(c[3]);
                    const h = parseFloat(c[4]);
                    if ([x, y, w, h].every(v => !Number.isNaN(v))) {
                        const key = `${eqLine}|${coordsLine}|X@${coordsStart}`;
                        return { coords: [x, y, w, h], key };
                    }
                }
            }
        }
        // 回退：支持单行坐标（无 X 标记）。取最后一次匹配的行
        const coordsRegexGlobal = /(-?\d+(?:\.\d+)?)\s+(-?\d+(?:\.\d+)?)\s+size\s*=\s*(\d+)\s+(\d+)/gi;
        let match, last = null;
        while ((match = coordsRegexGlobal.exec(norm)) !== null) {
            last = { match, index: match.index };
        }
        if (last) {
            const x = parseFloat(last.match[1]);
            const y = parseFloat(last.match[2]);
            const w = parseFloat(last.match[3]);
            const h = parseFloat(last.match[4]);
            if ([x, y, w, h].every(v => !Number.isNaN(v))) {
                // 以匹配到的行文本和其起始位置作为key，避免与旧帧混淆
                const line = last.match[0];
                const key = `${line}@${last.index}`;
                return { coords: [x, y, w, h], key };
            }
        }
        return { coords: [], key: '' };
    } catch (e) {
        return { coords: [], key: '' };
    }
}

async function waitForNewCameraCoordinates(prevKey, timeoutMs = 500) {
    const start = Date.now();
    while (Date.now() - start < timeoutMs) {
        const { coords, key } = getLatestCameraCoordinatesNoWait();
        if (key && key !== prevKey && Array.isArray(coords) && coords.length === 4) {
            if (typeof window !== 'undefined') {
                window.__lastCameraFrameKey = key;
            }
            return coords;
        }
        await new Promise(r => setTimeout(r, 20));
    }
    return [];
}

// ===== 手势传感器解析与等待 =====
function parseGestureValueFromText(text) {
    if (!text) return { value: null, key: '' };
    const norm = String(text).replace(/\r\n/g, "\n").replace(/\r/g, "\n");
    
    // 只匹配完整格式 =\n数字\nX（支持 -1, 0, 1, 2, 3）
    const frameRegex = /=\s*\n(-1|[0-3])?\s*\nX/gi;
    let match = null, lastMatch = null;
    while ((match = frameRegex.exec(norm)) !== null) {
        lastMatch = match;
    }
    if (lastMatch) {
        const digit = lastMatch[1];
        const val = (typeof digit !== 'undefined' && digit !== undefined && digit !== '') ? parseInt(digit, 10) : null;
        const key = `=${digit ?? ''}|X`;
        return { value: (val !== null && Number.isInteger(val)) ? val : null, key };
    }
    
    return { value: null, key: '' };
}

function getLatestGestureNoWait() {
    try {
        let buf = '';
        if (typeof serialBuffer !== 'undefined' && typeof serialBuffer === 'string') {
            buf = serialBuffer;
        } else if (typeof window !== 'undefined' && typeof window.serialBuffer === 'string') {
            buf = window.serialBuffer;
        }
        if (!buf) return { value: null, key: '' };
        const tail = buf.slice(-2000);
        return parseGestureValueFromText(tail);
    } catch (e) {
        return { value: null, key: '' };
    }
}

async function waitForNewGestureValue(prevKey, timeoutMs = 500) {
    const start = Date.now();
    while (Date.now() - start < timeoutMs) {
        const { value, key } = getLatestGestureNoWait();
        if (key && key !== prevKey) {
            if (typeof window !== 'undefined') {
                window.__lastGestureFrameKey = key;
            }
            if (value === null || value === undefined) {
                return -1;
            }
            return value;
        }
        await new Promise(r => setTimeout(r, 5));
    }
    return -1;
}

// rawResult可能是两种格式之一:
// 旧格式: "0\t1\t2\t...\n0,\t0,\t0,\t...\nj\n"
// 新格式: "=\n0 1 2 3 ...\n-1, -1, 0, 0, ...\nj\n"
function parseAllJointsResult(rawResult) {
    if (typeof showDebug !== 'undefined' && showDebug) {
        console.log(getText('debugParseAllJointsStart'), typeof rawResult);
        console.log(getText('debugParseAllJointsRawLength'), rawResult ? rawResult.length : 0);
    }
    
    // 检查rawResult是否为null或undefined
    if (!rawResult) {
        console.warn('parseAllJointsResult: rawResult is null or undefined');
        return [];
    }
    
    const lines = rawResult.split("\n").map(line => line.trim());
    if (typeof showDebug !== 'undefined' && showDebug) {
        console.log(getText('debugParseAllJointsSplitLines'), lines.length);
        console.log(getText('debugParseAllJointsLineContent'), JSON.stringify(lines));
    }
    
    // 查找结束标记 'j' 的位置
    let jIndex = -1;
    for (let i = 0; i < lines.length; i++) {
        if (lines[i] === 'j') {
            jIndex = i;
            break;
        }
    }
    
    if (jIndex < 0) {
        console.warn('parseAllJointsResult: 未找到结束标记 "j"');
        return [];
    }
    
    // 新格式: = \n 索引行 \n 角度行 \n j
    if (jIndex >= 3 && lines[jIndex - 3] === '=') {
        const indexLine = lines[jIndex - 2];
        const angleLine = lines[jIndex - 1];
        
        // 索引行用空格分隔
        const indexs = indexLine
            .split(/\s+/)
            .filter((item) => item.length > 0)
            .map((num) => parseInt(num));
        
        // 角度行用 ", " 分隔
        const angles = angleLine
            .split(/,\s*/)
            .filter((item) => item.length > 0)
            .map((num) => parseInt(num));
        
        if (angles.length > 0) {
            return angles;
        }
    }
    
    // 旧格式: 索引行 \n 角度行 \n j (兼容性支持)
    if (jIndex >= 2) {
        const indexLine = lines[jIndex - 2];
        const angleLine = lines[jIndex - 1];
        
        // 尝试用 \t 分隔
        const indexs = indexLine
            .split("\t")
            .filter((item) => item.length > 0)
            .map((num) => parseInt(num));
        
        const angles = angleLine
            .split(",\t")
            .filter((item) => item.length > 0)
            .map((num) => parseInt(num));
        
        if (angles.length > 0) {
            return angles;
        }
    }
    
    console.warn('parseAllJointsResult: 无法解析关节角度数据');
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
            if (typeof showDebug !== 'undefined' && showDebug) {
                console.log(getText('debugDetectedRelativeAngles'));
            }
            try {
                // 在串口模式下，webRequest已经被重定向到serialRequest
                // 所以可以直接使用webRequest，它会自动调用正确的函数
                const isSerialMode = (typeof window !== 'undefined' && window.__isSerialMode === true);
                if (typeof showDebug !== 'undefined' && showDebug) {
                    console.log(getText('debugCurrentMode'), isSerialMode ? getText('debugSerialMode') : getText('debugWiFiMode'));
                }
                let rawResult;
                rawResult = await webRequest("j", JOINT_QUERY_TIMEOUT, true);
                if (typeof showDebug !== 'undefined' && showDebug) {
                    console.log(getText('debugQueryJCommandRaw'), rawResult);
                }
                const result = parseAllJointsResult(rawResult);
                if (typeof showDebug !== 'undefined' && showDebug) {
                    console.log(getText('debugParsedJointAngles'), result);
                }
                // 检查查询结果是否有效
                if (result && result.length > 0) {
                    joints = result;
                    if (typeof showDebug !== 'undefined' && showDebug) {
                        console.log(getText('debugSuccessGetJointAngles'), result.length);
                    }
                } else {
                    console.warn(getText('debugCannotGetJointAngles'));
                    // joints 保持为 Array(16).fill(0)
                }
            } catch (error) {
                console.error(getText('debugWaitJointAnglesError'), error);
                console.warn(getText('debugUseDefaultJointAngles'));
            }
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
