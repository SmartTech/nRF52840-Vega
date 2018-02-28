#ifndef APP_USBD_STRING_CONFIG_H
#define APP_USBD_STRING_CONFIG_H


//-----------------------------------------------------------------------------
#define APP_USBD_STRINGS_LANGIDS \
    ((uint16_t)APP_USBD_LANG_ENGLISH | (uint16_t)APP_USBD_SUBLANG_ENGLISH_US)


//-----------------------------------------------------------------------------
#define APP_USBD_STRINGS_MANUFACTURER    \
    APP_USBD_STRING_DESC('N', 'o', 'r', 'd', 'i', 'c', ' ', 'S', 'e', 'm', 'i', 'c', 'o', 'n', 'd', 'u', 'c', 't', 'o', 'r')

#define APP_USBD_STRINGS_MANUFACTURER_EXTERN 0


//-----------------------------------------------------------------------------
#define APP_USBD_STRINGS_PRODUCT         \
    APP_USBD_STRING_DESC('n', 'R', 'F', '5', '2', ' ', 'U', 'S', 'B', ' ', 'D', 'e', 'm', 'o')

#define APP_USBD_STRINGS_PRODUCT_EXTERN 0

      
//-----------------------------------------------------------------------------
#define APP_USBD_STRING_SERIAL          \
    APP_USBD_STRING_DESC('0', 'x', '1', 'A', '2', 'B', '3', 'C', '4', 'D', '5', 'E')

#define APP_USBD_STRING_SERIAL_EXTERN 0

      
//-----------------------------------------------------------------------------
#define APP_USBD_STRINGS_USER          \
    X(APP_USER_1, , APP_USBD_STRING_DESC('U', 's', 'e', 'r', ' ', '1'))


#endif /* APP_USBD_STRING_CONFIG_H */
