import { Transport } from "./webserial.js";
/**
 * Set of reset functions for ESP Loader connection.
 * @interface ResetConstructors
 */
export interface ResetConstructors {
    /**
     * Execute a classic set of commands that will reset the chip.
     * @param transport Transport class to perform serial communication.
     * @param resetDelay Delay in milliseconds for reset.
     */
    classicReset?: (transport: Transport, resetDelay: number) => ClassicReset;
    /**
     * Execute a set of commands for USB JTAG serial reset.
     * @param transport Transport class to perform serial communication.
     */
    usbJTAGSerialReset?: (transport: Transport) => UsbJtagSerialReset;
    /**
     * Execute a classic set of commands that will reset the chip.
     * @param transport Transport class to perform serial communication.
     * @param {boolean} usingUsbOtg is it using USB-OTG ?
     */
    hardReset?: (transport: Transport, usingUsbOtg?: boolean) => HardReset;
    /**
     * Execute a custom set of commands that will reset the chip.
     * @param transport Transport class to perform serial communication.
     * @param {string} sequenceString Custom string sequence for reset strategy
     */
    customReset?: (transport: Transport, sequenceString: string) => CustomReset;
}
/**
 * Reset strategy class
 */
export interface ResetStrategy {
    transport: Transport;
    reset(): Promise<void>;
}
/**
 * Execute a classic set of commands that will reset the chip.
 *
 * Commands (e.g. R0) are defined by a code (R) and an argument (0).
 *
 * The commands are:
 *
 * D: setDTR - 1=True / 0=False
 *
 * R: setRTS - 1=True / 0=False
 *
 * W: Wait (time delay) - positive integer number (miliseconds)
 *
 * "D0|R1|W100|D1|R0|W50|D0" represents the classic reset strategy
 * @param {Transport} transport Transport class to perform serial communication.
 * @param {number} resetDelay Delay in milliseconds for reset.
 */
export declare class ClassicReset implements ResetStrategy {
    resetDelay: number;
    transport: Transport;
    constructor(transport: Transport, resetDelay: number);
    reset(): Promise<void>;
}
/**
 * Execute a set of commands for USB JTAG serial reset.
 *
 * Commands (e.g. R0) are defined by a code (R) and an argument (0).
 *
 * The commands are:
 *
 * D: setDTR - 1=True / 0=False
 *
 * R: setRTS - 1=True / 0=False
 *
 * W: Wait (time delay) - positive integer number (miliseconds)
 * @param {Transport} transport Transport class to perform serial communication.
 */
export declare class UsbJtagSerialReset implements ResetStrategy {
    transport: Transport;
    constructor(transport: Transport);
    reset(): Promise<void>;
}
/**
 * Execute a set of commands that will hard reset the chip.
 *
 * Commands (e.g. R0) are defined by a code (R) and an argument (0).
 *
 * The commands are:
 *
 * D: setDTR - 1=True / 0=False
 *
 * R: setRTS - 1=True / 0=False
 *
 * W: Wait (time delay) - positive integer number (miliseconds)
 * @param {Transport} transport Transport class to perform serial communication.
 * @param {boolean} usingUsbOtg is it using USB-OTG ?
 */
export declare class HardReset implements ResetStrategy {
    transport: Transport;
    private usingUsbOtg;
    constructor(transport: Transport, usingUsbOtg?: boolean);
    reset(): Promise<void>;
}
/**
 * Validate a sequence string based on the following format:
 *
 * Commands (e.g. R0) are defined by a code (R) and an argument (0).
 *
 * The commands are:
 *
 * D: setDTR - 1=True / 0=False
 *
 * R: setRTS - 1=True / 0=False
 *
 * W: Wait (time delay) - positive integer number (miliseconds)
 * @param {string} seqStr Sequence string to validate
 * @returns {boolean} Is the sequence string valid ?
 */
export declare function validateCustomResetStringSequence(seqStr: string): boolean;
/**
 * Custom reset strategy defined with a string.
 *
 * The sequenceString input string consists of individual commands divided by "|".
 *
 * Commands (e.g. R0) are defined by a code (R) and an argument (0).
 *
 * The commands are:
 *
 * D: setDTR - 1=True / 0=False
 *
 * R: setRTS - 1=True / 0=False
 *
 * W: Wait (time delay) - positive integer number (miliseconds)
 *
 * "D0|R1|W100|D1|R0|W50|D0" represents the classic reset strategy
 * @param {Transport} transport Transport class to perform serial communication.
 * @param {string} sequenceString Custom string sequence for reset strategy
 */
export declare class CustomReset implements ResetStrategy {
    transport: Transport;
    private sequenceString;
    constructor(transport: Transport, sequenceString: string);
    reset(): Promise<void>;
}
declare const _default: {
    ClassicReset: typeof ClassicReset;
    CustomReset: typeof CustomReset;
    HardReset: typeof HardReset;
    UsbJtagSerialReset: typeof UsbJtagSerialReset;
    validateCustomResetStringSequence: typeof validateCustomResetStringSequence;
};
export default _default;
