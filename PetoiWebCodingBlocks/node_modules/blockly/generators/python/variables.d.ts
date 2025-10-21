/**
 * @license
 * Copyright 2012 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 * @file Generating Python for variable blocks.
 */
import type { Block } from '../../core/block.js';
import type { PythonGenerator } from './python_generator.js';
import { Order } from './python_generator.js';
export declare function variables_get(block: Block, generator: PythonGenerator): [string, Order];
export declare function variables_set(block: Block, generator: PythonGenerator): string;
//# sourceMappingURL=variables.d.ts.map