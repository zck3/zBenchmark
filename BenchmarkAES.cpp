/*=========================================================================
 * This file is part of zBenchmark, which is a system benchmarking tool.
 * (C) 2021, 2026 Zack T Smith.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * The author may be reached at 3 at zs3 dot m3.
 *=======================================================================*/

#include "defs.h"
#include "Utility.h"
#include "BenchmarkAES.h"
#include "declarationOfIndependence.h"

#include <openssl/aes.h>
#include <openssl/evp.h>

//----------------------------------------------------------------------------
// Method:	measure 
// Purpose:	Perform AES encryption benchmark, report results.
// Returns:	Time in seconds to run.
//----------------------------------------------------------------------------
float BenchmarkAES::measure () 
{
	char message[128];

	//-------------------------
	// Create 64MB of plaintext.
	//
	const int nDuplications = 8192;
	size_t declarationLength = strlen (declarationOfIndependence8192Bytes);
	const unsigned char *plaintext = (const unsigned char*) calloc (declarationLength * nDuplications + 16, 1);
	for (int i=0; i < nDuplications; i++) {
		memcpy ((void*) (plaintext + i * declarationLength), declarationOfIndependence8192Bytes, declarationLength);
	}

	size_t plaintextLength = strlen ((const char*) plaintext);

	snprintf (message, sizeof(message), "Plaintext is %.1f MB\nEncrypting and decrypting...", (double)plaintextLength / 1048576.0);
	emit sendTextToUI(strdup(message));

	double totalEncryptionTime = 0;
	double totalDecryptionTime = 0;

	size_t totalBytesEncrypted = 0;
	size_t totalBytesDecrypted = 0;

	for (size_t round = 0; round < N_AES_ROUNDS; round++) {

		//---------------------------------
		// Initialize key, IV, and contexts.
		//
		unsigned char encryptionKey[128];
		snprintf ((char*)encryptionKey, sizeof(encryptionKey), 
				"This is test key %lu!", round);
		size_t keyLength = strlen ((char*)encryptionKey);

		unsigned char key[32], initializationVector[32];
		int count = EVP_BytesToKey(EVP_aes_256_cbc(), 
				EVP_sha1(), 
				NULL, 
				encryptionKey, keyLength, 
				6, 
				key, initializationVector);
		if (count != 32) {
			printf ("Wrong resultant key size %i bits\n", count << 3);
			return BENCHMARK_ERROR;
		}

		EVP_CIPHER_CTX *encryptionContext = EVP_CIPHER_CTX_new();

		EVP_CIPHER_CTX_init (encryptionContext);
		EVP_EncryptInit_ex (encryptionContext, EVP_aes_256_cbc(), 
				NULL, key, initializationVector);

		EVP_CIPHER_CTX *decryptionContext = EVP_CIPHER_CTX_new();

		EVP_CIPHER_CTX_init (decryptionContext);
		EVP_DecryptInit_ex (decryptionContext, EVP_aes_256_cbc(), 
				NULL, key, initializationVector);

		int ciphertextLength = plaintextLength + AES_BLOCK_SIZE;
		unsigned char *ciphertext = (unsigned char*) malloc(ciphertextLength);

		//-----------
		// Encrypt 
		//
		double t0 = getPreciseTime();

		EVP_EncryptInit_ex (encryptionContext, NULL, NULL, NULL, NULL);
		EVP_EncryptUpdate (encryptionContext, ciphertext, &ciphertextLength, plaintext, plaintextLength);

		int finalBytesLength = 0;
		EVP_EncryptFinal_ex (encryptionContext, ciphertext + ciphertextLength, &finalBytesLength);

		ciphertextLength += finalBytesLength;

		double now = getPreciseTime();
		double t = now - t0;
		totalEncryptionTime += t;

		totalBytesEncrypted += plaintextLength;

		//-----------
		// Decrypt 
		//
		t0 = now;
		EVP_DecryptInit_ex (decryptionContext, NULL, NULL, NULL, NULL);

		int decryptedLength = ciphertextLength;
		int finalDecryptedLength = 0;
		unsigned char *decryptedBuffer = (unsigned char*) malloc(decryptedLength);

		EVP_DecryptUpdate (decryptionContext, decryptedBuffer, &decryptedLength, ciphertext, ciphertextLength);
		EVP_DecryptFinal_ex (decryptionContext, decryptedBuffer + decryptedLength, &finalDecryptedLength);
		finalDecryptedLength += decryptedLength;

		t = getPreciseTime() - t0;

		if (plaintextLength == (size_t)finalDecryptedLength && !memcmp (plaintext, decryptedBuffer, plaintextLength)) {
			printf("Round %lu: Decrypted text is the same as the plaintext.\n", round);
		} else {
			printf("Round %lu: Decrypted text is different from the plaintext.\n", round);
		}

		totalBytesDecrypted += ciphertextLength;

		free (decryptedBuffer);

		totalDecryptionTime += t;

		free (ciphertext);

		EVP_CIPHER_CTX_cleanup (encryptionContext);
		EVP_CIPHER_CTX_cleanup (decryptionContext);
	}

	free ((void*) plaintext);

	double encryptionRate = (double) totalBytesEncrypted;
	encryptionRate /= totalEncryptionTime;
	encryptionRate /= ONE_MEGABYTE;

	double decryptionRate = (double) totalBytesDecrypted;
	decryptionRate /= totalDecryptionTime;
	decryptionRate /= ONE_MEGABYTE;

	double averageRate = (encryptionRate + decryptionRate) / 2.0;

	snprintf (message, sizeof(message), 
		"\nEncryption rate = %.1f MB/second\nDecryption rate = %.1f MB/second\nAverage = %.1f MB/second\n",
		encryptionRate, decryptionRate, averageRate);
	emit sendTextToUI(strdup(message));

	return averageRate;
}

