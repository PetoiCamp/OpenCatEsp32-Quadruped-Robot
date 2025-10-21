/**
 * @license
 * Copyright 2014 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 * @file Generating Dart for procedure blocks.
 */
import type { Block } from '../../core/block.js';
import type { DartGenerator } from './dart_generator.js';
import { Order } from './dart_generator.js';
export declare function procedures_defreturn(block: Block, generator: DartGenerator): null;
export declare const procedures_defnoreturn: typeof procedures_defreturn;
export declare function procedures_callreturn(block: Block, generator: DartGenerator): [string, Order];
export declare function procedures_callnoreturn(block: Block, generator: DartGenerator): string;
export declare function procedures_ifreturn(block: Block, generator: DartGenerator): string;
//# sourceMappingURL=procedures.d.ts.map