/**
 * @license
 * Copyright 2014 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 * @file Generating Dart for list blocks.
 */
import type { Block } from '../../core/block.js';
import type { DartGenerator } from './dart_generator.js';
import { Order } from './dart_generator.js';
export declare function lists_create_empty(block: Block, generator: DartGenerator): [string, Order];
export declare function lists_create_with(block: Block, generator: DartGenerator): [string, Order];
export declare function lists_repeat(block: Block, generator: DartGenerator): [string, Order];
export declare function lists_length(block: Block, generator: DartGenerator): [string, Order];
export declare function lists_isEmpty(block: Block, generator: DartGenerator): [string, Order];
export declare function lists_indexOf(block: Block, generator: DartGenerator): [string, Order];
export declare function lists_getIndex(block: Block, generator: DartGenerator): [string, Order] | string;
export declare function lists_setIndex(block: Block, generator: DartGenerator): string;
export declare function lists_getSublist(block: Block, generator: DartGenerator): [string, Order];
export declare function lists_sort(block: Block, generator: DartGenerator): [string, Order];
export declare function lists_split(block: Block, generator: DartGenerator): [string, Order];
export declare function lists_reverse(block: Block, generator: DartGenerator): [string, Order];
//# sourceMappingURL=lists.d.ts.map