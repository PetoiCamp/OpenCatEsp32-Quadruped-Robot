/**
 * @license
 * Copyright 2015 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */
import type { Block } from '../../core/block.js';
import { CodeGenerator } from '../../core/generator.js';
import type { Workspace } from '../../core/workspace.js';
/**
 * Order of operation ENUMs.
 * http://php.net/manual/en/language.operators.precedence.php
 */
export declare enum Order {
    ATOMIC = 0,// 0 "" ...
    CLONE = 1,// clone
    NEW = 1,// new
    MEMBER = 2.1,// []
    FUNCTION_CALL = 2.2,// ()
    POWER = 3,// **
    INCREMENT = 4,// ++
    DECREMENT = 4,// --
    BITWISE_NOT = 4,// ~
    CAST = 4,// (int) (float) (string) (array) ...
    SUPPRESS_ERROR = 4,// @
    INSTANCEOF = 5,// instanceof
    LOGICAL_NOT = 6,// !
    UNARY_PLUS = 7.1,// +
    UNARY_NEGATION = 7.2,// -
    MULTIPLICATION = 8.1,// *
    DIVISION = 8.2,// /
    MODULUS = 8.3,// %
    ADDITION = 9.1,// +
    SUBTRACTION = 9.2,// -
    STRING_CONCAT = 9.3,// .
    BITWISE_SHIFT = 10,// << >>
    RELATIONAL = 11,// < <= > >=
    EQUALITY = 12,// == != === !== <> <=>
    REFERENCE = 13,// &
    BITWISE_AND = 13,// &
    BITWISE_XOR = 14,// ^
    BITWISE_OR = 15,// |
    LOGICAL_AND = 16,// &&
    LOGICAL_OR = 17,// ||
    IF_NULL = 18,// ??
    CONDITIONAL = 19,// ?:
    ASSIGNMENT = 20,// = += -= *= /= %= <<= >>= ...
    LOGICAL_AND_WEAK = 21,// and
    LOGICAL_XOR = 22,// xor
    LOGICAL_OR_WEAK = 23,// or
    NONE = 99
}
export declare class PhpGenerator extends CodeGenerator {
    /** List of outer-inner pairings that do NOT require parentheses. */
    ORDER_OVERRIDES: [Order, Order][];
    /** @param name Name of the language the generator is for. */
    constructor(name?: string);
    /**
     * Initialise the database of variable names.
     *
     * @param workspace Workspace to generate code from.
     */
    init(workspace: Workspace): void;
    /**
     * Prepend the generated code with the variable definitions.
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
     * Encode a string as a properly escaped PHP string, complete with
     * quotes.
     *
     * @param string Text to encode.
     * @returns PHP string.
     */
    quote_(string: string): string;
    /**
     * Encode a string as a properly escaped multiline PHP string, complete with
     * quotes.
     * @param string Text to encode.
     * @returns PHP string.
     */
    multiline_quote_(string: string): string;
    /**
     * Common tasks for generating PHP from blocks.
     * Handles comments for the specified block and any connected value blocks.
     * Calls any statements following this block.
     *
     * @param block The current block.
     * @param code The PHP code created for this block.
     * @param thisOnly True to generate code for only this statement.
     * @returns PHP code with comments and subsequent blocks added.
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
//# sourceMappingURL=php_generator.d.ts.map