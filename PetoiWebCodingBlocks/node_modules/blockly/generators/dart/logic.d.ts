/**
 * @license
 * Copyright 2014 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 * @file Generating Dart for logic blocks.
 */
import type { Block } from '../../core/block.js';
import type { DartGenerator } from './dart_generator.js';
import { Order } from './dart_generator.js';
export declare function controls_if(block: Block, generator: DartGenerator): string;
export declare const controls_ifelse: typeof controls_if;
export declare function logic_compare(block: Block, generator: DartGenerator): [string, Order];
export declare function logic_operation(block: Block, generator: DartGenerator): [string, Order];
export declare function logic_negate(block: Block, generator: DartGenerator): [string, Order];
export declare function logic_boolean(block: Block, generator: DartGenerator): [string, Order];
export declare function logic_null(block: Block, generator: DartGenerator): [string, Order];
export declare function logic_ternary(block: Block, generator: DartGenerator): [string, Order];
//# sourceMappingURL=logic.d.ts.map