/**
 * @license
 * Copyright 2012 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 * @file Generating JavaScript for math blocks.
 */
import type { Block } from '../../core/block.js';
import type { JavascriptGenerator } from './javascript_generator.js';
import { Order } from './javascript_generator.js';
export declare function math_number(block: Block, generator: JavascriptGenerator): [string, Order];
export declare function math_arithmetic(block: Block, generator: JavascriptGenerator): [string, Order];
export declare function math_single(block: Block, generator: JavascriptGenerator): [string, Order];
export declare function math_constant(block: Block, generator: JavascriptGenerator): [string, Order];
export declare function math_number_property(block: Block, generator: JavascriptGenerator): [string, Order];
export declare function math_change(block: Block, generator: JavascriptGenerator): string;
export declare const math_round: typeof math_single;
export declare const math_trig: typeof math_single;
export declare function math_on_list(block: Block, generator: JavascriptGenerator): [string, Order];
export declare function math_modulo(block: Block, generator: JavascriptGenerator): [string, Order];
export declare function math_constrain(block: Block, generator: JavascriptGenerator): [string, Order];
export declare function math_random_int(block: Block, generator: JavascriptGenerator): [string, Order];
export declare function math_random_float(block: Block, generator: JavascriptGenerator): [string, Order];
export declare function math_atan2(block: Block, generator: JavascriptGenerator): [string, Order];
//# sourceMappingURL=math.d.ts.map