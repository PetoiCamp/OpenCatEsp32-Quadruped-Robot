/**
 * @license
 * Copyright 2012 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 * @file Generating JavaScript for procedure blocks.
 */
import type { Block } from '../../core/block.js';
import type { JavascriptGenerator } from './javascript_generator.js';
import { Order } from './javascript_generator.js';
export declare function procedures_defreturn(block: Block, generator: JavascriptGenerator): null;
export declare const procedures_defnoreturn: typeof procedures_defreturn;
export declare function procedures_callreturn(block: Block, generator: JavascriptGenerator): [string, Order];
export declare function procedures_callnoreturn(block: Block, generator: JavascriptGenerator): string;
export declare function procedures_ifreturn(block: Block, generator: JavascriptGenerator): string;
//# sourceMappingURL=procedures.d.ts.map