#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "mbedtls/platform.h"
#include "mbedtls/ssl.h"
#include "mbedtls/ecp.h"
#include "mbedtls/aes.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/sha256.h"
#include "mbedtls/rsa.h"
#include "mbedtls/ecdsa.h"
#include "mbedtls/version.h"

#define SERVER_PORT 1212
#define BUFFER_SIZE 1024
#define bufLen 65
#define SERVER_IP "127.0.0.1"
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
void aes_ctr_encrypt(const unsigned char *key, const unsigned char *input, unsigned char *output, size_t length)
{
    mbedtls_aes_context aes;
    unsigned char iv[16] = {0}; // 使用零IV进行CTR模式
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_enc(&aes, key, 256);

    size_t nc_off = 0;
    unsigned char stream_block[16] = {0};

    mbedtls_aes_crypt_ctr(&aes, length, &nc_off, iv, stream_block, input, output);

    mbedtls_aes_free(&aes);
}

void aes_ctr_decrypt(const unsigned char *key, const unsigned char *input, unsigned char *output, size_t length)
{
    aes_ctr_encrypt(key, input, output, length); // CTR解密与加密相同
}
void sign_message(mbedtls_ctr_drbg_context *ctr_drbg, mbedtls_ecp_keypair *key, const unsigned char *message, unsigned char *signature, size_t *sig_len)
{
    mbedtls_mpi r, s;
    mbedtls_mpi_init(&r);
    mbedtls_mpi_init(&s);

    // 计算消息的哈希
    unsigned char hash[32];  // 适用于SHA-256
    mbedtls_md_context_t ctx;
    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 0);
    mbedtls_md_starts(&ctx);
    mbedtls_md_update(&ctx, message, strlen((const char *)message));
    mbedtls_md_finish(&ctx, hash);
    mbedtls_md_free(&ctx);

    // 签名
    if (mbedtls_ecdsa_sign(&key->grp, &r, &s, &key->d, hash, sizeof(hash), mbedtls_ctr_drbg_random, ctr_drbg) != 0) {
        printf("签名失败\n");
        mbedtls_mpi_free(&r);
        mbedtls_mpi_free(&s);
        return;
    }

    // 将签名结果r和s序列化到signature数组
    size_t r_len = mbedtls_mpi_size(&r);
    size_t s_len = mbedtls_mpi_size(&s);
    *sig_len = r_len + s_len;

    // 将r和s写入signature
    mbedtls_mpi_write_binary(&r, signature, r_len);
    mbedtls_mpi_write_binary(&s, signature + r_len, s_len);

    // 清理资源
    mbedtls_mpi_free(&r);
    mbedtls_mpi_free(&s);
}

int verify_signature(mbedtls_ecp_keypair *key, const unsigned char *message, const unsigned char *signature, size_t sig_len)
{
    mbedtls_md_context_t ctx;
    unsigned char hash[32];

    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);
    mbedtls_md_starts(&ctx);
    mbedtls_md_update(&ctx, message, strlen((const char *)message));
    mbedtls_md_finish(&ctx, hash);
    mbedtls_md_free(&ctx);

    return mbedtls_ecdsa_verify(&key->grp, hash, sizeof(hash), &key->Q, signature, sig_len);
}
int main()
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr;
    socklen_t client_len = sizeof(server_addr);
    int ret = 0;

    printf("mbedTLS version: %s\n", MBEDTLS_VERSION_STRING_FULL);

    // 创建TCP套接字
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0)
    {
        perror("Socket creation failed");
        return -1;
    }

    // 设置服务器地址结构
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(SERVER_PORT);

    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect failed");
        close(client_fd);
        return -1;
    }
    printf("服务器连接成功\n");

    // 初始化密钥交换和随机数生成器
    mbedtls_ecp_group grp;
    mbedtls_ecp_group_init(&grp);
    mbedtls_ecp_group_load(&grp, MBEDTLS_ECP_DP_CURVE25519);

    mbedtls_mpi client_secret, client_pri;
    mbedtls_ecp_point client_pub, server_pub;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_entropy_context entropy;

    mbedtls_mpi_init(&client_secret);
    mbedtls_mpi_init(&client_pri);
    mbedtls_ecp_point_init(&client_pub);
    mbedtls_ecp_point_init(&server_pub);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_entropy_init(&entropy);

    mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, NULL, 0);

    // 生成客户端的密钥对
    if (mbedtls_ecp_gen_keypair(&grp, &client_pri, &client_pub, mbedtls_ctr_drbg_random, &ctr_drbg) != 0)
    {
        printf("客户端密钥生成失败\n");
        return -1;
    }

    // 接收服务器公钥
    unsigned char server_pub_bin[bufLen];
    ssize_t received = recv(client_fd, server_pub_bin, sizeof(server_pub_bin), 0);
    if (received <= 0)
    {
        perror("接收服务器公钥失败");
        close(client_fd);
        return -1;
    }
    printf("server pub key, recv %d\n", received);
    dump_buf(server_pub_bin, received);
    // 将二进制公钥转换为server_pub
    mbedtls_ecp_point_read_binary(&grp, &server_pub, server_pub_bin, sizeof(server_pub_bin));
    // 将客户端公钥发送给服务器
    unsigned char client_pub_bin[bufLen];
    size_t pub_len;
    mbedtls_ecp_point_write_binary(&grp, &client_pub, MBEDTLS_ECP_PF_UNCOMPRESSED, &pub_len, client_pub_bin, sizeof(client_pub_bin));
    ssize_t sendsize = send(client_fd, client_pub_bin, pub_len, 0);
    if (sendsize <= 0)
    {
        perror("发送客户端公钥失败");
        close(client_fd);
        close(server_fd);
        return -1;
    }
    printf("client pub key, bufLen %d\n", pub_len);
    dump_buf(client_pub_bin, pub_len);
#if 0
    // 生成挑战（随机数）
    unsigned char challenge[bufLen];
    mbedtls_ctr_drbg_random(&ctr_drbg, challenge, sizeof(challenge));

    // 签名挑战
    unsigned char client_signature[64];
    size_t sig_len;
    sign_message(&ctr_drbg, &client_pri, challenge, client_signature, &sig_len);
    send(client_fd, client_signature, sig_len, 0);

    // 接收服务器签名
    unsigned char server_signature[64];
    recv(client_fd, server_signature, sizeof(server_signature), 0);

    // 验证服务器签名
    if (verify_signature(&server_pub, challenge, server_signature, sig_len) != 0) {
        printf("服务器签名验证失败！\n");
        close(client_fd);
        return -1;
    }
#endif
    // 计算共享密钥
    if (mbedtls_ecdh_compute_shared(&grp, &client_secret, &server_pub, &client_pri, mbedtls_ctr_drbg_random, &ctr_drbg) != 0)
    {
        printf("计算共享密钥失败\n");
        return -1;
    }

    // 将共享密钥转换为二进制以用于AES加密
    unsigned char shared_key[bufLen];
    mbedtls_mpi_write_binary(&client_secret, shared_key, sizeof(shared_key));
    printf("share key\n");
    dump_buf(shared_key, sizeof(shared_key));

    // 发送消息给服务器
    const char *message = "你好，服务器!";
    unsigned char encrypted_message[BUFFER_SIZE];
    aes_ctr_encrypt(shared_key, (unsigned char *)message, encrypted_message, strlen(message) + 1);
    send(client_fd, encrypted_message, strlen(message) + 1, 0);

    // 接收并解密服务器响应
    unsigned char buffer[BUFFER_SIZE];
    received = recv(client_fd, buffer, sizeof(buffer), 0);
    if (received > 0)
    {
        unsigned char decrypted_response[BUFFER_SIZE];
        aes_ctr_decrypt(shared_key, buffer, decrypted_response, received);
        printf("收到的响应: %s\n", decrypted_response);
    }

exit:
    close(client_fd);
    mbedtls_mpi_free(&client_secret);
    mbedtls_mpi_free(&client_pri);
    mbedtls_ecp_point_free(&client_pub);
    mbedtls_ecp_point_free(&server_pub);
    mbedtls_ecp_group_free(&grp);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
    return 0;
}