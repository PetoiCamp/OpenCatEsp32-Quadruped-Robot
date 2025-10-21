/**
 * @license
 * Copyright 2016 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 * @file Generating Lua for procedure blocks.
 */
import type { Block } from '../../core/block.js';
import type { LuaGenerator } from './lua_generator.js';
import { Order } from './lua_generator.js';
export declare function procedures_defreturn(block: Block, generator: LuaGenerator): null;
export declare const procedures_defnoreturn: typeof procedures_defreturn;
export declare function procedures_callreturn(block: Block, generator: LuaGenerator): [string, Order];
export declare function procedures_callnoreturn(block: Block, generator: LuaGenerator): string;
export declare function procedures_ifreturn(block: Block, generator: LuaGenerator): string;
//# sourceMappingURL=procedures.d.ts.map