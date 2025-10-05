// 定义工具箱颜色变量
const COMMUNICATION_COLOR = "#e2677c"; // 通信积木：红色
const MOTION_COLOR = "#546acb";        // 动作积木：蓝色
const CONSOLE_COLOR = "#57AFCF";       // 控制台积木：蓝色
const MUSIC_COLOR = "#e488ce";         // 音乐积木：粉色
const MATH_COLOR = "#5B67A5";          // 数学积木：蓝色

const customTheme = Blockly.Theme.defineTheme("myCustomTheme", {
    base: Blockly.Themes.Classic, // 继承经典主题的基础配置
    categoryStyles: {
        communication_category: {
            colour: COMMUNICATION_COLOR, // 红色
        },
        motion_category: {
            colour: MOTION_COLOR, // 蓝色
        },
        console_category: {
            colour: CONSOLE_COLOR, // 蓝色
        },
        music_category: {
            colour: MUSIC_COLOR, // 粉色
        },
    },
});
// 动态创建工具箱配置
function createToolbox() {
    return {
        kind: "categoryToolbox",
        contents: [
            {
                kind: "category",
                name: getText("categoryLogic"),
                categorystyle: "logic_category",
                contents: [
                    { kind: "block", type: "controls_if" },
                    { kind: "block", type: "logic_compare" },
                    { kind: "block", type: "logic_operation" },
                    { kind: "block", type: "logic_negate" },
                    { kind: "block", type: "logic_boolean" },
                    { kind: "block", type: "logic_null" },
                    { kind: "block", type: "logic_ternary" },
                ],
            },
            {
                kind: "category",
                name: getText("categoryLoops"),
                categorystyle: "loop_category",
                contents: [
                    { kind: "block", type: "controls_repeat_ext" },
                    { kind: "block", type: "controls_whileUntil" },
                    { kind: "block", type: "controls_for" },
                    { kind: "block", type: "controls_forEach" },
                    { kind: "block", type: "controls_flow_statements" },
                ],
            },
            {
                kind: "category",
                name: getText("categoryMath"),
                categorystyle: "math_category",
                contents: [
                    { kind: "block", type: "math_number" },
                    { kind: "block", type: "math_arithmetic" },
                    { kind: "block", type: "math_single" },
                    { kind: "block", type: "math_trig" },
                    { kind: "block", type: "math_constant" },
                    { kind: "block", type: "math_number_property" },
                    { kind: "block", type: "math_round" },
                    { kind: "block", type: "math_modulo" },
                    { kind: "block", type: "math_random" },
                ],
            },
            {
                kind: "category",
                name: getText("categoryText"),
                categorystyle: "text_category",
                contents: [
                    { kind: "block", type: "text" },
                    { kind: "block", type: "text_join" },
                    { kind: "block", type: "text_append" },
                    { kind: "block", type: "text_length" },
                    { kind: "block", type: "text_isEmpty" },
                    { kind: "block", type: "text_indexOf" },
                    { kind: "block", type: "text_charAt" },
                    { kind: "block", type: "text_getSubstring" },
                    { kind: "block", type: "text_changeCase" },
                    { kind: "block", type: "text_trim" },
                    { kind: "block", type: "text_print" },
                ],
            },
            {
                kind: "category",
                name: getText("categoryVariables"),
                categorystyle: "variable_category",
                custom: "VARIABLE",
            },
            {
                kind: "category",
                name: getText("categoryLists"),
                categorystyle: "list_category",
                contents: [
                    {
                        kind: "block",
                        type: "lists_create_with",
                    },
                    {
                        kind: "block",
                        type: "lists_length",
                    },
                    {
                        kind: "block",
                        type: "lists_isEmpty",
                    },
                    {
                        kind: "block",
                        type: "lists_indexOf",
                        inputs: {
                            VALUE: {
                                block: {
                                    type: "variables_get",
                                },
                            },
                        },
                    },
                    {
                        kind: "block",
                        type: "lists_getIndex",
                        inputs: {
                            VALUE: {
                                block: {
                                    type: "variables_get",
                                },
                            },
                        },
                    },
                    {
                        kind: "block",
                        type: "lists_setIndex",
                        inputs: {
                            LIST: {
                                block: {
                                    type: "variables_get",
                                },
                            },
                        },
                    },
                ],
            },
            {
                kind: "category",
                name: getText("categoryFunctions"),
                categorystyle: "procedure_category",
                custom: "PROCEDURE",
            },
            {
                kind: "category",
                name: getText("categoryCommunication"),
                categorystyle: "communication_category",
                contents: [
                    { kind: "block", type: "get_digital_input" },
                    { kind: "block", type: "get_analog_input" },
                    { kind: "block", type: "set_digital_output" },
                    { kind: "block", type: "set_analog_output",
                        inputs: {
                            VALUE: {
                                shadow: {
                                    type: "math_number",
                                    fields: {
                                        NUM: 128,
                                    },
                                },
                            },
                        } 
                    },
                    { kind: "block", type: "getUltrasonicDistance" },
                    { kind: "block", type: "getCameraCoordinate" },
                    { kind: "block", type: "get_gesture_value" },
                    { kind: "block", type: "send_custom_command",
                        inputs: {
                            COMMAND: {
                                shadow: {
                                    type: "text",
                                    fields: {
                                        TEXT: "m 0 60",
                                    },
                                },
                            },
                        } 
                    },
                    { kind: "block", type: "gyro_control" },
                ],
            },
            {
                kind: "category",
                name: getText("categoryMotion"),
                categorystyle: "motion_category",
                contents: [
                    { kind: "block", type: "gait" },
                    { kind: "block", type: "posture" },
                    { kind: "block", type: "acrobatic_moves" },
                    { kind: "block", type: "arm_action" },
                    { kind: "block", type: "get_joint_angle" },
                    { kind: "block", type: "get_all_joint_angles" },
                    { kind: "block", type: "set_joint_angle", inputs: {
                        VARIABLE: {
                            block: {
                                type: "joint_absolute_angle_value",
                                inputs: {
                                    ANGLE: {
                                        shadow: {
                                            type: "math_number",
                                        },
                                    },
                                },
                            },
                        },
                    } },
                    { kind: "block", type: "set_joint_angle", inputs: {
                        VARIABLE: {
                            block: {
                                type: "joint_relative_angle_value",
                                inputs: {
                                    ANGLE: {
                                        shadow: {
                                            type: "math_number",
                                        },
                                    },
                                },
                            },
                        },
                    } },
                    { kind: "block", type: "set_joints_angle_sim", inputs: {
                        VARIABLE: {
                            block: {
                                type: "lists_create_with",
                            },
                        },
                    } },
                    { kind: "block", type: "set_joints_angle_seq", inputs: {
                        VARIABLE: {
                            block: {
                                type: "lists_create_with",
                            },
                        },
                    } },
                    { kind: "block", type: "joint_absolute_angle_value", 
                        inputs: {
                            ANGLE: {
                                shadow: {
                                    type: "math_number"
                                },
                            },
                        } 
                    },
                    { kind: "block", 
                        type: "joint_relative_angle_value", 
                        inputs: {
                            ANGLE: {
                                shadow: {
                                    type: "math_number"
                                },
                            },
                        } 
                    },
                    { kind: "block", type: "set_joints_angle_sim_raw", inputs: {
                        VARIABLE: {
                            block: {
                                type: "joints_angle_frame_raw",
                                fields: {
                                    VARIABLE: "0, 0, 0, 0, 0, 0, 0, 0, 30, 30, 30, 30, 30, 30, 30, 30",
                                },
                            },
                        },
                    } },
                    { kind: "block", type: "joints_angle_frame_raw", fields: {
                        VARIABLE: "0, 0, 0, 0, 0, 0, 0, 0, 30, 30, 30, 30, 30, 30, 30, 30",
                    } },
                    { kind: "block", type: "action_skill_file" },
                ],
            },
            {
                kind: "category",
                name: getText("categoryConsole"),
                colour: CONSOLE_COLOR,
                contents: [
                    { kind: "block", type: "console_log_variable" },
                    { kind: "block", type: "console_input" },
                    { kind: "block", type: "delay_ms" },
                ],
            },
            {
                kind: "category",
                name: getText("categoryMusic"),
                colour: MUSIC_COLOR,
                contents: [
                    { kind: "block", type: "play_note" },
                    { kind: "block", type: "play_melody" },
                    { kind: "block", type: "play_tone_list" },
                ],
            },
        ],
    };
}

function blocklyGlobalConfig() {
    // 保持工具箱中的积木块尺寸不变
    Blockly.VerticalFlyout.prototype.getFlyoutScale = function () {
        return 1;
    };

    const digitalInputOptions = [
        ["34", "34"],
        ["35", "35"],
        ["36", "36"],
        ["39", "39"],
        ["BackTouch(38)", "38"],
        ["Rx2(9)", "9"],
        ["Tx2(10)", "10"],
    ];

    const analogInputOptions = [
        ["34", "34"],
        ["35", "35"],
        ["36", "36"],
        ["39", "39"],
        ["BackTouch(38)", "38"],
        ["Rx2(9)", "9"],
        ["Tx2(10)", "10"],
        ["Battery", "37"],
    ];

    const digitalOutputOptions = [
        ["BackTouch(38)", "38"],
        ["Rx2(9)", "9"],
        ["Tx2(10)", "10"],
        ["Buzzer","2"],
        ["LED","27"],
    ];

    const analogOutputOptions = [
        ["BackTouch(38)", "38"],
        ["Rx2(9)", "9"],
        ["Tx2(10)", "10"],
        ["Buzzer","2"],
        ["LED","27"],
    ];

    const jointOptions = [
        [getText("jointHeadPanning"), "0"],
        [getText("jointHeadTiltingNybble"), "1"],
        [getText("jointTailNybble"), "2"],
        [getText("jointReserved"), "3"],
        [getText("jointLFArm"), "8"],
        [getText("jointRFArm"), "9"],
        [getText("jointRBArm"), "10"],
        [getText("jointLBArm"), "11"],
        [getText("jointLFKnee"), "12"],
        [getText("jointRFKnee"), "13"],
        [getText("jointRBKnee"), "14"],
        [getText("jointLBKnee"), "15"],
    ];

    // 通信积木
    Blockly.Blocks["make_connection"] = {
        init: function() {
            this.jsonInit({
                type: "make_connection",
                message0: getText("connectWithIP"),
                args0: [
                    {
                        type: "field_input",
                        name: "IP_ADDRESS",
                        text: currentDeviceIP || "192.168.4.1",
                    },
                ],
                nextStatement: null,
                colour: COMMUNICATION_COLOR, // 通信积木：红色
                tooltip: "",
                helpUrl: "",
            });
        }
    }

    // 超声波传感器距离(cm): Trigger [TRPIN] Echo [ECPIN]
    Blockly.Blocks["trackUltrasonicDistance"] = {
        init: function () {
            this.jsonInit({
                type: "trackUltrasonicDistance",
                message0: getText("trackUltrasonicDistance"),
                args0: [
                    {
                        type: "field_variable",
                        name: "RPC_VAR",
                    },
                    {
                        type: "field_dropdown",
                        name: "TRPIN",
                        options: digitalInputOptions,
                    },
                    {
                        type: "field_dropdown",
                        name: "ECPIN",
                        options: digitalInputOptions,
                    },
                ],
                message1: "then %1",
                args1: [
                    {
                        type: "input_statement",
                        name: "DO",
                    },
                ],
                previousStatement: null,
                colour: COMMUNICATION_COLOR, // 通信积木：红色
                tooltip: "",
            });
        },
    };

    // 超声波传感器距离(cm): Trigger [TRPIN] Echo [ECPIN]
    const trpinOptions = [
        ["BiBoard V0 Rx2", "16"],
        ["BiBoard V0 Tx2", "17"],
        ["BiBoard V1 Rx2", "9"],
        ["BiBoard V1 Tx2", "10"]
    ]
    Blockly.Blocks["getUltrasonicDistance"] = {
        init: function () {
            const ecpinOptions = [["Same as trigger", "-1"]].concat(trpinOptions);
            this.jsonInit({
                type: "getUltrasonicDistance",
                message0: getText("getUltrasonicDistance"),
                args0: [
                    {
                        type: "field_dropdown",
                        name: "TRPIN",
                        options: trpinOptions,

                    },
                    {
                        type: "field_dropdown",
                        name: "ECPIN",
                        options: ecpinOptions,
                    },
                ],
                output: "Number",
                colour: COMMUNICATION_COLOR, // 通信积木：红色
                tooltip: "",
            });
            this.setFieldValue(trpinOptions[2][1], "TRPIN");
            this.setFieldValue(ecpinOptions[0][1], "ECPIN"); // 默认选择第一个选项
        },
    };

    // 摄像头识别目标的坐标值(%1,%2,%3,%4)
    Blockly.Blocks["trackCameraCoordinate"] = {
        init: function () {
            this.jsonInit({
                type: "trackCameraCoordinate",
                message0: getText("trackCameraCoordinate"),
                args0: [
                    {
                        type: "field_variable",
                        name: "CAM_VAR",
                    }
                ],
                message1: "then %1",
                args1: [
                    {
                        type: "input_statement",
                        name: "DO",
                    },
                ],
                previousStatement: null,
                colour: COMMUNICATION_COLOR, // 通信积木：红色
                tooltip: "",
            });
        },
    };

    // 摄像头识别目标的坐标值(%1,%2,%3,%4)
    Blockly.Blocks["getCameraCoordinate"] = {
        init: function () {
            this.jsonInit({
                type: "getCameraCoordinate",
                message0: getText("getCameraCoordinate"),
                output: "Array",
                colour: COMMUNICATION_COLOR, // 通信积木：红色
                tooltip: "",
            });
        },
    };

    // 手势传感器读取积木
    Blockly.Blocks["get_gesture_value"] = {
        init: function () {
            this.jsonInit({
                type: "get_gesture_value",
                message0: "Get gesture sensor value",
                output: "Number",
                colour: COMMUNICATION_COLOR, // 通信：红色
                tooltip: "Read gesture value (0:Up,1:Down,2:Left,3:Right)"
            });
        },
    };

    // 模拟输入积木
    Blockly.Blocks["get_analog_input"] = {
        init: function () {
            this.jsonInit({
                type: "get_analog_input",
                message0: getText("getAnalogInput"),
                args0: [
                    {
                        type: "field_dropdown",
                        name: "PIN",
                        options: analogInputOptions,
                    },
                ],
                output: "Number",
                colour: COMMUNICATION_COLOR, // 通信积木：红色
                tooltip: "",
            });
        }
    }

    // 数字输入积木
    Blockly.Blocks["get_digital_input"] = {
        init: function () {
            this.jsonInit({
                type: "get_digital_input",
                message0: getText("getDigitalInput"),
                args0: [
                    {
                        type: "field_dropdown",
                        name: "PIN",
                        options: digitalInputOptions,
                    },
                ],
                output: "Number",
                colour: COMMUNICATION_COLOR, // 通信积木：红色
                tooltip: "",
            });
        }
    }

    // 数字输出积木
    Blockly.Blocks["set_digital_output"] = {
        init: function () {
            this.jsonInit({
                type: "set_digital_output",
                message0: getText("setDigitalOutput"),
                args0: [
                    {
                        type: "field_dropdown",
                        name: "PIN",
                        options: digitalOutputOptions,
                    },
                    {
                        type: "field_dropdown",
                        name: "STATE",
                        options: [
                            ["HIGH", "1"],
                            ["LOW", "0"],
                        ],
                    },
                ],
                previousStatement: null,
                nextStatement: null,
                colour: COMMUNICATION_COLOR, // 通信积木：红色
                tooltip: "",
            });
        }
    }

    // 模拟输出积木
    Blockly.Blocks["set_analog_output"] = {
        init: function () {
            this.jsonInit({
                type: "set_analog_output",
                message0: getText("setAnalogOutput"),
                args0: [
                    {
                        type: "field_dropdown",
                        name: "PIN",
                        options: analogOutputOptions,
                    },
                    {
                        type: "input_value",
                        name: "VALUE",
                        check: "Number",
                    },
                ],
                previousStatement: null,
                nextStatement: null,
                colour: COMMUNICATION_COLOR, // 通信积木：红色
                tooltip: "",
            })
        }
    }

    // 自定义命令积木
    Blockly.Blocks["send_custom_command"] = {
        init: function () {
            this.jsonInit({
                type: "send_custom_command",
                message0: getText("sendCustomCommand"),
                args0: [
                    {
                        type: "input_value",
                        name: "COMMAND",
                        check: "String",
                    },
                    {
                        type: "field_number",
                        name: "DELAY",
                        value: 1,
                        min: 0,
                    },
                ],
                previousStatement: null,
                nextStatement: null,
                colour: COMMUNICATION_COLOR, // 通信积木：红色
                tooltip: "",
            });
        }
    }

    // 控制台输入积木
    Blockly.Blocks["console_input"] = {
        init: function () {
            this.jsonInit({
                type: "console_input",
                message0: getText("consoleInput"),
                args0: [
                    {
                        type: "field_input",
                        name: "PROMPT",
                        text: getText("consoleInputDefaultPrompt"),
                        spellcheck: false,
                    },
                ],
                output: "String",
                colour: CONSOLE_COLOR, // 控制台积木：蓝色
                tooltip: "",
            });
        }
    }

    // 关节角度积木
    Blockly.Blocks["set_joint_angle"] = {
        init: function () {
            this.jsonInit({
                type: "set_joint_angle",
                message0: getText("setJointAngle"),
                args0: [
                    {
                        type: "input_value",
                        name: "VARIABLE",
                        check: "Array",
                    },
                    {
                        type: "field_number",
                        name: "DELAY",
                        value: 0.2,
                        min: 0,
                        max: 10,
                        step: 0.01
                    },
                ],
                previousStatement: null,
                nextStatement: null,
                colour: MOTION_COLOR, // 动作积木：蓝色
                tooltip: "",
            });
        },
    };

    // 关节角度积木
    Blockly.Blocks["set_joints_angle_seq"] = {
        init: function () {
            this.jsonInit({
                type: "set_joints_angle_seq",
                message0: getText("setJointsAngleGroupSeq"),
                args0: [
                    {
                        type: "input_value",
                        name: "VARIABLE",
                        check: "Array",
                    },
                    {
                        type: "field_number",
                        name: "DELAY",
                        value: 0.2,
                        min: 0,
                        max: 10,
                        step: 0.01
                    },
                ],
                previousStatement: null,
                nextStatement: null,
                colour: MOTION_COLOR, // 动作积木：蓝色
                tooltip: "",
            });
        },
    };


    // 关节角度积木
    Blockly.Blocks["set_joints_angle_sim"] = {
        init: function () {
            this.jsonInit({
                type: "set_joints_angle_sim",
                message0: getText("setJointsAngleGroupSim"),
                args0: [
                    {
                        type: "input_value",
                        name: "VARIABLE",
                        check: "Array",
                    },
                    {
                        type: "field_number",
                        name: "DELAY",
                        value: 0.2,
                        min: 0,
                        max: 10,
                        step: 0.01
                    },
                ],
                previousStatement: null,
                nextStatement: null,
                colour: MOTION_COLOR, // 动作积木：蓝色
                tooltip: "",
            });
        },
    };


    // 关节角度积木
    Blockly.Blocks["set_joints_angle_sim_raw"] = {
        init: function () {
            this.jsonInit({
                type: "set_joints_angle_sim_raw",
                message0: getText("setJointsAngleGroupSimRaw"),
                args0: [
                    {
                        type: "input_value",
                        name: "VARIABLE",
                        check: "Array",
                    },
                    {
                        type: "field_number",
                        name: "DELAY",
                        value: 0.2,
                        min: 0,
                        max: 10,
                        step: 0.01
                    },
                ],
                previousStatement: null,
                nextStatement: null,
                colour: MOTION_COLOR, // 动作积木：蓝色
                tooltip: "",
            });
        },
    };

    Blockly.Blocks["joints_angle_frame_raw"] = {
        init: function () {
            this.jsonInit({
                type: "joints_angle_frame_raw",
                message0: getText("jointsAngleFrameRaw"),
                args0: [
                    {
                        type: "field_input",
                        name: "VARIABLE",
                        text: "",
                    }
                ],
                output: "Array",
                colour: MOTION_COLOR, // 动作积木：蓝色
                tooltip: "",
            });
        },
    };

    // 设置的马达角度积木
    Blockly.Blocks["joint_absolute_angle_value"] = {
        init: function () {
            this.jsonInit({
                type: "joint_absolute_angle_value",
                message0: getText("jointAbsoluteAngleValue"),
                args0: [
                    {
                        type: "field_dropdown",
                        name: "JOINT",
                        options: jointOptions,
                    },
                    {
                        type: "input_value",
                        name: "ANGLE",
                        check: "Number",
                    },
                ],
                output: "Array",
                colour: MOTION_COLOR,
            });
        },
    };

    // 设置的马达相对角度积木
    Blockly.Blocks["joint_relative_angle_value"] = {
        init: function () {
            this.jsonInit({
                type: "joint_relative_angle_value",
                message0: getText("jointRelativeAngleValue"),
                args0: [
                    {
                        type: "field_dropdown",
                        name: "JOINT",
                        options: jointOptions,
                    },
                    {
                        type: "field_dropdown",
                        name: "ANGLE_SIGN",
                        options: [
                            ["+", "1"],
                            ["-", "-1"],
                        ],
                    },
                    {
                        type: "input_value",
                        name: "ANGLE",
                        check: "Number",
                    },
                ],
                output: "Array",
                colour: MOTION_COLOR,
            });
        },
    };

    // 获取舵机角度积木
    Blockly.Blocks["get_joint_angle"] = {
        init: function () {
            this.jsonInit({
                type: "get_joint_angle",
                message0: getText("getJointAngle"),
                args0: [
                    {
                        type: "field_dropdown",
                        name: "JOINT",
                        options: jointOptions,
                    },
                ],
                output: "Number",
                colour: MOTION_COLOR, // 动作积木：蓝色
                tooltip: "",
            });
        },
    };

    // 获取所有舵机角度积木
    Blockly.Blocks["get_all_joint_angles"] = {
        init: function () {
            this.jsonInit({
                type: "get_all_joint_angles",
                message0: getText("getAllJointAngles"),
                output: "Array",
                colour: MOTION_COLOR, // 动作积木：蓝色
                tooltip: "",
            });
        },
    };

    // 延时积木
    Blockly.Blocks["delay_ms"] = {
        init: function () {
            this.jsonInit({
                type: "delay_ms",
                message0: getText("delayMs"),
                args0: [
                    {
                        type: "field_number",
                        name: "DELAY",
                        value: 1,
                        min: 0,
                    },
                ],
                previousStatement: null,
                nextStatement: null,
                colour: CONSOLE_COLOR, // 控制台积木：蓝色
                tooltip: "",
            });
        }
    }

    // 陀螺仪控制积木
    Blockly.Blocks["gyro_control"] = {
        init: function () {
            this.jsonInit({
                type: "gyro_control",
                message0: getText("gyroControl"),
                args0: [
                    {
                        type: "field_dropdown",
                        name: "STATE",
                        options: [
                            [getText("gyroEnable"), "1"],
                            [getText("gyroDisable"), "0"],
                        ],
                    },
                ],
                previousStatement: null,
                nextStatement: null,
                colour: COMMUNICATION_COLOR, // 传感器积木：红色
                tooltip: "",
            });
        },
    };

    // 控制台输出积木
    Blockly.Blocks["console_log_variable"] = {
        init: function () {
            this.jsonInit({
                type: "console_log_variable",
                message0: getText("consoleLogVariable"),
                args0: [
                    {
                        type: "input_value",
                        name: "VARIABLE",
                    },
                ],
                previousStatement: null,
                nextStatement: null,
                colour: CONSOLE_COLOR, // 控制台积木：蓝色
                tooltip: "",
            });
        },
    };

    // 执行技能文件
    Blockly.Blocks["action_skill_file"] = {
        init: function () {
            this.jsonInit({
                type: "action_skill_file",
                message0: getText("actionSkillFile"),
                args0: [
                    {
                        type: "field_input",
                        name: "FILENAME",
                        text: "",
                    },
                    {
                        type: "field_number",
                        name: "DELAY",
                        value: 0,
                        min: 0,
                    },
                ],
                previousStatement: null,
                nextStatement: null,
                colour: MOTION_COLOR, // 动作积木：蓝色
                tooltip: "",
            });
        },
    };
    

    const noteOptions = [
        [getText("noteRest"), "0"],
        [getText("noteLowC"), "8"],
        [getText("noteLowCSharp"), "9"],
        [getText("noteLowD"), "10"],
        [getText("noteLowDSharp"), "11"],
        [getText("noteLowE"), "12"],
        [getText("noteLowF"), "13"],
        [getText("noteLowFSharp"), "14"],
        [getText("noteLowG"), "15"],
        [getText("noteLowGSharp"), "16"],
        [getText("noteLowA"), "17"],
        [getText("noteLowASharp"), "18"],
        [getText("noteLowB"), "19"],
        [getText("noteMiddleC"), "20"],
        [getText("noteMiddleCSharp"), "21"],
        [getText("noteMiddleD"), "22"],
        [getText("noteMiddleDSharp"), "23"],
        [getText("noteMiddleE"), "24"],
        [getText("noteMiddleF"), "25"],
        [getText("noteMiddleFSharp"), "26"],
        [getText("noteMiddleG"), "27"],
        [getText("noteMiddleGSharp"), "28"],
        [getText("noteMiddleA"), "29"],
        [getText("noteMiddleASharp"), "30"],
        [getText("noteMiddleB"), "31"],
        [getText("noteHighC"), "32"],
    ];

    const durationOptions = [
        ["1", "1"],
        ["1/2", "2"],
        ["1/4", "4"],
        ["1/8", "8"],
        ["1/16", "16"],
    ];
    // 音乐积木
    Blockly.Blocks["play_note"] = {
        init: function () {
            this.jsonInit({
                type: "play_note",
                message0: getText("playNoteMessage"),
                args0: [
                    {
                        type: "field_dropdown",
                        name: "NOTE",
                        options: noteOptions,
                    },
                    {
                        type: "field_dropdown",
                        name: "DURATION",
                        options: durationOptions,
                    },
                ],
                previousStatement: null,
                nextStatement: null,
                colour: MUSIC_COLOR, // 音乐积木：粉色
                tooltip: "",
            });
        },
    };

    // 音乐积木
    Blockly.Blocks["play_note_value"] = {
        init: function () {
            this.jsonInit({
                type: "play_note_value",
                message0: getText("playNoteMessage"),
                args0: [
                    {
                        type: "field_dropdown",
                        name: "NOTE",
                        options: noteOptions,
                    },
                    {
                        type: "field_dropdown",
                        name: "DURATION",
                        options: durationOptions,
                    },
                ],
                output: "String",
                colour: MUSIC_COLOR, // 音乐积木：粉色
                tooltip: "",
            });
        },
    };

    // 音乐积木
    Blockly.Blocks["play_melody"] = {
        init: function () {
            this.jsonInit({
                type: "play_melody",
                message0: getText("playMelodyMessage"),
                args0: [
                    {
                        type: "input_statement",
                        name: "MELODY",
                        check: ["play_note"],
                    },
                    {
                        type: "field_number",
                        name: "DELAY",
                        value: 1,
                        min: 0,
                    }
                ],
                previousStatement: null,
                nextStatement: null,
                colour: MUSIC_COLOR, // 音乐积木：粉色
                tooltip: "",
            });
        }
    };

    // 步态动作积木
    Blockly.Blocks["gait"] = {
        init: function () {
            var gaitLabel = getText("gaitLabel");
            var delayLabel = getText("thenDelay");
            var unitLabel = getText("secUnit");
            var options = [
                [getText("gaitStep"), "kvtF"],
                [getText("gaitRotateLeft"), "kvtL"],
                [getText("gaitRotateRight"), "kvtR"],
                [getText("gaitWalkForward"), "kwkF"],
                [getText("gaitWalkLeft"), "kwkL"],
                [getText("gaitWalkRight"), "kwkR"],
                [getText("gaitWalkBackward"), "kbkF"],
                [getText("gaitBackLeft"), "kbkL"],
                [getText("gaitBackRight"), "kbkR"],
                [getText("gaitTrotForward"), "ktrF"],
                [getText("gaitTrotLeft"), "ktrL"],
                [getText("gaitTrotRight"), "ktrR"],
                [getText("gaitCrawlForward"), "kcrF"],
                [getText("gaitCrawlLeft"), "kcrL"],
                [getText("gaitCrawlRight"), "kcrR"],
                [getText("gaitGapForward"), "kgpF"],
                [getText("gaitGapLeft"), "kgpL"],
                [getText("gaitGapRight"), "kgpR"],
                [getText("gaitMoonwalk"), "kmw"],
            ];

            this.appendDummyInput()
                .appendField(gaitLabel)
                .appendField(new Blockly.FieldDropdown(options), "COMMAND");
            this.appendDummyInput()
                .appendField(delayLabel)
                .appendField(new Blockly.FieldNumber(1, 0, 10, 0.01), "DELAY")
                .appendField(unitLabel);
            this.setInputsInline(true);
            this.setPreviousStatement(true, null);
            this.setNextStatement(true, null);
            this.setColour(MOTION_COLOR);
        },
    };

    // 姿势动作积木
    Blockly.Blocks["posture"] = {
        init: function () {
            var postureLabel = getText("postureLabel");
            var delayLabel = getText("thenDelay");
            var unitLabel = getText("secUnit");
            var options = [
                [getText("postureStand"), "kup"],
                [getText("postureSit"), "ksit"],
                [getText("postureRest"), "d"],
                [getText("posturePee"), "kpee"],
                // 添加新的选项
                [getText("postureSayHi"), "khi"],  // 新增
            ];

            this.appendDummyInput()
                .appendField(postureLabel)
                .appendField(new Blockly.FieldDropdown(options), "COMMAND");
            this.appendDummyInput()
                .appendField(delayLabel)
                .appendField(new Blockly.FieldNumber(0, 0, 10, 0.01), "DELAY")
                .appendField(unitLabel);
            this.setInputsInline(true);
            this.setPreviousStatement(true, null);
            this.setNextStatement(true, null);
            this.setColour(MOTION_COLOR);
        },
    };

    // 杂技动作积木
    Blockly.Blocks["acrobatic_moves"] = {
        init: function () {
            var moveLabel = getText("acrobaticMovesLabel");
            var delayLabel = getText("thenDelay");
            var unitLabel = getText("secUnit");
            var options = [
                [getText("acrobaticHandstand"), "khds"],
                [getText("acrobaticBoxing"), "kbx"],
                [getText("acrobaticBackflip"), "kflipD"],
                [getText("acrobaticFrontflip"), "kflipF"],
                [getText("acrobaticJump"), "kjmp"],
            ];

            this.appendDummyInput()
                .appendField(moveLabel)
                .appendField(new Blockly.FieldDropdown(options), "COMMAND");
            this.appendDummyInput()
                .appendField(delayLabel)
                .appendField(new Blockly.FieldNumber(0, 0, 10, 0.01), "DELAY")
                .appendField(unitLabel);
            this.setInputsInline(true);
            this.setPreviousStatement(true, null);
            this.setNextStatement(true, null);
            this.setColour(MOTION_COLOR);
        },
    };

    // 添加机械臂动作积木
    Blockly.Blocks["arm_action"] = {
        init: function () {
            this.jsonInit({
                type: "arm_action",
                message0: getText("armActionMessage"),
                args0: [
                    {
                        type: "field_dropdown",
                        name: "COMMAND",
                        options: [
                            [getText("armPickDown"), "kpickD"],
                            [getText("armPickFront"), "kpickF"],
                            [getText("armPickLeft"), "kpickL"],
                            [getText("armPickRight"), "kpickR"],
                            [getText("armPutDown"), "kputD"],
                            [getText("armPutFront"), "kputF"],
                            [getText("armPutLeft"), "kputL"],
                            [getText("armPutRight"), "kputR"],
                            [getText("armShoot"), "klaunch"],
                            [getText("armThrowFront"), "ktossF"],
                            [getText("armThrowLeft"), "ktossL"],
                            [getText("armThrowRight"), "ktossR"],
                            [getText("armHunt"), "khunt"],
                            [getText("armShowOff"), "kshowOff"],
                            [getText("armClap"), "kclap"]
                        ],
                    },
                    {
                        type: "field_number",
                        name: "DELAY",
                        value: 0.2,
                        min: 0,
                        max: 10,
                        step: 0.01
                    },
                ],
                previousStatement: null,
                nextStatement: null,
                colour: MOTION_COLOR,
            });
        },
    };

    // 音调和时长列表积木
    Blockly.Blocks["play_tone_list"] = {
        init: function () {
            this.jsonInit({
                type: "play_tone_list",
                message0: getText("playToneListMessage"),
                args0: [
                    {
                        type: "field_input",
                        name: "TONE_LIST",
                        text: "14,4,14,4,21,4,21,4,23,4,23,4,21,2",
                        spellcheck: false,
                    },
                    {
                        type: "field_number",
                        name: "DELAY",
                        value: 1,
                        min: 0,
                        max: 10,
                        step: 0.01
                    },
                ],
                previousStatement: null,
                nextStatement: null,
                colour: MUSIC_COLOR, // 音乐积木：粉色
                tooltip: getText("playToneListTooltip"),
            });
        },
    };

    // 随机数积木块
    Blockly.Blocks["math_random"] = {
        init: function () {
            this.jsonInit({
                type: "math_random",
                message0: getText("mathRandomMessage"),
                args0: [
                    {
                        type: "field_number",
                        name: "FROM",
                        value: 1,
                        min: -999999,
                        max: 999999,
                        step: 1
                    },
                    {
                        type: "field_number",
                        name: "TO",
                        value: 10,
                        min: -999999,
                        max: 999999,
                        step: 1
                    },
                    {
                        type: "field_dropdown",
                        name: "TYPE",
                        options: [
                            [getText("mathRandomInteger"), "Integer"],
                            [getText("mathRandomDecimal"), "Decimal"]
                        ]
                    }
                ],
                output: "Number",
                colour: MATH_COLOR, // 数学积木：蓝色
                tooltip: getText("mathRandomTooltip"),
                helpUrl: "",
            });
            
            // 添加自定义验证逻辑
            this.setOnChange(function(changeEvent) {
                if (changeEvent.type === "change" && 
                    (changeEvent.name === "FROM" || changeEvent.name === "TO" || changeEvent.name === "TYPE")) {
                    this.validateRandomBlock();
                }
            });
        },
        
        validateRandomBlock: function() {
            const fromField = this.getField("FROM");
            const toField = this.getField("TO");
            const typeField = this.getField("TYPE");
            
            if (!fromField || !toField || !typeField) return;
            
            const fromValue = parseFloat(fromField.getValue());
            const toValue = parseFloat(toField.getValue());
            const typeValue = typeField.getValue();
            
            let isValid = true;
            
            // 验证范围：后面的数字必须大于或等于前面的数字
            if (toValue < fromValue) {
                isValid = false;
            }
            
            // 验证整数类型：只能输入整数
            if (typeValue === "Integer") {
                if (!Number.isInteger(fromValue) || !Number.isInteger(toValue)) {
                    isValid = false;
                }
            }
            
            // 设置字段颜色
            const errorColor = "#FF0000"; // 红色
            const normalColor = "#FFFFFF"; // 白色
            
            fromField.setColour(isValid ? normalColor : errorColor);
            toField.setColour(isValid ? normalColor : errorColor);
        }
    };
}
