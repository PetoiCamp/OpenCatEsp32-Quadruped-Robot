import { ESPLoader } from "../esploader.js";
import { ROM } from "./rom.js";
export declare class ESP32ROM extends ROM {
    CHIP_NAME: string;
    IMAGE_CHIP_ID: number;
    EFUSE_RD_REG_BASE: number;
    DR_REG_SYSCON_BASE: number;
    UART_CLKDIV_REG: number;
    UART_CLKDIV_MASK: number;
    UART_DATE_REG_ADDR: number;
    XTAL_CLK_DIVIDER: number;
    FLASH_SIZES: {
        [key: string]: number;
    };
    FLASH_WRITE_SIZE: number;
    BOOTLOADER_FLASH_OFFSET: number;
    SPI_REG_BASE: number;
    SPI_USR_OFFS: number;
    SPI_USR1_OFFS: number;
    SPI_USR2_OFFS: number;
    SPI_W0_OFFS: number;
    SPI_MOSI_DLEN_OFFS: number;
    SPI_MISO_DLEN_OFFS: number;
    readEfuse(loader: ESPLoader, offset: number): Promise<number>;
    getPkgVersion(loader: ESPLoader): Promise<number>;
    getChipRevision(loader: ESPLoader): Promise<number>;
    getChipDescription(loader: ESPLoader): Promise<string>;
    getChipFeatures(loader: ESPLoader): Promise<string[]>;
    getCrystalFreq(loader: ESPLoader): Promise<number>;
    _d2h(d: number): string;
    readMac(loader: ESPLoader): Promise<string>;
}
