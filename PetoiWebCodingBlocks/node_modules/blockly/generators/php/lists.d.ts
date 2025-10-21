/**
 * @license
 * Copyright 2015 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */
import type { Block } from '../../core/block.js';
import { Order } from './php_generator.js';
import type { PhpGenerator } from './php_generator.js';
export declare function lists_create_empty(block: Block, generator: PhpGenerator): [string, Order];
export declare function lists_create_with(block: Block, generator: PhpGenerator): [string, Order];
export declare function lists_repeat(block: Block, generator: PhpGenerator): [string, Order];
export declare function lists_length(block: Block, generator: PhpGenerator): [string, Order];
export declare function lists_isEmpty(block: Block, generator: PhpGenerator): [string, Order];
export declare function lists_indexOf(block: Block, generator: PhpGenerator): [string, Order];
export declare function lists_getIndex(block: Block, generator: PhpGenerator): [string, Order] | string;
export declare function lists_setIndex(block: Block, generator: PhpGenerator): string;
export declare function lists_getSublist(block: Block, generator: PhpGenerator): [string, Order];
export declare function lists_sort(block: Block, generator: PhpGenerator): [string, Order];
export declare function lists_split(block: Block, generator: PhpGenerator): [string, Order];
export declare function lists_reverse(block: Block, generator: PhpGenerator): [string, Order];
//# sourceMappingURL=lists.d.ts.map