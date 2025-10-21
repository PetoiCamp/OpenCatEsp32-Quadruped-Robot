/**
 * @license
 * Copyright 2015 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 * @file Generating PHP for text blocks.
 */
import type { Block } from '../../core/block.js';
import { Order } from './php_generator.js';
import type { PhpGenerator } from './php_generator.js';
export declare function text(block: Block, generator: PhpGenerator): [string, Order];
export declare function text_join(block: Block, generator: PhpGenerator): [string, Order];
export declare function text_append(block: Block, generator: PhpGenerator): string;
export declare function text_length(block: Block, generator: PhpGenerator): [string, Order];
export declare function text_isEmpty(block: Block, generator: PhpGenerator): [string, Order];
export declare function text_indexOf(block: Block, generator: PhpGenerator): [string, Order];
export declare function text_charAt(block: Block, generator: PhpGenerator): [string, Order];
export declare function text_getSubstring(block: Block, generator: PhpGenerator): [string, Order];
export declare function text_changeCase(block: Block, generator: PhpGenerator): [string, Order];
export declare function text_trim(block: Block, generator: PhpGenerator): [string, Order];
export declare function text_print(block: Block, generator: PhpGenerator): string;
export declare function text_prompt_ext(block: Block, generator: PhpGenerator): [string, Order];
export declare const text_prompt: typeof text_prompt_ext;
export declare function text_count(block: Block, generator: PhpGenerator): [string, Order];
export declare function text_replace(block: Block, generator: PhpGenerator): [string, Order];
export declare function text_reverse(block: Block, generator: PhpGenerator): [string, Order];
//# sourceMappingURL=text.d.ts.map