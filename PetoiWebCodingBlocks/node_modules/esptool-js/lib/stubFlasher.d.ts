export interface Stub {
    bss_start?: number;
    data: string;
    decodedData: Uint8Array;
    data_start: number;
    entry: number;
    text: string;
    decodedText: Uint8Array;
    text_start: number;
}
/**
 * Import flash stub json for the given chip name.
 * @param {string} chipName Name of chip to obtain flasher stub
 * @returns {Stub} Stub information and decoded text and data
 */
export declare function getStubJsonByChipName(chipName: string): Promise<Stub | undefined>;
/**
 * Convert a base 64 string to Uint8Array.
 * @param {string} dataStr Base64 String to decode
 * @returns {Uint8Array} Decoded Uint8Array
 */
export declare function decodeBase64Data(dataStr: string): Uint8Array;
