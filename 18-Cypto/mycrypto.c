#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <crypto/hash.h>
#include <crypto/skcipher.h>
#include <linux/scatterlist.h>

#define __DRIVER_NAME	"mycrypto"

struct crypto_shash *my_aes_cmac_key_setup(u8 *k, size_t k_len)
{
	char *algo = "cmac(aes)";
	struct crypto_shash *tfm;

	tfm = crypto_alloc_shash(algo, 0, 0);
	if (!IS_ERR(tfm))
		crypto_shash_setkey(tfm, k, k_len);

	return tfm;
}

int my_aes_cmac(struct crypto_shash *tfm, u8 *data, size_t len, u8 *out)
{
	SHASH_DESC_ON_STACK(desc, tfm);
	int err;

	desc->tfm = tfm;

	/* crypto_shash_digest is short hand of crypto_shash_init, updata & final */
	err = crypto_shash_digest(desc, data, len, out);

	return err;
}

void my_aes_cmac_key_free(struct crypto_shash *tfm)
{
	crypto_free_shash(tfm);
}

int my_aes_cmac_demo(u8 *key, size_t key_sz, u8 *msg, size_t msg_len, u8 *mact)
{
	struct crypto_shash *shash;
	int err;

	shash = my_aes_cmac_key_setup(key, key_sz);
	if (IS_ERR(shash)) {
		pr_err("%s: could not allocate crypto cmac-aes\n", __DRIVER_NAME);
		err = -ENOMEM;
		goto my_aes_cmac_demo_end;
	}

	err = my_aes_cmac(shash, msg, msg_len, mact);

my_aes_cmac_demo_end:
	my_aes_cmac_key_free(shash);

	return err;
}

struct crypto_skcipher *my_aes_cbc_key_setup(u8 *k, size_t k_len)
{
	char *algo = "cbc(aes)";
	struct crypto_skcipher *tfm;

	tfm = crypto_alloc_skcipher(algo, 0, CRYPTO_ALG_ASYNC);
	if (!IS_ERR(tfm))
		crypto_skcipher_setkey(tfm, k, k_len);

	return tfm;
}

int my_aes_cbc_encrypt(struct crypto_skcipher *tfm, u8 *data, size_t len, u8 *out)
{
	u8 iv[16];
	struct scatterlist src, dst;
	SKCIPHER_REQUEST_ON_STACK(req, tfm);
	int err;

	memset(iv, 0, 16);
	sg_init_one(&src, data, len);
	sg_init_one(&dst, out, len);

	skcipher_request_set_tfm(req, tfm);
	skcipher_request_set_callback(req, 0, NULL, NULL);
	skcipher_request_set_crypt(req, &src, &dst, len, iv);
	err = crypto_skcipher_encrypt(req);
	skcipher_request_zero(req);

	return err;
}

void my_aes_cbc_key_free(struct crypto_skcipher *tfm)
{
	crypto_free_skcipher(tfm);
}

int my_aes_cbc_encrypt_demo(u8 *key, size_t key_sz, u8 *msg, size_t msg_len, u8 *enc_data)
{
	struct crypto_skcipher *tfm;
	int err;

	tfm = my_aes_cbc_key_setup(key, key_sz);
	if (IS_ERR(tfm)) {
		pr_err("%s: could not allocate crypto aes-cbc\n", __DRIVER_NAME);
		err = -ENOMEM;
		goto my_aes_cbc_demo_end;
	}

	err = my_aes_cbc_encrypt(tfm, msg, msg_len, enc_data);

my_aes_cbc_demo_end:
	my_aes_cbc_key_free(tfm);

	return err;
}

static int mycrypto_init(void)
{
	u8 key[] = { 0x2b,0x7e,0x15,0x16,0x28,0xae,0xd2,0xa6,
		     0xab,0xf7,0x15,0x88,0x09,0xcf,0x4f,0x3c};
	size_t key_sz = ARRAY_SIZE(key);
	u8 msg[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	size_t msg_len = 32;
	u8 enc_data[32] = {0};
	size_t enc_len = ARRAY_SIZE(enc_data);
	u8 dec_data[32] = {0};
	size_t dec_len = ARRAY_SIZE(dec_data);
	u8 mact[16] = {0};
	size_t mact_len = ARRAY_SIZE(mact);
	int err;

	pr_info("%s: add the %s module", __DRIVER_NAME, __DRIVER_NAME);

	pr_debug("%s: original:\n", __DRIVER_NAME);
	print_hex_dump_debug("", DUMP_PREFIX_NONE, 16, 1, msg, msg_len, true);

	err = my_aes_cmac_demo(key, key_sz, msg, msg_len, mact);
	if (err == 0) {
		pr_debug("%s: CMAC(AES-128):\n", __DRIVER_NAME);
		print_hex_dump_debug("", DUMP_PREFIX_NONE, 16, 1, mact, mact_len, true);
	}

	err = my_aes_cbc_encrypt_demo(key, key_sz, msg, msg_len, enc_data);
	if (err == 0) {
		pr_debug("%s: CBC(AES-128) encryption:\n", __DRIVER_NAME);
		print_hex_dump_debug("", DUMP_PREFIX_NONE, 16, 1, enc_data, enc_len, true);
	}

	return 0;
}

static void mycrypto_exit(void)
{
	pr_info("%s: remove the %s module", __DRIVER_NAME, __DRIVER_NAME);
}

module_init(mycrypto_init);
module_exit(mycrypto_exit);

MODULE_AUTHOR("Jian-Hong Pan, <starnight@g.ncu.edu.tw>");
MODULE_DESCRIPTION("Crypto practice driver");
MODULE_LICENSE("Dual BSD/GPL");
