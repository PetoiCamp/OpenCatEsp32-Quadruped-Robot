/**
 * @license
 * Copyright 2015 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */
import type { Block } from '../../core/block.js';
import { Order } from './php_generator.js';
import type { PhpGenerator } from './php_generator.js';
export declare function procedures_defreturn(block: Block, generator: PhpGenerator): null;
export declare const procedures_defnoreturn: typeof procedures_defreturn;
export declare function procedures_callreturn(block: Block, generator: PhpGenerator): [string, Order];
export declare function procedures_callnoreturn(block: Block, generator: PhpGenerator): string;
export declare function procedures_ifreturn(block: Block, generator: PhpGenerator): string;
//# sourceMappingURL=procedures.d.ts.map