/**
 * Represents a Espressif chip error.
 */
declare class ESPError extends Error {
}
/**
 * Represents a Espressif timeout chip error.
 */
declare class TimeoutError extends ESPError {
}
export { ESPError, TimeoutError };
