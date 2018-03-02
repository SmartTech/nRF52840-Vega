#ifndef APP_USBD_STRING_CONFIG_H
#define APP_USBD_STRING_CONFIG_H


#define APP_USBD_STRINGS_LANGIDS \
    ((uint16_t)APP_USBD_LANG_ENGLISH | (uint16_t)APP_USBD_SUBLANG_ENGLISH_US)

/**
 * @brief Manufacturer name string descriptor
 *
 * Comma separated list of manufacturer names for each defined language.
 * Use @ref APP_USBD_STRING_DESC macro to create string descriptor.
 *
 * The order of manufacturer names has to be the same like in
 * @ref APP_USBD_STRINGS_LANGIDS.
 */
#define APP_USBD_STRINGS_MANUFACTURER    \
    APP_USBD_STRING_DESC('V','e','g','a','-','A','b','s','o','l','u','t','e','.',' ','2','0','1','8')

/**
 * @brief Define whether @ref APP_USBD_STRINGS_MANUFACTURER is created by @ref APP_USBD_STRING_DESC
 * or declared as global variable.
 * */
#define APP_USBD_STRINGS_MANUFACTURER_EXTERN 0

/**
 * @brief Product name string descriptor
 *
 * List of product names defined the same way like in @ref APP_USBD_STRINGS_MANUFACTURER
 */
#define APP_USBD_STRINGS_PRODUCT         \
    APP_USBD_STRING_DESC('n', 'R', 'F', '5', 'x', ' ', 'U', 'S', 'B', ' ', 'V', 'e', 'g', 'a')


/**
 * @brief Define whether @ref APP_USBD_STRINGS_PRODUCT is created by @ref APP_USBD_STRING_DESC
 * or declared as global variable.
 * */
#define APP_USBD_STRINGS_PRODUCT_EXTERN 0

/**
 * @brief Serial number string descriptor
 *
 * Create serial number string descriptor using @ref APP_USBD_STRING_DESC,
 * or configure it to point to any internal variable pointer filled with descriptor.
 *
 * @note
 * There is only one SERIAL number inside the library and it is Language independent.
 */
#define APP_USBD_STRING_SERIAL          \
    APP_USBD_STRING_DESC('0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0')

/**
 * @brief Define whether @ref APP_USBD_STRING_SERIAL is created by @ref APP_USBD_STRING_DESC
 * or declared as global variable.
 * */
#define APP_USBD_STRING_SERIAL_EXTERN 0

/**
 * @brief User strings default values
 *
 * This value stores all application specific user strings with its default initialization.
 * The setup is done by X-macros.
 * Expected macro parameters:
 * @code
 * X(mnemonic, [=str_idx], ...)
 * @endcode
 * - @c mnemonic: Mnemonic of the string descriptor that would be added to
 *                @ref app_usbd_string_desc_idx_t enumerator.
 * - @c str_idx : String index value, may be set or left empty.
 *                For example WinUSB driver requires descriptor to be present on 0xEE index.
 *                Then use X(USBD_STRING_WINUSB, =0xEE, (APP_USBD_STRING_DESC(...)))
 * - @c ...     : List of string descriptors for each defined language.
 */
#define APP_USBD_STRINGS_USER          \
    X(APP_USER_1, , APP_USBD_STRING_DESC('U', 's', 'e', 'r', ' ', '1'))

/** @} */
#endif /* APP_USBD_STRING_CONFIG_H */
