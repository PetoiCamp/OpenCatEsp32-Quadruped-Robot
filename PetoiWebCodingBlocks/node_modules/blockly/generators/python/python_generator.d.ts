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
 * http://docs.python.org/reference/expressions.html#summary
 */
export declare enum Order {
    ATOMIC = 0,// 0 "" ...
    COLLECTION = 1,// tuples, lists, dictionaries
    STRING_CONVERSION = 1,// `expression...`
    MEMBER = 2.1,// . []
    FUNCTION_CALL = 2.2,// ()
    EXPONENTIATION = 3,// **
    UNARY_SIGN = 4,// + -
    BITWISE_NOT = 4,// ~
    MULTIPLICATIVE = 5,// * / // %
    ADDITIVE = 6,// + -
    BITWISE_SHIFT = 7,// << >>
    BITWISE_AND = 8,// &
    BITWISE_XOR = 9,// ^
    BITWISE_OR = 10,// |
    RELATIONAL = 11,// in, not in, is, is not, >, >=, <>, !=, ==
    LOGICAL_NOT = 12,// not
    LOGICAL_AND = 13,// and
    LOGICAL_OR = 14,// or
    CONDITIONAL = 15,// if else
    LAMBDA = 16,// lambda
    NONE = 99
}
/**
 * PythonScript code generator class.
 */
export declare class PythonGenerator extends CodeGenerator {
    /** List of outer-inner pairings that do NOT require parentheses. */
    ORDER_OVERRIDES: [Order, Order][];
    /**
     * Empty loops or conditionals are not allowed in Python.
     */
    PASS: string;
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
     * Encode a string as a properly escaped Python string, complete with quotes.
     *
     * @param string Text to encode.
     * @returns Python string.
     */
    quote_(string: string): string;
    /**
     * Encode a string as a properly escaped multiline Python string, complete
     * with quotes.
     *
     * @param string Text to encode.
     * @returns Python string.
     */
    multiline_quote_(string: string): string;
    /**
     * Common tasks for generating Python from blocks.
     * Handles comments for the specified block and any connected value blocks.
     * Calls any statements following this block.
     *
     * @param block The current block.
     * @param code The Python code created for this block.
     * @param thisOnly True to generate code for only this statement.
     * @returns Python code with comments and subsequent blocks added.
     */
    scrub_(block: Block, code: string, thisOnly?: boolean): string;
    /**
     * Gets a property and adjusts the value, taking into account indexing.
     * If a static int, casts to an integer, otherwise returns a code string.
     *
     * @param block The block.
     * @param atId The ID of the input block to get (and adjust) the value of.
     * @param delta Value to add.
     * @param negate Whether to negate the value.
     * @returns The adjusted value or code that evaluates to it.
     */
    getAdjustedInt(block: Block, atId: string, delta?: number, negate?: boolean): string | number;
}
//# sourceMappingURL=python_generator.d.ts.map