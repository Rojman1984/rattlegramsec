/*
Java native interface to C++ encoder and decoder

Copyright 2022 Ahmet Inan <inan@aicodix.de>
*/

#include <jni.h>
#define assert(expr) do {} while (0)
#include "encoder.hh"
#include "decoder.hh"

/*
 * -----------------------------------------------------------------
 * SIMPLE XOR CIPHER - DEMONSTRATION ONLY
 * -----------------------------------------------------------------
 * This is a simple stream cipher for privacy, NOT for real security.
 * It uses CRC32 as a "hash" (insecure KDF) to seed Xorshift32 (insecure PRNG).
 * A real implementation should use PBKDF2/Argon2 and ChaCha20/AES-CTR.
 */
#include "xorshift.hh"
#include "crc.hh"
#include <stdint.h>
#include <cstring>

void simple_xor_cipher(uint8_t *data, int data_len, const char *password) {
    if (!password || password[0] == '\0') {
        // No password, so no encryption/decryption
        return;
    }

    // 1. "Hash" the password to create a 32-bit seed.
    // We use the CRC32 polynomial from polar.hh (0x8F6E37A0)
    CODE::CRC<uint32_t> crc_hash(0x8F6E37A0);
    for (const char* p = password; *p; ++p) {
        crc_hash(static_cast<uint8_t>(*p));
    }
    uint32_t seed = crc_hash();

    // 2. Ensure seed is never zero (would break Xorshift32)
    if (seed == 0) {
        seed = 0xDEADBEEF;  // Arbitrary non-zero constant
    }

    // 3. Seed the Xorshift32 PRNG with our password-derived seed.
    CODE::Xorshift32 keystream_prng(seed);

    // 4. Encrypt/Decrypt data in-place by XORing with the keystream.
    uint32_t key_chunk = 0;
    for (int i = 0; i < data_len; ++i) {
        // Generate a new 4-byte key chunk when needed
        if (i % 4 == 0) {
            key_chunk = keystream_prng();
        }

        // XOR the data byte with one byte from the key chunk
        data[i] ^= (key_chunk >> ((i % 4) * 8)) & 0xFF;
    }
}
/*
 * -----------------------------------------------------------------
 * END OF SIMPLE XOR CIPHER
 * -----------------------------------------------------------------
 */

static EncoderInterface *encoder;
static DecoderInterface *decoder;

extern "C" JNIEXPORT jboolean JNICALL
Java_com_aicodix_rattlegram_MainActivity_createEncoder(
	JNIEnv *,
	jobject,
	jint sampleRate) {
	if (encoder && encoder->rate() == sampleRate)
		return true;
	delete encoder;
	switch (sampleRate) {
		case 8000:
			encoder = new(std::nothrow) Encoder<8000>();
			break;
		case 16000:
			encoder = new(std::nothrow) Encoder<16000>();
			break;
		case 32000:
			encoder = new(std::nothrow) Encoder<32000>();
			break;
		case 44100:
			encoder = new(std::nothrow) Encoder<44100>();
			break;
		case 48000:
			encoder = new(std::nothrow) Encoder<48000>();
			break;
		default:
			encoder = nullptr;
	}
	return encoder != nullptr;
}

extern "C" JNIEXPORT void JNICALL
Java_com_aicodix_rattlegram_MainActivity_destroyEncoder(
	JNIEnv *,
	jobject) {
	delete encoder;
	encoder = nullptr;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_aicodix_rattlegram_MainActivity_produceEncoder(
	JNIEnv *env,
	jobject,
	jshortArray JNI_audioBuffer,
	jint channelSelect) {

	if (!encoder)
		return false;

	jshort *audioBuffer = env->GetShortArrayElements(JNI_audioBuffer, nullptr);
	jboolean okay = false;
	if (audioBuffer)
		okay = encoder->produce(audioBuffer, channelSelect);
	env->ReleaseShortArrayElements(JNI_audioBuffer, audioBuffer, 0);
	return okay;
}

extern "C" JNIEXPORT void JNICALL
Java_com_aicodix_rattlegram_MainActivity_configureEncoder(
	JNIEnv *env,
	jobject,
	jbyteArray JNI_payload,
	jbyteArray JNI_callSign,
	jint carrierFrequency,
	jint noiseSymbols,
	jboolean fancyHeader,
	jstring JNI_password) {

	if (!encoder)
		return;

	// Get password string
	const char *password = nullptr;
	if (JNI_password != nullptr) {
		password = env->GetStringUTFChars(JNI_password, nullptr);
		if (!password)
			return;  // Failed to get password
	}

	jbyte *payload, *callSign;
	payload = env->GetByteArrayElements(JNI_payload, nullptr);
	if (!payload)
		goto payloadFail;
	callSign = env->GetByteArrayElements(JNI_callSign, nullptr);
	if (!callSign)
		goto callSignFail;

	// Create a copy of the payload for encryption (don't modify original)
	jsize payload_len = env->GetArrayLength(JNI_payload);
	{
		uint8_t *encrypted_payload = new(std::nothrow) uint8_t[payload_len];
		if (encrypted_payload) {
			// Copy original payload
			memcpy(encrypted_payload, payload, payload_len);

			// Encrypt the copy
			simple_xor_cipher(encrypted_payload, payload_len, password);

			// Configure encoder with encrypted data
			encoder->configure(
				encrypted_payload,
				reinterpret_cast<int8_t *>(callSign),
				carrierFrequency,
				noiseSymbols,
				fancyHeader);

			// Clean up
			delete[] encrypted_payload;
		}
	}

	env->ReleaseByteArrayElements(JNI_callSign, callSign, JNI_ABORT);
	callSignFail:
	env->ReleaseByteArrayElements(JNI_payload, payload, JNI_ABORT);
	payloadFail:
	if (JNI_password != nullptr && password != nullptr)
		env->ReleaseStringUTFChars(JNI_password, password);
}

extern "C" JNIEXPORT void JNICALL
Java_com_aicodix_rattlegram_MainActivity_destroyDecoder(
	JNIEnv *,
	jobject) {
	delete decoder;
	decoder = nullptr;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_aicodix_rattlegram_MainActivity_createDecoder(
	JNIEnv *,
	jobject,
	jint sampleRate) {
	if (decoder && decoder->rate() == sampleRate)
		return true;
	delete decoder;
	switch (sampleRate) {
		case 8000:
			decoder = new(std::nothrow) Decoder<8000>();
			break;
		case 16000:
			decoder = new(std::nothrow) Decoder<16000>();
			break;
		case 32000:
			decoder = new(std::nothrow) Decoder<32000>();
			break;
		case 44100:
			decoder = new(std::nothrow) Decoder<44100>();
			break;
		case 48000:
			decoder = new(std::nothrow) Decoder<48000>();
			break;
		default:
			decoder = nullptr;
	}
	return decoder != nullptr;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_aicodix_rattlegram_MainActivity_fetchDecoder(
	JNIEnv *env,
	jobject,
	jbyteArray JNI_payload,
	jstring JNI_password) {

	jint status = -1;
	if (!decoder)
		return status;

	// Get password string
	const char *password = nullptr;
	if (JNI_password != nullptr) {
		password = env->GetStringUTFChars(JNI_password, nullptr);
		if (!password)
			return -1;  // Failed to get password
	}

	jbyte *payload = env->GetByteArrayElements(JNI_payload, nullptr);
	if (payload) {
		// Call original fetch
		status = decoder->fetch(reinterpret_cast<uint8_t *>(payload));

		// Only decrypt if fetch was successful
		if (status >= 0) {
			jsize payload_len = env->GetArrayLength(JNI_payload);
			simple_xor_cipher(reinterpret_cast<uint8_t *>(payload), payload_len, password);
		}

		// Copy back the payload (decrypted if successful, unchanged if failed)
		env->ReleaseByteArrayElements(JNI_payload, payload, 0);
	}

	if (JNI_password != nullptr && password != nullptr)
		env->ReleaseStringUTFChars(JNI_password, password);

	return status;
}

extern "C" JNIEXPORT void JNICALL
Java_com_aicodix_rattlegram_MainActivity_stagedDecoder(
	JNIEnv *env,
	jobject,
	jfloatArray JNI_carrierFrequencyOffset,
	jintArray JNI_operationMode,
	jbyteArray JNI_callSign) {

	if (!decoder)
		return;

	jint *operationMode;
	jfloat *carrierFrequencyOffset;
	jbyte *callSign;
	carrierFrequencyOffset = env->GetFloatArrayElements(JNI_carrierFrequencyOffset, nullptr);
	if (!carrierFrequencyOffset)
		goto carrierFrequencyOffsetFail;
	operationMode = env->GetIntArrayElements(JNI_operationMode, nullptr);
	if (!operationMode)
		goto operationModeFail;
	callSign = env->GetByteArrayElements(JNI_callSign, nullptr);
	if (!callSign)
		goto callSignFail;

	decoder->staged(
		reinterpret_cast<float *>(carrierFrequencyOffset),
		reinterpret_cast<int32_t *>(operationMode),
		reinterpret_cast<uint8_t *>(callSign));

	env->ReleaseByteArrayElements(JNI_callSign, callSign, 0);
	callSignFail:
	env->ReleaseIntArrayElements(JNI_operationMode, operationMode, 0);
	operationModeFail:
	env->ReleaseFloatArrayElements(JNI_carrierFrequencyOffset, carrierFrequencyOffset, 0);
	carrierFrequencyOffsetFail:;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_aicodix_rattlegram_MainActivity_feedDecoder(
	JNIEnv *env,
	jobject,
	jshortArray JNI_audioBuffer,
	jint sampleCount,
	jint channelSelect) {

	jboolean status = false;

	if (!decoder)
		return status;

	jshort *audioBuffer;
	audioBuffer = env->GetShortArrayElements(JNI_audioBuffer, nullptr);
	if (!audioBuffer)
		goto audioBufferFail;

	status = decoder->feed(
		reinterpret_cast<int16_t *>(audioBuffer),
		sampleCount, channelSelect);

	env->ReleaseShortArrayElements(JNI_audioBuffer, audioBuffer, JNI_ABORT);
	audioBufferFail:

	return status;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_aicodix_rattlegram_MainActivity_processDecoder(
	JNIEnv *,
	jobject) {

	if (!decoder)
		return STATUS_HEAP;

	return decoder->process();
}

extern "C" JNIEXPORT void JNICALL
Java_com_aicodix_rattlegram_MainActivity_spectrumDecoder(
	JNIEnv *env,
	jobject,
	jintArray JNI_spectrumPixels,
	jintArray JNI_spectrogramPixels,
	jint spectrumTint) {

	if (!decoder)
		return;

	jint *spectrumPixels, *spectrogramPixels;
	spectrumPixels = env->GetIntArrayElements(JNI_spectrumPixels, nullptr);
	if (!spectrumPixels)
		goto spectrumFail;
	spectrogramPixels = env->GetIntArrayElements(JNI_spectrogramPixels, nullptr);
	if (!spectrogramPixels)
		goto spectrogramFail;

	decoder->spectrum(
		reinterpret_cast<uint32_t *>(spectrumPixels),
		reinterpret_cast<uint32_t *>(spectrogramPixels),
		spectrumTint);

	env->ReleaseIntArrayElements(JNI_spectrogramPixels, spectrogramPixels, 0);
	spectrogramFail:
	env->ReleaseIntArrayElements(JNI_spectrumPixels, spectrumPixels, 0);
	spectrumFail:;
}
