/**
 * @license
 * Copyright 2016 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 * @file Generating Lua for logic blocks.
 */
import type { Block } from '../../core/block.js';
import type { LuaGenerator } from './lua_generator.js';
import { Order } from './lua_generator.js';
export declare function controls_if(block: Block, generator: LuaGenerator): string;
export declare const controls_ifelse: typeof controls_if;
export declare function logic_compare(block: Block, generator: LuaGenerator): [string, Order];
export declare function logic_operation(block: Block, generator: LuaGenerator): [string, Order];
export declare function logic_negate(block: Block, generator: LuaGenerator): [string, Order];
export declare function logic_boolean(block: Block, generator: LuaGenerator): [string, Order];
export declare function logic_null(block: Block, generator: LuaGenerator): [string, Order];
export declare function logic_ternary(block: Block, generator: LuaGenerator): [string, Order];
//# sourceMappingURL=logic.d.ts.map