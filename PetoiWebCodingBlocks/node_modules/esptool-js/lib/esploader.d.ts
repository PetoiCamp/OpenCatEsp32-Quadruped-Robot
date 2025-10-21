import { Transport } from "./webserial.js";
import { ROM } from "./targets/rom.js";
import { ResetStrategy } from "./reset.js";
import { LoaderOptions } from "./types/loaderOptions.js";
import { FlashOptions } from "./types/flashOptions.js";
import { After, Before } from "./types/resetModes.js";
declare type FlashReadCallback = ((packet: Uint8Array, progress: number, totalSize: number) => void) | null;
export declare class ESPLoader {
    ESP_RAM_BLOCK: number;
    ESP_FLASH_BEGIN: number;
    ESP_FLASH_DATA: number;
    ESP_FLASH_END: number;
    ESP_MEM_BEGIN: number;
    ESP_MEM_END: number;
    ESP_MEM_DATA: number;
    ESP_WRITE_REG: number;
    ESP_READ_REG: number;
    ESP_SPI_ATTACH: number;
    ESP_CHANGE_BAUDRATE: number;
    ESP_FLASH_DEFL_BEGIN: number;
    ESP_FLASH_DEFL_DATA: number;
    ESP_FLASH_DEFL_END: number;
    ESP_SPI_FLASH_MD5: number;
    ESP_ERASE_FLASH: number;
    ESP_ERASE_REGION: number;
    ESP_READ_FLASH: number;
    ESP_RUN_USER_CODE: number;
    ESP_IMAGE_MAGIC: number;
    ESP_CHECKSUM_MAGIC: number;
    ROM_INVALID_RECV_MSG: number;
    DEFAULT_TIMEOUT: number;
    ERASE_REGION_TIMEOUT_PER_MB: number;
    ERASE_WRITE_TIMEOUT_PER_MB: number;
    MD5_TIMEOUT_PER_MB: number;
    CHIP_ERASE_TIMEOUT: number;
    FLASH_READ_TIMEOUT: number;
    MAX_TIMEOUT: number;
    CHIP_DETECT_MAGIC_REG_ADDR: number;
    DETECTED_FLASH_SIZES: {
        [key: number]: string;
    };
    DETECTED_FLASH_SIZES_NUM: {
        [key: number]: number;
    };
    USB_JTAG_SERIAL_PID: number;
    chip: ROM;
    IS_STUB: boolean;
    FLASH_WRITE_SIZE: number;
    transport: Transport;
    private baudrate;
    private serialOptions?;
    private terminal?;
    private romBaudrate;
    private debugLogging;
    private syncStubDetected;
    private resetConstructors;
    /**
     * Create a new ESPLoader to perform serial communication
     * such as read/write flash memory and registers using a LoaderOptions object.
     * @param {LoaderOptions} options - LoaderOptions object argument for ESPLoader.
     * ```
     * const myLoader = new ESPLoader({ transport: Transport, baudrate: number, terminal?: IEspLoaderTerminal });
     * ```
     */
    constructor(options: LoaderOptions);
    _sleep(ms: number): Promise<unknown>;
    /**
     * Write to ESP Loader constructor's terminal with or without new line.
     * @param {string} str - String to write.
     * @param {boolean} withNewline - Add new line at the end ?
     */
    write(str: string, withNewline?: boolean): void;
    /**
     * Write error message to ESP Loader constructor's terminal with or without new line.
     * @param {string} str - String to write.
     * @param {boolean} withNewline - Add new line at the end ?
     */
    error(str: string, withNewline?: boolean): void;
    /**
     * Write information message to ESP Loader constructor's terminal with or without new line.
     * @param {string} str - String to write.
     * @param {boolean} withNewline - Add new line at the end ?
     */
    info(str: string, withNewline?: boolean): void;
    /**
     * Write debug message to ESP Loader constructor's terminal with or without new line.
     * @param {string} str - String to write.
     * @param {boolean} withNewline - Add new line at the end ?
     */
    debug(str: string, withNewline?: boolean): void;
    /**
     * Convert short integer to byte array
     * @param {number} i - Number to convert.
     * @returns {Uint8Array} Byte array.
     */
    _shortToBytearray(i: number): Uint8Array;
    /**
     * Convert an integer to byte array
     * @param {number} i - Number to convert.
     * @returns {ROM} The chip ROM class related to given magic hex number.
     */
    _intToByteArray(i: number): Uint8Array;
    /**
     * Convert a byte array to short integer.
     * @param {number} i - Number to convert.
     * @param {number} j - Number to convert.
     * @returns {number} Return a short integer number.
     */
    _byteArrayToShort(i: number, j: number): number;
    /**
     * Convert a byte array to integer.
     * @param {number} i - Number to convert.
     * @param {number} j - Number to convert.
     * @param {number} k - Number to convert.
     * @param {number} l - Number to convert.
     * @returns {number} Return a integer number.
     */
    _byteArrayToInt(i: number, j: number, k: number, l: number): number;
    /**
     * Append a buffer array after another buffer array
     * @param {ArrayBuffer} buffer1 - First array buffer.
     * @param {ArrayBuffer} buffer2 - magic hex number to select ROM.
     * @returns {ArrayBufferLike} Return an array buffer.
     */
    _appendBuffer(buffer1: ArrayBuffer, buffer2: ArrayBuffer): ArrayBufferLike;
    /**
     * Append a buffer array after another buffer array
     * @param {Uint8Array} arr1 - First array buffer.
     * @param {Uint8Array} arr2 - magic hex number to select ROM.
     * @returns {Uint8Array} Return a 8 bit unsigned array.
     */
    _appendArray(arr1: Uint8Array, arr2: Uint8Array): Uint8Array;
    /**
     * Convert a unsigned 8 bit integer array to byte string.
     * @param {Uint8Array} u8Array - magic hex number to select ROM.
     * @returns {string} Return the equivalent string.
     */
    ui8ToBstr(u8Array: Uint8Array): string;
    /**
     * Convert a byte string to unsigned 8 bit integer array.
     * @param {string} bStr - binary string input
     * @returns {Uint8Array} Return a 8 bit unsigned integer array.
     */
    bstrToUi8(bStr: string): Uint8Array;
    /**
     * Flush the serial input by raw read with 200 ms timeout.
     */
    flushInput(): Promise<void>;
    /**
     * Use the device serial port read function with given timeout to create a valid packet.
     * @param {number} op Operation number
     * @param {number} timeout timeout number in milliseconds
     * @returns {[number, Uint8Array]} valid response packet.
     */
    readPacket(op?: number | null, timeout?: number): Promise<[number, Uint8Array]>;
    /**
     * Write a serial command to the chip
     * @param {number} op - Operation number
     * @param {Uint8Array} data - Unsigned 8 bit array
     * @param {number} chk - channel number
     * @param {boolean} waitResponse - wait for response ?
     * @param {number} timeout - timeout number in milliseconds
     * @returns {Promise<[number, Uint8Array]>} Return a number and a 8 bit unsigned integer array.
     */
    command(op?: number | null, data?: Uint8Array, chk?: number, waitResponse?: boolean, timeout?: number): Promise<[number, Uint8Array]>;
    /**
     * Read a register from chip.
     * @param {number} addr - Register address number
     * @param {number} timeout - Timeout in milliseconds (Default: 3000ms)
     * @returns {number} - Command number value
     */
    readReg(addr: number, timeout?: number): Promise<number>;
    /**
     * Write a number value to register address in chip.
     * @param {number} addr - Register address number
     * @param {number} value - Number value to write in register
     * @param {number} mask - Hex number for mask
     * @param {number} delayUs Delay number
     * @param {number} delayAfterUs Delay after previous delay
     */
    writeReg(addr: number, value: number, mask?: number, delayUs?: number, delayAfterUs?: number): Promise<void>;
    /**
     * Sync chip by sending sync command.
     * @returns {[number, Uint8Array]} Command result
     */
    sync(): Promise<[number, Uint8Array]>;
    /**
     * Attempt to connect to the chip by sending a reset sequence and later a sync command.
     * @param {string} mode - Reset mode to use
     * @param {ResetStrategy} resetStrategy - Reset strategy class to use for connect
     * @returns {string} - Returns 'success' or 'error' message.
     */
    _connectAttempt(mode: string | undefined, resetStrategy: ResetStrategy | null): Promise<string>;
    /**
     * Constructs a sequence of reset strategies based on the OS,
     * used ESP chip, external settings, and environment variables.
     * Returns a tuple of one or more reset strategies to be tried sequentially.
     * @param {string} mode - Reset mode to use
     * @returns {ResetStrategy[]} - Array of reset strategies
     */
    constructResetSequence(mode: Before): ResetStrategy[];
    /**
     * Perform a connection to chip.
     * @param {string} mode - Reset mode to use. Example: 'default_reset' | 'no_reset'
     * @param {number} attempts - Number of connection attempts
     * @param {boolean} detecting - Detect the connected chip
     */
    connect(mode?: Before, attempts?: number, detecting?: boolean): Promise<void>;
    /**
     * Connect and detect the existing chip.
     * @param {string} mode Reset mode to use for connection.
     */
    detectChip(mode?: Before): Promise<void>;
    /**
     * Execute the command and check the command response.
     * @param {string} opDescription Command operation description.
     * @param {number} op Command operation number
     * @param {Uint8Array} data Command value
     * @param {number} chk Checksum to use
     * @param {number} timeout TImeout number in milliseconds (ms)
     * @returns {number} Command result
     */
    checkCommand(opDescription?: string, op?: number | null, data?: Uint8Array, chk?: number, timeout?: number): Promise<number | Uint8Array>;
    /**
     * Start downloading an application image to RAM
     * @param {number} size Image size number
     * @param {number} blocks Number of data blocks
     * @param {number} blocksize Size of each data block
     * @param {number} offset Image offset number
     */
    memBegin(size: number, blocks: number, blocksize: number, offset: number): Promise<void>;
    /**
     * Get the checksum for given unsigned 8-bit array
     * @param {Uint8Array} data Unsigned 8-bit integer array
     * @param {number} state Initial checksum
     * @returns {number} - Array checksum
     */
    checksum(data: Uint8Array, state?: number): number;
    /**
     * Send a block of image to RAM
     * @param {Uint8Array} buffer Unsigned 8-bit array
     * @param {number} seq Sequence number
     */
    memBlock(buffer: Uint8Array, seq: number): Promise<void>;
    /**
     * Leave RAM download mode and run application
     * @param {number} entrypoint - Entrypoint number
     */
    memFinish(entrypoint: number): Promise<void>;
    /**
     * Configure SPI flash pins
     * @param {number} hspiArg -  Argument for SPI attachment
     */
    flashSpiAttach(hspiArg: number): Promise<void>;
    /**
     * Scale timeouts which are size-specific.
     * @param {number} secondsPerMb Seconds per megabytes as number
     * @param {number} sizeBytes Size bytes number
     * @returns {number} - Scaled timeout for specified size.
     */
    timeoutPerMb(secondsPerMb: number, sizeBytes: number): number;
    /**
     * Start downloading to Flash (performs an erase)
     * @param {number} size Size to erase
     * @param {number} offset Offset to erase
     * @returns {number} Number of blocks (of size self.FLASH_WRITE_SIZE) to write.
     */
    flashBegin(size: number, offset: number): Promise<number>;
    /**
     * Start downloading compressed data to Flash (performs an erase)
     * @param {number} size Write size
     * @param {number} compsize Compressed size
     * @param {number} offset Offset for write
     * @returns {number} Returns number of blocks (size self.FLASH_WRITE_SIZE) to write.
     */
    flashDeflBegin(size: number, compsize: number, offset: number): Promise<number>;
    /**
     * Write block to flash, retry if fail
     * @param {Uint8Array} data Unsigned 8-bit array data.
     * @param {number} seq Sequence number
     * @param {number} timeout Timeout in milliseconds (ms)
     */
    flashBlock(data: Uint8Array, seq: number, timeout: number): Promise<void>;
    /**
     * Write block to flash, send compressed, retry if fail
     * @param {Uint8Array} data Unsigned int 8-bit array data to write
     * @param {number} seq Sequence number
     * @param {number} timeout Timeout in milliseconds (ms)
     */
    flashDeflBlock(data: Uint8Array, seq: number, timeout: number): Promise<void>;
    /**
     * Leave flash mode and run/reboot
     * @param {boolean} reboot Reboot after leaving flash mode ?
     */
    flashFinish(reboot?: boolean): Promise<void>;
    /**
     * Leave compressed flash mode and run/reboot
     * @param {boolean} reboot Reboot after leaving flash mode ?
     */
    flashDeflFinish(reboot?: boolean): Promise<void>;
    /**
     * Run an arbitrary SPI flash command.
     *
     * This function uses the "USR_COMMAND" functionality in the ESP
     * SPI hardware, rather than the precanned commands supported by
     * hardware. So the value of spiflashCommand is an actual command
     * byte, sent over the wire.
     *
     * After writing command byte, writes 'data' to MOSI and then
     * reads back 'readBits' of reply on MISO. Result is a number.
     * @param {number} spiflashCommand Command to execute in SPI
     * @param {Uint8Array} data Data to send
     * @param {number} readBits Number of bits to read
     * @returns {number} Register SPI_W0_REG value
     */
    runSpiflashCommand(spiflashCommand: number, data: Uint8Array, readBits: number): Promise<number>;
    /**
     * Read flash id by executing the SPIFLASH_RDID flash command.
     * @returns {Promise<number>} Register SPI_W0_REG value
     */
    readFlashId(): Promise<number>;
    /**
     * Execute the erase flash command
     * @returns {Promise<number | Uint8Array>} Erase flash command result
     */
    eraseFlash(): Promise<number | Uint8Array>;
    /**
     * Convert a number or unsigned 8-bit array to hex string
     * @param {number | Uint8Array } buffer Data to convert to hex string.
     * @returns {string} A hex string
     */
    toHex(buffer: number | Uint8Array): string;
    /**
     * Calculate the MD5 Checksum command
     * @param {number} addr Address number
     * @param {number} size Package size
     * @returns {string} MD5 Checksum string
     */
    flashMd5sum(addr: number, size: number): Promise<string>;
    readFlash(addr: number, size: number, onPacketReceived?: FlashReadCallback): Promise<Uint8Array>;
    /**
     * Upload the flasher ROM bootloader (flasher stub) to the chip.
     * @returns {ROM} The Chip ROM
     */
    runStub(): Promise<ROM>;
    /**
     * Change the chip baudrate.
     */
    changeBaud(): Promise<void>;
    /**
     * Execute the main function of ESPLoader.
     * @param {string} mode Reset mode to use
     * @returns {string} chip ROM
     */
    main(mode?: Before): Promise<string>;
    /**
     * Get flash size bytes from flash size string.
     * @param {string} flashSize Flash Size string
     * @returns {number} Flash size bytes
     */
    flashSizeBytes: (flashSize: string) => number;
    /**
     * Parse a given flash size string to a number
     * @param {string} flsz Flash size to request
     * @returns {number} Flash size number
     */
    parseFlashSizeArg(flsz: string): number;
    /**
     * Update the image flash parameters with given arguments.
     * @param {string} image binary image as string
     * @param {number} address flash address number
     * @param {string} flashSize Flash size string
     * @param {string} flashMode Flash mode string
     * @param {string} flashFreq Flash frequency string
     * @returns {string} modified image string
     */
    _updateImageFlashParams(image: string, address: number, flashSize: string, flashMode: string, flashFreq: string): string;
    /**
     * Write set of file images into given address based on given FlashOptions object.
     * @param {FlashOptions} options FlashOptions to configure how and what to write into flash.
     */
    writeFlash(options: FlashOptions): Promise<void>;
    /**
     * Read SPI flash manufacturer and device id.
     */
    flashId(): Promise<void>;
    getFlashSize(): Promise<number>;
    /**
     * Soft reset the device chip. Soft reset with run user code is the closest.
     * @param {boolean} stayInBootloader Flag to indicate if to stay in bootloader
     */
    softReset(stayInBootloader: boolean): Promise<void>;
    /**
     * Execute this function to execute after operation reset functions.
     * @param {After} mode After operation mode. Default is 'hard_reset'.
     * @param { boolean } usingUsbOtg For 'hard_reset' to specify if using USB-OTG
     */
    after(mode?: After, usingUsbOtg?: boolean): Promise<void>;
}
export {};
