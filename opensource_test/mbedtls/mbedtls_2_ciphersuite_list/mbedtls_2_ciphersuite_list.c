#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "mbedtls/ssl.h"
#include "mbedtls/platform.h"
int main(void)
{

    const int *list;
    const char *name;
    int index = 1;

    mbedtls_printf("\n  Available Ciphersuite:\n\n");
    list = mbedtls_ssl_list_ciphersuites();
    for (; *list; list++)
    {
        name = mbedtls_ssl_get_ciphersuite_name(*list);
        mbedtls_printf("  [%03d] %s\n", index++, name);
    }
    mbedtls_printf("\n");
    return 0;
}
