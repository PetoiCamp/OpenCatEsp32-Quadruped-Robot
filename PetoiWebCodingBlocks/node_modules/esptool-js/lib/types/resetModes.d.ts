/**
 * Reset modes that can be used before connection to chip
 */
export declare type Before = "default_reset" | "usb_reset" | "no_reset" | "no_reset_no_sync";
/**
 * Reset modes that can be used after operation is finished.
 */
export declare type After = "hard_reset" | "soft_reset" | "no_reset" | "no_reset_stub";
