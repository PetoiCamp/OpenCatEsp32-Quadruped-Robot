/**
 * @license
 * Copyright 2016 Google LLC
 * SPDX-License-Identifier: Apache-2.0
 */
import type { Block } from '../../core/block.js';
import { CodeGenerator } from '../../core/generator.js';
import type { Workspace } from '../../core/workspace.js';
/**
 * Order of operation ENUMs.
 * http://www.lua.org/manual/5.3/manual.html#3.4.8
 */
export declare enum Order {
    ATOMIC = 0,// literals
    HIGH = 1,// Function calls, tables[]
    EXPONENTIATION = 2,// ^
    UNARY = 3,// not # - ~
    MULTIPLICATIVE = 4,// * / %
    ADDITIVE = 5,// + -
    CONCATENATION = 6,// ..
    RELATIONAL = 7,// < > <=  >= ~= ==
    AND = 8,// and
    OR = 9,// or
    NONE = 99
}
/**
 * Lua code generator class.
 *
 * Note: Lua is not supporting zero-indexing since the language itself is
 * one-indexed, so the generator does not repoct the oneBasedIndex configuration
 * option used for lists and text.
 */
export declare class LuaGenerator extends CodeGenerator {
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
     * anything. In Lua, an expression is not a legal statement, so we must assign
     * the value to the (conventionally ignored) _.
     * http://lua-users.org/wiki/ExpressionsAsStatements
     *
     * @param line Line of generated code.
     * @return Legal line of code.
     */
    scrubNakedValue(line: string): string;
    /**
     * Encode a string as a properly escaped Lua string, complete with
     * quotes.
     *
     * @param string Text to encode.
     * @returns Lua string.
     */
    quote_(string: string): string;
    /**
     * Encode a string as a properly escaped multiline Lua string, complete with
     * quotes.
     *
     * @param string Text to encode.
     * @returns Lua string.
     */
    multiline_quote_(string: string): string;
    /**
     * Common tasks for generating Lua from blocks.
     * Handles comments for the specified block and any connected value blocks.
     * Calls any statements following this block.
     * @param block The current block.
     * @param code The Lua code created for this block.
     * @param thisOnly True to generate code for only this statement.
     * @returns Lua code with comments and subsequent blocks added.
     */
    scrub_(block: Block, code: string, thisOnly?: boolean): string;
}
//# sourceMappingURL=lua_generator.d.ts.map