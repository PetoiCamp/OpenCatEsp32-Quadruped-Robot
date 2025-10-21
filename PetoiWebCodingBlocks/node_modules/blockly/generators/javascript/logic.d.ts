/**
 * @license
 * Copyright 2012 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 * @file Generating JavaScript for logic blocks.
 */
import type { Block } from '../../core/block.js';
import type { JavascriptGenerator } from './javascript_generator.js';
import { Order } from './javascript_generator.js';
export declare function controls_if(block: Block, generator: JavascriptGenerator): string;
export declare const controls_ifelse: typeof controls_if;
export declare function logic_compare(block: Block, generator: JavascriptGenerator): [string, Order];
export declare function logic_operation(block: Block, generator: JavascriptGenerator): [string, Order];
export declare function logic_negate(block: Block, generator: JavascriptGenerator): [string, Order];
export declare function logic_boolean(block: Block, generator: JavascriptGenerator): [string, Order];
export declare function logic_null(block: Block, generator: JavascriptGenerator): [string, Order];
export declare function logic_ternary(block: Block, generator: JavascriptGenerator): [string, Order];
//# sourceMappingURL=logic.d.ts.map