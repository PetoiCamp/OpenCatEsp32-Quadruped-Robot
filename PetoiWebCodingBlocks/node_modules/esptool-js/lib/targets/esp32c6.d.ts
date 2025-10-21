import { ESPLoader } from "../esploader.js";
import { ROM } from "./rom.js";
export declare class ESP32C6ROM extends ROM {
    CHIP_NAME: string;
    IMAGE_CHIP_ID: number;
    EFUSE_BASE: number;
    MAC_EFUSE_REG: number;
    UART_CLKDIV_REG: number;
    UART_CLKDIV_MASK: number;
    UART_DATE_REG_ADDR: number;
    FLASH_WRITE_SIZE: number;
    BOOTLOADER_FLASH_OFFSET: number;
    FLASH_SIZES: {
        "1MB": number;
        "2MB": number;
        "4MB": number;
        "8MB": number;
        "16MB": number;
    };
    SPI_REG_BASE: number;
    SPI_USR_OFFS: number;
    SPI_USR1_OFFS: number;
    SPI_USR2_OFFS: number;
    SPI_MOSI_DLEN_OFFS: number;
    SPI_MISO_DLEN_OFFS: number;
    SPI_W0_OFFS: number;
    getPkgVersion(loader: ESPLoader): Promise<number>;
    getChipRevision(loader: ESPLoader): Promise<number>;
    getChipDescription(loader: ESPLoader): Promise<string>;
    getChipFeatures(loader: ESPLoader): Promise<string[]>;
    getCrystalFreq(loader: ESPLoader): Promise<number>;
    _d2h(d: number): string;
    readMac(loader: ESPLoader): Promise<string>;
    getEraseSize(offset: number, size: number): number;
}
