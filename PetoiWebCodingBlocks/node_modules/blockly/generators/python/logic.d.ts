/**
 * @license
 * Copyright 2012 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 * @file Generating Python for logic blocks.
 */
import type { Block } from '../../core/block.js';
import type { PythonGenerator } from './python_generator.js';
import { Order } from './python_generator.js';
export declare function controls_if(block: Block, generator: PythonGenerator): string;
export declare const controls_ifelse: typeof controls_if;
export declare function logic_compare(block: Block, generator: PythonGenerator): [string, Order];
export declare function logic_operation(block: Block, generator: PythonGenerator): [string, Order];
export declare function logic_negate(block: Block, generator: PythonGenerator): [string, Order];
export declare function logic_boolean(block: Block, generator: PythonGenerator): [string, Order];
export declare function logic_null(block: Block, generator: PythonGenerator): [string, Order];
export declare function logic_ternary(block: Block, generator: PythonGenerator): [string, Order];
//# sourceMappingURL=logic.d.ts.map