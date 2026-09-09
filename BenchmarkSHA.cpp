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
#include "BenchmarkSHA.h"
#include "declarationOfIndependence.h"

#include <openssl/evp.h>

//----------------------------------------------------------------------------
// Name:	Test_sha224_openssl
// Purpose:	Compute sha224 hash of specified string for nRuns times.
// Returns:	Tuple of time to execute and bytes/second.
//----------------------------------------------------------------------------
static std::tuple<double, double> Test_sha224_openssl (const char *textToBeHashed, unsigned long nRuns)
{
	double t0 = getPreciseTime();

	size_t textLength = strlen(textToBeHashed);
	bool encountered_error = false;

	for (unsigned long i=0; i < nRuns && !encountered_error; i++) {
#ifdef deprecated_approach
		unsigned char hash[SHA224_DIGEST_LENGTH];
		SHA256_CTX context;
		SHA224_Init (&context);
		SHA224_Update (&context, textToBeHashed, strlen(textToBeHashed)); 
		SHA224_Final (hash, &context);
#endif
		EVP_MD_CTX *ctx = EVP_MD_CTX_new ();
		if (ctx == NULL) {
			encountered_error = true;
			break;
		}
		if (EVP_DigestInit_ex (ctx, EVP_sha224(), NULL) != 1) {
			encountered_error = true;
			break;
		}
		if (EVP_DigestUpdate (ctx, textToBeHashed, textLength) != 1) {
			encountered_error = true;
			break;
		}
		unsigned char digest[EVP_MAX_MD_SIZE];
		unsigned int digest_len;
		if (EVP_DigestFinal_ex (ctx, digest, &digest_len) != 1) {
			encountered_error = true;
			break;
		}
		EVP_MD_CTX_free (ctx);
	}

	if (encountered_error) {
		return std::make_tuple(-1, 0);
	}

	double t = getPreciseTime() - t0;
	double rate = strlen(textToBeHashed) * nRuns / t;
	return std::make_tuple(t, rate);
}

//----------------------------------------------------------------------------
// Name:	Test_sha256_openssl
// Purpose:	Compute sha256 hash of specified string for nRuns times.
// Returns:	Tuple of time to execute and bytes/second.
//----------------------------------------------------------------------------
static std::tuple<double, double> Test_sha256_openssl (const char *textToBeHashed, unsigned long nRuns)
{
	double t0 = getPreciseTime();

	size_t textLength = strlen(textToBeHashed);
	bool encountered_error = false;

	for (unsigned long i=0; i < nRuns; i++) {
#ifdef deprecated_approach
		unsigned char hash[SHA256_DIGEST_LENGTH];
		SHA256_CTX context;
		SHA256_Init (&context);
		SHA256_Update (&context, textToBeHashed, strlen(textToBeHashed)); 
		SHA256_Final (hash, &context);
#endif
		EVP_MD_CTX *ctx = EVP_MD_CTX_new ();
		if (ctx == NULL) {
			encountered_error = true;
			break;
		}
		if (EVP_DigestInit_ex (ctx, EVP_sha256(), NULL) != 1) {
			encountered_error = true;
			break;
		}
		if (EVP_DigestUpdate (ctx, textToBeHashed, textLength) != 1) {
			encountered_error = true;
			break;
		}
		unsigned char digest[EVP_MAX_MD_SIZE];
		unsigned int digest_len;
		if (EVP_DigestFinal_ex (ctx, digest, &digest_len) != 1) {
			encountered_error = true;
			break;
		}
		EVP_MD_CTX_free (ctx);
	}

	if (encountered_error) {
		return std::make_tuple(-1, 0);
	}

	double t = getPreciseTime() - t0;
	double rate = strlen(textToBeHashed) * nRuns / t;
	return std::make_tuple(t, rate);
}

//----------------------------------------------------------------------------
// Name:	Test_sha384_openssl
// Purpose:	Compute sha384 hash of specified string for nRuns times.
// Returns:	Tuple of time to execute and bytes/second.
//----------------------------------------------------------------------------
static std::tuple<double, double> Test_sha384_openssl (const char *textToBeHashed, unsigned long nRuns)
{

	double t0 = getPreciseTime();

	size_t textLength = strlen(textToBeHashed);
	bool encountered_error = false;

	for (unsigned long i=0; i < nRuns; i++) {
#ifdef deprecated_approach
		unsigned char hash[SHA384_DIGEST_LENGTH];
		SHA512_CTX context;
		SHA384_Init (&context);
		SHA384_Update (&context, textToBeHashed, strlen(textToBeHashed)); 
		SHA384_Final (hash, &context);
#endif
		EVP_MD_CTX *ctx = EVP_MD_CTX_new ();
		if (ctx == NULL) {
			encountered_error = true;
			break;
		}
		if (EVP_DigestInit_ex (ctx, EVP_sha384(), NULL) != 1) {
			encountered_error = true;
			break;
		}
		if (EVP_DigestUpdate (ctx, textToBeHashed, textLength) != 1) {
			encountered_error = true;
			break;
		}
		unsigned char digest[EVP_MAX_MD_SIZE];
		unsigned int digest_len;
		if (EVP_DigestFinal_ex (ctx, digest, &digest_len) != 1) {
			encountered_error = true;
			break;
		}
		EVP_MD_CTX_free (ctx);
	}

	if (encountered_error) {
		return std::make_tuple(-1, 0);
	}

	double t = getPreciseTime() - t0;
	double rate = strlen(textToBeHashed) * nRuns / t;
	return std::make_tuple(t, rate);
}

//----------------------------------------------------------------------------
// Name:	Test_sha512_openssl
// Purpose:	Compute sha512 hash of specified string for nRuns times.
// Returns:	Tuple of time to execute and bytes/second.
//----------------------------------------------------------------------------
static std::tuple<double, double> Test_sha512_openssl (const char *textToBeHashed, unsigned long nRuns)
{
	double t0 = getPreciseTime();

	size_t textLength = strlen(textToBeHashed);
	bool encountered_error = false;

	for (unsigned long i=0; i < nRuns; i++) {
#ifdef deprecated_approach
		unsigned char hash[SHA512_DIGEST_LENGTH];
		SHA512_CTX context;
		SHA512_Init (&context);
		SHA512_Update (&context, textToBeHashed, strlen(textToBeHashed)); 
		SHA512_Final (hash, &context);
#endif
		EVP_MD_CTX *ctx = EVP_MD_CTX_new ();
		if (ctx == NULL) {
			encountered_error = true;
			break;
		}
		if (EVP_DigestInit_ex (ctx, EVP_sha512(), NULL) != 1) {
			encountered_error = true;
			break;
		}
		if (EVP_DigestUpdate (ctx, textToBeHashed, textLength) != 1) {
			encountered_error = true;
			break;
		}
		unsigned char digest[EVP_MAX_MD_SIZE];
		unsigned int digest_len;
		if (EVP_DigestFinal_ex (ctx, digest, &digest_len) != 1) {
			encountered_error = true;
			break;
		}
		EVP_MD_CTX_free (ctx);
	}

	if (encountered_error) {
		return std::make_tuple(-1, 0);
	}

	double t = getPreciseTime() - t0;
	double rate = strlen(textToBeHashed) * nRuns / t;
	return std::make_tuple(t, rate);
}


//----------------------------------------------------------------------------
// Method:	measure 
// Purpose:	Perform SHA2 hashing benchmark, report results.
// Returns:	Time in seconds to run.
//----------------------------------------------------------------------------
float BenchmarkSHA::measure () 
{
	double t, rate;
	char message[128];
	double averageRate = 0;

	const char *textToBeHashed = declarationOfIndependence8192Bytes;

	snprintf (message, sizeof(message), "sha224 of %lu bytes, %lu iterations = ", (unsigned long)strlen(textToBeHashed), N_SHA_RUNS);
	emit sendTextToUI(strdup(message));
	std::tuple<double, double> tuple224 = Test_sha224_openssl (textToBeHashed, N_SHA_RUNS);
	t = std::get<0>(tuple224);
	rate = std::get<1>(tuple224);
	averageRate += rate;
	snprintf (message, sizeof(message), "%.1f MB/sec (%.2f seconds)\n", rate / (double)ONE_MEGABYTE, t);
	emit sendTextToUI(strdup(message));

	snprintf (message, sizeof(message), "sha256 of %lu bytes, %lu iterations = ", (unsigned long)strlen(textToBeHashed), N_SHA_RUNS);
	emit sendTextToUI(strdup(message));
	std::tuple<double, double> tuple256 = Test_sha256_openssl (textToBeHashed, N_SHA_RUNS);
	t = std::get<0>(tuple256);
	rate = std::get<1>(tuple256);
	averageRate += rate;
	snprintf (message, sizeof(message), "%.1f MB/sec (%.2f seconds)\n", rate / (double)ONE_MEGABYTE, t);
	emit sendTextToUI(strdup(message));

	snprintf (message, sizeof(message), "sha384 of %lu bytes, %lu iterations = ", (unsigned long)strlen(textToBeHashed), N_SHA_RUNS);
	emit sendTextToUI(strdup(message));
	std::tuple<double, double> tuple384 = Test_sha384_openssl (textToBeHashed, N_SHA_RUNS);
	t = std::get<0>(tuple384);
	rate = std::get<1>(tuple384);
	averageRate += rate;
	snprintf (message, sizeof(message), "%.1f MB/sec (%.2f seconds)\n", rate / (double)ONE_MEGABYTE, t);
	emit sendTextToUI(strdup(message));

	snprintf (message, sizeof(message), "sha512 of %lu bytes, %lu iterations = ", (unsigned long)strlen(textToBeHashed), N_SHA_RUNS);
	emit sendTextToUI(strdup(message));
	std::tuple<double, double> tuple512 = Test_sha512_openssl (textToBeHashed, N_SHA_RUNS);
	t = std::get<0>(tuple512);
	rate = std::get<1>(tuple512);
	averageRate += rate;
	snprintf (message, sizeof(message), "%.1f MB/sec (%.2f seconds)\n", rate / (double)ONE_MEGABYTE, t);
	emit sendTextToUI(strdup(message));

	averageRate /= (double)ONE_MEGABYTE;
	averageRate /= 4.0;

	snprintf (message, sizeof(message), "Average rate = %.1f MB/sec.\n", averageRate);
	emit sendTextToUI(strdup(message));

	return averageRate;
}

