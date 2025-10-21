/**
 * @license
 * Copyright 2016 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 * @file Generating Lua for variable blocks.
 */
import type { Block } from '../../core/block.js';
import type { LuaGenerator } from './lua_generator.js';
import { Order } from './lua_generator.js';
export declare function variables_get(block: Block, generator: LuaGenerator): [string, Order];
export declare function variables_set(block: Block, generator: LuaGenerator): string;
//# sourceMappingURL=variables.d.ts.map