
### Rattlegram

Transceive UTF-8 text messages with up to 170 bytes over audio in about a second!

## Features

- **Audio-based transmission**: Send text messages via COFDM-encoded audio signals
- **Short-range communication**: Perfect for local, peer-to-peer messaging
- **Password encryption**: Optional XOR cipher encryption for message privacy
- **Configurable settings**: Adjust sample rates, carrier frequencies, and more
- **Repeater mode**: Automatically relay received messages

## New in This Version: Password Encryption

This fork adds optional password-based encryption to Rattlegram using a simple XOR stream cipher. While not suitable for high-security applications, it provides basic privacy for casual communications.

### Security Notes

⚠️ **FOR DEMONSTRATION/PRIVACY PURPOSES ONLY**

The encryption implementation uses:
- **CRC32** as a password hash (NOT cryptographically secure)
- **Xorshift32 PRNG** for keystream generation (NOT cryptographically secure)

For production security, use established cryptographic libraries with:
- **Key Derivation**: PBKDF2, Argon2, or scrypt
- **Encryption**: ChaCha20, AES-CTR, or AES-GCM

### How Encryption Works

1. **No password** = no encryption (messages sent in plaintext)
2. **With password**:
   - Sender encrypts the message before transmission
   - Receiver decrypts using the same password
   - Both parties must use identical passwords
   - Empty password on one end = garbled message on the other

## Building the App

### Prerequisites

- **Android Studio** (Arctic Fox or newer)
- **Android NDK** 28.2.13676358 or newer
- **CMake** 3.22.1 or newer
- **Minimum Android SDK**: API 21 (Android 5.0)
- **Target Android SDK**: API 35

### Build Steps

1. **Clone the repository**:
   ```bash
   git clone https://github.com/Rojman1984/rattlegramsec.git
   cd rattlegramsec
   ```

2. **Open in Android Studio**:
   - File → Open → Select the `rattlegramsec` directory
   - Wait for Gradle sync to complete

3. **Build the app**:
   ```bash
   # From command line
   ./gradlew assembleDebug

   # Or in Android Studio
   Build → Make Project
   ```

4. **Install on device**:
   ```bash
   # Via ADB
   adb install app/build/outputs/apk/debug/app-debug.apk

   # Or in Android Studio
   Run → Run 'app'
   ```

### Troubleshooting Build Issues

If you encounter CMake cache issues:

```bash
# Clean everything
./gradlew clean
rm -rf app/.cxx
rm -rf app/build
rm -rf .gradle

# Rebuild
./gradlew assembleDebug
```

## Using Rattlegram

### Basic Usage

1. **Set your call sign**: Menu → Call Sign
2. **Set encryption password** (optional): Menu → Encryption Password
3. **Compose a message**: Tap the compose icon
4. **Transmit**: Play audio near another device running Rattlegram
5. **Receive**: App automatically listens and decodes incoming messages

### Encryption Setup

**Both sender and receiver must use the same password!**

1. Open Menu → **Encryption Password**
2. Enter your shared password
3. Leave empty to disable encryption
4. Password is stored locally and persists across app restarts

### Settings

- **Sample Rate**: 8-48 kHz (higher = better quality, more bandwidth)
- **Carrier Frequency**: 1-3 kHz (default 1500 Hz)
- **Noise Symbols**: Add redundancy for noisy environments (0-22 symbols)
- **Repeater Mode**: Auto-relay received messages after a delay
- **Channel Select**: Mono, stereo, or analytic signal processing

### Tips for Best Results

- Use **8 kHz sample rate** for most environments
- Keep devices **1-3 feet apart** for optimal reception
- Reduce background noise when transmitting
- Use **noise symbols** in noisy environments (try 6 symbols)
- Test with **ping** (notification icon) before sending messages

## Technical Details

### Architecture

- **Native C++ core** (JNI): OFDM modem, polar coding, encryption
- **Java/Android UI**: Message composition, settings, audio I/O
- **Audio processing**: Real-time encoding/decoding at configurable sample rates

### Encryption Implementation

**Location**: `app/src/main/cpp/native-lib.cpp`

```cpp
// Password → CRC32 → Seed → Xorshift32 PRNG → XOR with data
void simple_xor_cipher(uint8_t *data, int data_len, const char *password);
```

**Process**:
1. Hash password with CRC32 polynomial (0x8F6E37A0)
2. Use hash as seed for Xorshift32 PRNG (with zero-seed protection)
3. Generate keystream and XOR with plaintext/ciphertext
4. Symmetric operation (encrypt and decrypt use same function)

### File Structure

```
rattlegramsec/
├── app/src/main/
│   ├── cpp/              # Native C++ code
│   │   ├── native-lib.cpp      # JNI, encryption
│   │   ├── encoder.hh          # OFDM encoder
│   │   ├── decoder.hh          # OFDM decoder
│   │   ├── xorshift.hh         # PRNG for encryption
│   │   └── crc.hh              # CRC32 for key derivation
│   ├── java/             # Android app code
│   │   └── com/aicodix/rattlegram/
│   │       └── MainActivity.java
│   └── res/              # UI resources, layouts, strings
└── README.md             # This file
```

## Original Project

Based on **Rattlegram** by aicodix GmbH:
- GitHub: https://github.com/aicodix/rattlegram
- License: BSD Zero Clause License

## License

This fork maintains the original BSD Zero Clause License.

Copyright 2022 aicodix GmbH
Encryption additions 2025

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
