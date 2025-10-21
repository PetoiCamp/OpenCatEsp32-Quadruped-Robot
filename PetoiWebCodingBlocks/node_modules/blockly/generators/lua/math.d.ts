/**
 * @license
 * Copyright 2016 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 * @file Generating Lua for math blocks.
 */
import type { Block } from '../../core/block.js';
import type { LuaGenerator } from './lua_generator.js';
import { Order } from './lua_generator.js';
export declare function math_number(block: Block, generator: LuaGenerator): [string, Order];
export declare function math_arithmetic(block: Block, generator: LuaGenerator): [string, Order];
export declare function math_single(block: Block, generator: LuaGenerator): [string, Order];
export declare function math_constant(block: Block, generator: LuaGenerator): [string, Order];
export declare function math_number_property(block: Block, generator: LuaGenerator): [string, Order];
export declare function math_change(block: Block, generator: LuaGenerator): string;
export declare const math_round: typeof math_single;
export declare const math_trig: typeof math_single;
export declare function math_on_list(block: Block, generator: LuaGenerator): [string, Order];
export declare function math_modulo(block: Block, generator: LuaGenerator): [string, Order];
export declare function math_constrain(block: Block, generator: LuaGenerator): [string, Order];
export declare function math_random_int(block: Block, generator: LuaGenerator): [string, Order];
export declare function math_random_float(block: Block, generator: LuaGenerator): [string, Order];
export declare function math_atan2(block: Block, generator: LuaGenerator): [string, Order];
//# sourceMappingURL=math.d.ts.map