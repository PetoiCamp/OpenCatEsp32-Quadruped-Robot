/**
 * @license
 * Copyright 2012 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */
import type { Block } from '../../core/block.js';
import { Order } from './python_generator.js';
import type { PythonGenerator } from './python_generator.js';
export declare function lists_create_empty(block: Block, generator: PythonGenerator): [string, Order];
export declare function lists_create_with(block: Block, generator: PythonGenerator): [string, Order];
export declare function lists_repeat(block: Block, generator: PythonGenerator): [string, Order];
export declare function lists_length(block: Block, generator: PythonGenerator): [string, Order];
export declare function lists_isEmpty(block: Block, generator: PythonGenerator): [string, Order];
export declare function lists_indexOf(block: Block, generator: PythonGenerator): [string, Order];
export declare function lists_getIndex(block: Block, generator: PythonGenerator): [string, Order] | string;
export declare function lists_setIndex(block: Block, generator: PythonGenerator): string;
export declare function lists_getSublist(block: Block, generator: PythonGenerator): [string, Order];
export declare function lists_sort(block: Block, generator: PythonGenerator): [string, Order];
export declare function lists_split(block: Block, generator: PythonGenerator): [string, Order];
export declare function lists_reverse(block: Block, generator: PythonGenerator): [string, Order];
//# sourceMappingURL=lists.d.ts.map