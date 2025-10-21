/// <reference types="w3c-web-serial" />
/**
 * Options for device serialPort.
 * @interface SerialOptions
 *
 * Note: According to the documentation of the Web Serial API, 'baudRate' is a
 * 'required' field as part of serial options. However, we are currently
 * maintaining 'baudRate' as a separate parameter outside the options
 * dictionary, and it is effectively used in the code. For now, we are
 * keeping it optional in the dictionary to avoid conflicts.
 */
export interface SerialOptions {
    /**
     * A positive, non-zero value indicating the baud rate at which serial communication should be established.
     * @type {number | undefined}
     */
    baudRate?: number | undefined;
    /**
     * The number of data bits per frame. Either 7 or 8.
     * @type {number | undefined}
     */
    dataBits?: number | undefined;
    /**
     * The number of stop bits at the end of a frame. Either 1 or 2.
     * @type {number | undefined}
     */
    stopBits?: number | undefined;
    /**
     * The parity mode: none, even or odd
     * @type {ParityType | undefined}
     */
    parity?: ParityType | undefined;
    /**
     * A positive, non-zero value indicating the size of the read and write buffers that should be created.
     * @type {number | undefined}
     */
    bufferSize?: number | undefined;
    /**
     * The flow control mode: none or hardware.
     * @type {FlowControlType | undefined}
     */
    flowControl?: FlowControlType | undefined;
}
/**
 * Wrapper class around Webserial API to communicate with the serial device.
 * @param {typeof import("w3c-web-serial").SerialPort} device - Requested device prompted by the browser.
 *
 * ```
 * const port = await navigator.serial.requestPort();
 * ```
 */
declare class Transport {
    device: SerialPort;
    tracing: boolean;
    slipReaderEnabled: boolean;
    baudrate: number;
    private traceLog;
    private lastTraceTime;
    private reader;
    private buffer;
    constructor(device: SerialPort, tracing?: boolean, enableSlipReader?: boolean);
    /**
     * Request the serial device vendor ID and Product ID as string.
     * @returns {string} Return the device VendorID and ProductID from SerialPortInfo as formatted string.
     */
    getInfo(): string;
    /**
     * Request the serial device product id from SerialPortInfo.
     * @returns {number | undefined} Return the product ID.
     */
    getPid(): number | undefined;
    /**
     * Format received or sent data for tracing output.
     * @param {string} message Message to format as trace line.
     */
    trace(message: string): void;
    returnTrace(): Promise<void>;
    hexify(s: Uint8Array): string;
    hexConvert(uint8Array: Uint8Array, autoSplit?: boolean): string;
    /**
     * Format data packet using the Serial Line Internet Protocol (SLIP).
     * @param {Uint8Array} data Binary unsigned 8 bit array data to format.
     * @returns {Uint8Array} Formatted unsigned 8 bit data array.
     */
    slipWriter(data: Uint8Array): Uint8Array;
    /**
     * Write binary data to device using the WebSerial device writable stream.
     * @param {Uint8Array} data 8 bit unsigned data array to write to device.
     */
    write(data: Uint8Array): Promise<void>;
    /**
     * Append a buffer array after another buffer array
     * @param {Uint8Array} arr1 - First array buffer.
     * @param {Uint8Array} arr2 - magic hex number to select ROM.
     * @returns {Uint8Array} Return a 8 bit unsigned array.
     */
    appendArray(arr1: Uint8Array, arr2: Uint8Array): Uint8Array;
    private readLoop;
    newRead(numBytes: number, timeout: number): Promise<Uint8Array>;
    flushInput(): Promise<void>;
    flushOutput(): Promise<void>;
    inWaiting(): number;
    /**
     * Detect if the data read from device is a Fatal or Guru meditation error.
     * @param {Uint8Array} input Data read from device
     */
    private detectPanicHandler;
    private SLIP_END;
    private SLIP_ESC;
    private SLIP_ESC_END;
    private SLIP_ESC_ESC;
    /**
     * Take a data array and return the first well formed packet after
     * replacing the escape sequence. Reads at least 8 bytes.
     * @param {number} timeout Timeout read data.
     * @yields {Uint8Array} Formatted packet using SLIP escape sequences.
     */
    read(timeout: number): AsyncGenerator<Uint8Array>;
    /**
     * Read from serial device without slip formatting.
     * @yields {Uint8Array} The next number in the Fibonacci sequence.
     */
    rawRead(): AsyncGenerator<Uint8Array>;
    _DTR_state: boolean;
    /**
     * Send the RequestToSend (RTS) signal to given state
     * # True for EN=LOW, chip in reset and False EN=HIGH, chip out of reset
     * @param {boolean} state Boolean state to set the signal
     */
    setRTS(state: boolean): Promise<void>;
    /**
     * Send the dataTerminalReady (DTS) signal to given state
     * # True for IO0=LOW, chip in reset and False IO0=HIGH
     * @param {boolean} state Boolean state to set the signal
     */
    setDTR(state: boolean): Promise<void>;
    /**
     * Connect to serial device using the Webserial open method.
     * @param {number} baud Number baud rate for serial connection. Default is 115200.
     * @param {typeof import("w3c-web-serial").SerialOptions} serialOptions Serial Options for WebUSB SerialPort class.
     */
    connect(baud?: number, serialOptions?: SerialOptions): Promise<void>;
    sleep(ms: number): Promise<unknown>;
    /**
     * Wait for a given timeout ms for serial device unlock.
     * @param {number} timeout Timeout time in milliseconds (ms) to sleep
     */
    waitForUnlock(timeout: number): Promise<void>;
    /**
     * Disconnect from serial device by running SerialPort.close() after streams unlock.
     */
    disconnect(): Promise<void>;
}
export { Transport };
