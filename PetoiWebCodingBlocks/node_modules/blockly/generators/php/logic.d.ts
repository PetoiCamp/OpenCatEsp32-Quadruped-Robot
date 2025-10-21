/**
 * @license
 * Copyright 2015 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 * @file Generating PHP for logic blocks.
 */
import type { Block } from '../../core/block.js';
import { Order } from './php_generator.js';
import type { PhpGenerator } from './php_generator.js';
export declare function controls_if(block: Block, generator: PhpGenerator): string;
export declare const controls_ifelse: typeof controls_if;
export declare function logic_compare(block: Block, generator: PhpGenerator): [string, Order];
export declare function logic_operation(block: Block, generator: PhpGenerator): [string, Order];
export declare function logic_negate(block: Block, generator: PhpGenerator): [string, Order];
export declare function logic_boolean(block: Block, generator: PhpGenerator): [string, Order];
export declare function logic_null(block: Block, generator: PhpGenerator): [string, Order];
export declare function logic_ternary(block: Block, generator: PhpGenerator): [string, Order];
//# sourceMappingURL=logic.d.ts.map