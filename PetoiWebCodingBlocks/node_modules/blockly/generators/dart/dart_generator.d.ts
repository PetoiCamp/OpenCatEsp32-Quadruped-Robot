/**
 * @license
 * Copyright 2014 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */
import type { Block } from '../../core/block.js';
import { CodeGenerator } from '../../core/generator.js';
import type { Workspace } from '../../core/workspace.js';
/**
 * Order of operation ENUMs.
 * https://dart.dev/guides/language/language-tour#operators
 */
export declare enum Order {
    ATOMIC = 0,// 0 "" ...
    UNARY_POSTFIX = 1,// expr++ expr-- () [] . ?.
    UNARY_PREFIX = 2,// -expr !expr ~expr ++expr --expr
    MULTIPLICATIVE = 3,// * / % ~/
    ADDITIVE = 4,// + -
    SHIFT = 5,// << >>
    BITWISE_AND = 6,// &
    BITWISE_XOR = 7,// ^
    BITWISE_OR = 8,// |
    RELATIONAL = 9,// >= > <= < as is is!
    EQUALITY = 10,// == !=
    LOGICAL_AND = 11,// &&
    LOGICAL_OR = 12,// ||
    IF_NULL = 13,// ??
    CONDITIONAL = 14,// expr ? expr: expr
    CASCADE = 15,// ..
    ASSIGNMENT = 16,// = *= /= ~/= %= += -= <<= >>= &= ^= |=
    NONE = 99
}
/**
 * Dart code generator class.
 */
export declare class DartGenerator extends CodeGenerator {
    /** @param name Name of the language the generator is for. */
    constructor(name?: string);
    /**
     * Initialise the database of variable names.
     *
     * @param workspace Workspace to generate code from.
     */
    init(workspace: Workspace): void;
    /**
     * Prepend the generated code with import statements and variable definitions.
     *
     * @param code Generated code.
     * @returns Completed code.
     */
    finish(code: string): string;
    /**
     * Naked values are top-level blocks with outputs that aren't plugged into
     * anything.
     *
     * @param line Line of generated code.
     * @returns Legal line of code.
     */
    scrubNakedValue(line: string): string;
    /**
     * Encode a string as a properly escaped Dart string, complete with quotes.
     *
     * @param string Text to encode.
     * @returns Dart string.
     */
    quote_(string: string): string;
    /**
     * Encode a string as a properly escaped multiline Dart string, complete
     * with quotes.
     *
     * @param string Text to encode.
     * @returns Dart string.
     */
    multiline_quote_(string: string): string;
    /**
     * Common tasks for generating Dart from blocks.
     * Handles comments for the specified block and any connected value blocks.
     * Calls any statements following this block.
     *
     * @param block The current block.
     * @param code The Dart code created for this block.
     * @param thisOnly True to generate code for only this statement.
     * @returns Dart code with comments and subsequent blocks added.
     */
    scrub_(block: Block, code: string, thisOnly?: boolean): string;
    /**
     * Generate code representing the specified value input, adjusted to take into
     * account indexing (zero- or one-based) and optionally by a specified delta
     * and/or by negation.
     *
     * @param block The block.
     * @param atId The ID of the input block to get (and adjust) the value of.
     * @param delta Value to add.
     * @param negate Whether to negate the value.
     * @param order The highest order acting on this value.
     * @returns The adjusted value or code that evaluates to it.
     */
    getAdjusted(block: Block, atId: string, delta?: number, negate?: boolean, order?: Order): string;
}
//# sourceMappingURL=dart_generator.d.ts.map