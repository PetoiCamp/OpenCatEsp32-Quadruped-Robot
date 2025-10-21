/**
 * @license
 * Copyright 2012 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */
import type { Block } from '../../core/block.js';
import { Order } from './python_generator.js';
import type { PythonGenerator } from './python_generator.js';
export declare function procedures_defreturn(block: Block, generator: PythonGenerator): null;
export declare const procedures_defnoreturn: typeof procedures_defreturn;
export declare function procedures_callreturn(block: Block, generator: PythonGenerator): [string, Order];
export declare function procedures_callnoreturn(block: Block, generator: PythonGenerator): string;
export declare function procedures_ifreturn(block: Block, generator: PythonGenerator): string;
//# sourceMappingURL=procedures.d.ts.map