/**
 * @license
 * Copyright 2012 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 * @file Generating JavaScript for text blocks.
 */
import type { Block } from '../../core/block.js';
import type { JavascriptGenerator } from './javascript_generator.js';
import { Order } from './javascript_generator.js';
export declare function text(block: Block, generator: JavascriptGenerator): [string, Order];
export declare function text_join(block: Block, generator: JavascriptGenerator): [string, Order];
export declare function text_append(block: Block, generator: JavascriptGenerator): string;
export declare function text_length(block: Block, generator: JavascriptGenerator): [string, Order];
export declare function text_isEmpty(block: Block, generator: JavascriptGenerator): [string, Order];
export declare function text_indexOf(block: Block, generator: JavascriptGenerator): [string, Order];
export declare function text_charAt(block: Block, generator: JavascriptGenerator): [string, Order];
export declare function text_getSubstring(block: Block, generator: JavascriptGenerator): [string, Order];
export declare function text_changeCase(block: Block, generator: JavascriptGenerator): [string, Order];
export declare function text_trim(block: Block, generator: JavascriptGenerator): [string, Order];
export declare function text_print(block: Block, generator: JavascriptGenerator): string;
export declare function text_prompt_ext(block: Block, generator: JavascriptGenerator): [string, Order];
export declare const text_prompt: typeof text_prompt_ext;
export declare function text_count(block: Block, generator: JavascriptGenerator): [string, Order];
export declare function text_replace(block: Block, generator: JavascriptGenerator): [string, Order];
export declare function text_reverse(block: Block, generator: JavascriptGenerator): [string, Order];
//# sourceMappingURL=text.d.ts.map