/**
 * @license
 * Copyright 2014 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 * @file Generating Dart for variable blocks.
 */
import type { Block } from '../../core/block.js';
import type { DartGenerator } from './dart_generator.js';
import { Order } from './dart_generator.js';
export declare function variables_get(block: Block, generator: DartGenerator): [string, Order];
export declare function variables_set(block: Block, generator: DartGenerator): string;
//# sourceMappingURL=variables.d.ts.map