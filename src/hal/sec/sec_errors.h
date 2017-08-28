/*
 * Copyright (c) 2017, CESAR.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license. See the LICENSE file for details.
 *
 */
#ifdef __cplusplus
extern "C" {
#endif

#define	ERROR_EVP_CIPHER_CTX_NEW	-1 /*creating context*/
#define ERROR_EVP_ENC_INIT			-2 /*initializing enc function*/
#define ERROR_EVP_ENC_UPDATE		-3 /*updating encrypt function*/
#define ERROR_EVP_ENC_FINAL			-4 /*finilizing enc function*/
#define ERROR_EVP_DEC_INIT			-5 /*initializing decrypt func*/
#define ERROR_EVP_DEC_UPDATE		-6 /*updating decrypt function*/
#define ERROR_EVP_DEC_FINAL			-7 /*finilizing decrypt func*/
#define ERROR_ECC_LOC_PKEY_CURVE	-8 /*creating local pkey by curve*/
#define ERROR_ECC_PEER_PKEY_CURVE	-9 /*creating peer pkey by curve*/
#define ERROR_ECC_SET_LOC_PKEY		-10 /*setting local public key*/
#define ERROR_ECC_SET_PEER_PKEY		-11 /*setting peer public key*/
#define ERROR_ECC_SET_PRIV_KEY		-12 /*setting private key*/
#define ERROR_EVP_ASSIGN_PEER		-13 /*assigning peer key*/
#define ERROR_EVP_ASSIGN_KEY		-14 /*assigning private key*/
#define ERROR_EVP_DERIVE_INIT		-15 /*initializing context derivation*/
#define ERROR_EVP_DERIVE_SET_PEER	-16 /*setting peer key to context*/
#define ERROR_EVP_DERIVE_ALLOC		-17 /*allocating derivation buffer*/
#define ERROR_EVP_DERIVE			-18 /*derivating key*/
#define ERROR_GET_RANDOM			-19 /*getting random bytes*/
#define ERROR_ACCESS_URANDOM		-20 /*Cannot access urandom*/
#define ERROR_ECC_MK_KEYS			-21 /*Bad rand bytes to keys*/
#define ERROR_BAD_PADDING			-22 /*while unpadding data*/
#define	ERROR_CURVE_NOT_DEFINED		-23 /*ECC curve hasn't been defined*/
#define ERROR_NANO_DERIVE_SKEY		-24 /*derivating key on nanoecc*/
#define ERROR_FUNCTION_UNAVALIABLE	-25 /*Function currently unavaliable*/

#ifdef __cplusplus
}
#endif
