/**
 * @license
 * Copyright 2015 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */
import type { Block } from '../../core/block.js';
import type { PhpGenerator } from './php_generator.js';
export declare function controls_repeat_ext(block: Block, generator: PhpGenerator): string;
export declare const controls_repeat: typeof controls_repeat_ext;
export declare function controls_whileUntil(block: Block, generator: PhpGenerator): string;
export declare function controls_for(block: Block, generator: PhpGenerator): string;
export declare function controls_forEach(block: Block, generator: PhpGenerator): string;
export declare function controls_flow_statements(block: Block, generator: PhpGenerator): string;
//# sourceMappingURL=loops.d.ts.map