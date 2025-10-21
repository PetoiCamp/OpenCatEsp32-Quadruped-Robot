/**
 * @license
 * Copyright 2012 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 * @file Generating JavaScript for variable blocks.
 */
import type { Block } from '../../core/block.js';
import type { JavascriptGenerator } from './javascript_generator.js';
import { Order } from './javascript_generator.js';
export declare function variables_get(block: Block, generator: JavascriptGenerator): [string, Order];
export declare function variables_set(block: Block, generator: JavascriptGenerator): string;
//# sourceMappingURL=variables.d.ts.map