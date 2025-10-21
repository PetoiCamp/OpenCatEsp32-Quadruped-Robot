/**
 * @license
 * Copyright 2012 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 * @file Generating JavaScript for list blocks.
 */
import type { Block } from '../../core/block.js';
import type { JavascriptGenerator } from './javascript_generator.js';
import { Order } from './javascript_generator.js';
export declare function lists_create_empty(block: Block, generator: JavascriptGenerator): [string, Order];
export declare function lists_create_with(block: Block, generator: JavascriptGenerator): [string, Order];
export declare function lists_repeat(block: Block, generator: JavascriptGenerator): [string, Order];
export declare function lists_length(block: Block, generator: JavascriptGenerator): [string, Order];
export declare function lists_isEmpty(block: Block, generator: JavascriptGenerator): [string, Order];
export declare function lists_indexOf(block: Block, generator: JavascriptGenerator): [string, Order];
export declare function lists_getIndex(block: Block, generator: JavascriptGenerator): [string, Order] | string;
export declare function lists_setIndex(block: Block, generator: JavascriptGenerator): string;
export declare function lists_getSublist(block: Block, generator: JavascriptGenerator): [string, Order];
export declare function lists_sort(block: Block, generator: JavascriptGenerator): [string, Order];
export declare function lists_split(block: Block, generator: JavascriptGenerator): [string, Order];
export declare function lists_reverse(block: Block, generator: JavascriptGenerator): [string, Order];
//# sourceMappingURL=lists.d.ts.map