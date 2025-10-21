/**
 * @license
 * Copyright 2015 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 * @file Generating PHP for variable blocks.
 */
import type { Block } from '../../core/block.js';
import { Order } from './php_generator.js';
import type { PhpGenerator } from './php_generator.js';
export declare function variables_get(block: Block, generator: PhpGenerator): [string, Order];
export declare function variables_set(block: Block, generator: PhpGenerator): string;
//# sourceMappingURL=variables.d.ts.map