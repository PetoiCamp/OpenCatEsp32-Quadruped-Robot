/**
 * @license
 * Copyright 2012 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */
import type { Block } from '../../core/block.js';
import { CodeGenerator } from '../../core/generator.js';
import type { Workspace } from '../../core/workspace.js';
/**
 * Order of operation ENUMs.
 * https://developer.mozilla.org/en/JavaScript/Reference/Operators/Operator_Precedence
 */
export declare enum Order {
    ATOMIC = 0,// 0 "" ...
    NEW = 1.1,// new
    MEMBER = 1.2,// . []
    FUNCTION_CALL = 2,// ()
    INCREMENT = 3,// ++
    DECREMENT = 3,// --
    BITWISE_NOT = 4.1,// ~
    UNARY_PLUS = 4.2,// +
    UNARY_NEGATION = 4.3,// -
    LOGICAL_NOT = 4.4,// !
    TYPEOF = 4.5,// typeof
    VOID = 4.6,// void
    DELETE = 4.7,// delete
    AWAIT = 4.8,// await
    EXPONENTIATION = 5,// **
    MULTIPLICATION = 5.1,// *
    DIVISION = 5.2,// /
    MODULUS = 5.3,// %
    SUBTRACTION = 6.1,// -
    ADDITION = 6.2,// +
    BITWISE_SHIFT = 7,// << >> >>>
    RELATIONAL = 8,// < <= > >=
    IN = 8,// in
    INSTANCEOF = 8,// instanceof
    EQUALITY = 9,// == != === !==
    BITWISE_AND = 10,// &
    BITWISE_XOR = 11,// ^
    BITWISE_OR = 12,// |
    LOGICAL_AND = 13,// &&
    LOGICAL_OR = 14,// ||
    CONDITIONAL = 15,// ?:
    ASSIGNMENT = 16,// = += -= **= *= /= %= <<= >>= ...
    YIELD = 17,// yield
    COMMA = 18,// ,
    NONE = 99
}
/**
 * JavaScript code generator class.
 */
export declare class JavascriptGenerator extends CodeGenerator {
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
     * anything.  A trailing semicolon is needed to make this legal.
     *
     * @param line Line of generated code.
     * @returns Legal line of code.
     */
    scrubNakedValue(line: string): string;
    /**
     * Encode a string as a properly escaped JavaScript string, complete with
     * quotes.
     *
     * @param string Text to encode.
     * @returns JavaScript string.
     */
    quote_(string: string): string;
    /**
     * Encode a string as a properly escaped multiline JavaScript string, complete
     * with quotes.
     * @param string Text to encode.
     * @returns JavaScript string.
     */
    multiline_quote_(string: string): string;
    /**
     * Common tasks for generating JavaScript from blocks.
     * Handles comments for the specified block and any connected value blocks.
     * Calls any statements following this block.
     *
     * @param block The current block.
     * @param code The JavaScript code created for this block.
     * @param thisOnly True to generate code for only this statement.
     * @returns JavaScript code with comments and subsequent blocks added.
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
//# sourceMappingURL=javascript_generator.d.ts.map