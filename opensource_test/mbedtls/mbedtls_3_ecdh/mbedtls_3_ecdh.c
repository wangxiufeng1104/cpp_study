#include <stdio.h>
#include "string.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/ecdh.h"
#include "mbedtls/platform.h"

uint8_t buf[65];

static void dump_buf(uint8_t *buf, uint32_t len)
{
    int i;
    for (i = 0; i < len; i++)
    {
        printf("%s%02X%s", i % 16 == 0 ? "\r\n\t" : " ",
               buf[i],
               i == len - 1 ? "\r\n" : "");
    }
}

int mbedtls_ecdh_test(void)
{
    int ret;
    size_t olen;

    const char *pers = "ecdh_test";
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ecp_point client_pub, server_pub;
    mbedtls_ecp_group grp;
    mbedtls_mpi client_secret, server_secret, test_secret;
    mbedtls_mpi client_pri, server_pri;

    /* 1. init structure */
    mbedtls_entropy_init(&entropy);   // 初始化椭圆曲线熵结构体
    mbedtls_ctr_drbg_init(&ctr_drbg); // 初始化随机数结构体
    mbedtls_mpi_init(&client_secret);
    mbedtls_mpi_init(&server_secret);
    mbedtls_mpi_init(&client_pri);
    mbedtls_mpi_init(&server_pri);
    mbedtls_ecp_group_init(&grp);        // 初始化椭圆曲线群结构体
    mbedtls_ecp_point_init(&client_pub); // 初始化椭圆曲线点结构体
    mbedtls_ecp_point_init(&server_pub); // 初始化椭圆曲线点结构体

    /* 2. update seed with we own interface ported */
    printf("\n  . Seeding the random number generator...");

    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                (const unsigned char *)pers,
                                strlen(pers));
    if (ret != 0)
    {
        printf(" failed\n  ! mbedtls_ctr_drbg_seed returned %d(-0x%04x)\n", ret, -ret);
        goto exit;
    }
    printf(" ok\n");

    /* 3.  select ecp group SECP256R1 */
    printf("\n  . Select ecp group SECP256R1...");
    // 加载椭圆曲线，选择SECP256R1
    //ret = mbedtls_ecp_group_load(&grp, MBEDTLS_ECP_DP_SECP256R1);
    ret = mbedtls_ecp_group_load(&grp, MBEDTLS_ECP_DP_CURVE25519);
    if (ret != 0)
    {
        printf(" failed\n  ! mbedtls_ecp_group_load returned %d(-0x%04x)\n", ret, -ret);
        goto exit;
    }

    printf("ok\r\n");

    /* 4. Client generate public parameter */
    printf("\n  . Client Generate public parameter...");
    // cli生成公开参数
    ret = mbedtls_ecdh_gen_public(&grp,        // 椭圆曲线结构体
                                  &client_pri, // 输出cli私密参数d
                                  &client_pub, // 输出cli公开参数Q
                                  mbedtls_ctr_drbg_random,
                                  &ctr_drbg);
    if (ret != 0)
    {
        printf(" failed\n  ! mbedtls_ecdh_gen_public returned %d(-0x%04x)\n", ret, -ret);
        goto exit;
    }
    printf(" ok\n");

    /* show public parameter */
    // 把cli的公开参数到处到buf中
    mbedtls_ecp_point_write_binary(&grp, &client_pub, MBEDTLS_ECP_PF_UNCOMPRESSED, &olen, buf, sizeof(buf));
    dump_buf(buf, olen);

    /* 5. Client generate public parameter */
    printf("\n  . Server Generate public parameter...");
    // srv生成公开参数
    ret = mbedtls_ecdh_gen_public(&grp,        // 椭圆曲线结构体
                                  &server_pri, // 输出srv私密参数d
                                  &server_pub, // 输出srv公开参数Q
                                  mbedtls_ctr_drbg_random, &ctr_drbg);
    if (ret != 0)
    {
        printf(" failed\n  ! mbedtls_ecdh_gen_public returned %d(-0x%04x)\n", ret, -ret);
        goto exit;
    }
    printf(" ok\n");

    /* show public parameter */
    // 把srv的公开参数到处到buf中
    mbedtls_ecp_point_write_binary(&grp, &server_pub, MBEDTLS_ECP_PF_UNCOMPRESSED, &olen, buf, sizeof(buf));
    dump_buf(buf, olen);

    /* 6. Calc shared secret */
    printf("\n  . Client Calc shared secret...");
    // cli计算共享密钥
    ret = mbedtls_ecdh_compute_shared(&grp,           // 椭圆曲线结构体
                                      &client_secret, // cli计算出的共享密钥
                                      &server_pub,    // 输入srv公开参数Q
                                      &client_pri,    // 输入cli本身的私密参数d
                                      mbedtls_ctr_drbg_random, &ctr_drbg);
    if (ret != 0)
    {
        printf(" failed\n  ! mbedtls_ecdh_compute_shared returned %d(-0x%04x)\n", ret, -ret);
        goto exit;
    }
    printf(" ok\n");

    /* show public parameter */
    // 把cli计算出的共享密钥导出buf中
    mbedtls_mpi_write_binary(&client_secret, buf, sizeof(buf));
    // mbedtls_ecp_point_write_binary(&grp, &client_secret, MBEDTLS_ECP_PF_UNCOMPRESSED, &olen, buf, sizeof(buf));
    dump_buf(buf, olen);

    /* 7. Server Calc shared secret */
    printf("\n  . Server Calc shared secret...");
    // srv计算共享密钥
    ret = mbedtls_ecdh_compute_shared(&grp,           // 椭圆曲线结构体
                                      &server_secret, // srv计算出的共享密钥
                                      &client_pub,    // 输入cli公开参数Q
                                      &server_pri,    // 输入srv本身的私密参数d
                                      mbedtls_ctr_drbg_random, &ctr_drbg);
    if (ret != 0)
    {
        printf(" failed\n  ! mbedtls_ecdh_compute_shared returned %d(-0x%04x)\n", ret, -ret);
        goto exit;
    }
    printf(" ok\n");

    /* show public parameter */
    // 把srv计算出的共享密钥导出buf中
    mbedtls_mpi_write_binary(&server_secret, buf, sizeof(buf));
    dump_buf(buf, olen);

    /* 8. mpi compare */
    // 比较2个大数是否相等
    ret = mbedtls_mpi_cmp_mpi(&server_secret, &client_secret);
    printf("compare result: %d\r\n", ret);

    // 将buf中的密钥还原为mbedtls_mpi
    mbedtls_mpi_init(&test_secret);
    if (mbedtls_mpi_read_binary(&test_secret, buf, sizeof(buf)) == 0)
        mbedtls_printf("test_secret generate success from buf\n");
    else
        mbedtls_printf("test_secret generate failed from buf\n");

    ret = mbedtls_mpi_cmp_mpi(&test_secret, &client_secret);
    mbedtls_printf("test_secret, client_secret compare result: %d\r\n", ret);
    mbedtls_mpi_free(&test_secret);

exit:

    /* 10. release structure */
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
    mbedtls_mpi_free(&client_secret);
    mbedtls_mpi_free(&server_secret);
    mbedtls_mpi_free(&client_pri);
    mbedtls_mpi_free(&server_pri);
    mbedtls_ecp_group_free(&grp);
    mbedtls_ecp_point_free(&client_pub);
    mbedtls_ecp_point_free(&server_pub);

    return ret;
}

int main(void)
{
    mbedtls_ecdh_test();
    return 0;
}