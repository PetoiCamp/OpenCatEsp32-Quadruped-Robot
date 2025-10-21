/**
 * @license
 * Copyright 2016 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 * @file Generating Lua for list blocks.
 */
import type { Block } from '../../core/block.js';
import type { LuaGenerator } from './lua_generator.js';
import { Order } from './lua_generator.js';
export declare function lists_create_empty(block: Block, generator: LuaGenerator): [string, Order];
export declare function lists_create_with(block: Block, generator: LuaGenerator): [string, Order];
export declare function lists_repeat(block: Block, generator: LuaGenerator): [string, Order];
export declare function lists_length(block: Block, generator: LuaGenerator): [string, Order];
export declare function lists_isEmpty(block: Block, generator: LuaGenerator): [string, Order];
export declare function lists_indexOf(block: Block, generator: LuaGenerator): [string, Order];
export declare function lists_getIndex(block: Block, generator: LuaGenerator): [string, Order] | string;
export declare function lists_setIndex(block: Block, generator: LuaGenerator): string;
export declare function lists_getSublist(block: Block, generator: LuaGenerator): [string, Order];
export declare function lists_sort(block: Block, generator: LuaGenerator): [string, Order];
export declare function lists_split(block: Block, generator: LuaGenerator): [string, Order];
export declare function lists_reverse(block: Block, generator: LuaGenerator): [string, Order];
//# sourceMappingURL=lists.d.ts.map