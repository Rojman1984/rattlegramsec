# Rattlegram

Transceive UTF-8 text messages with up to 170 bytes over audio in about a second!

## Features

- **Audio-based messaging**: Send text messages encoded as COFDMTV audio signals
- **Message encryption**: XOR-based stream cipher for message privacy (optional password protection)
- **Flexible configuration**: Adjustable sample rates, carrier frequencies, and noise protection levels
- **Repeater/Parrot mode**: Automatically retransmit received messages
- **Spectrum analyzer**: Real-time visualization of audio frequency content

## New: XOR Encryption Feature

This fork adds optional password-protected encryption to Rattlegram messages. When a password is set, all transmitted messages are encrypted and all received messages are automatically decrypted.

### How It Works

The encryption system uses:
- **CRC32 hash** to derive a 32-bit seed from the password
- **Xorshift32 PRNG** to generate a keystream from the seed
- **XOR cipher** to encrypt/decrypt data byte-by-byte

**Important Security Note**: This is a **demonstration cipher for privacy, NOT real security**. It uses:
- CRC32 (which is NOT a cryptographic hash function)
- Xorshift32 (which is NOT a cryptographically secure PRNG)

For production use requiring real security, this should be replaced with:
- **PBKDF2** or **Argon2** for key derivation
- **ChaCha20** or **AES-CTR** for encryption

### Usage

1. **Set a password**: Open the app menu and select "Set Password"
2. **Enter your password**: Both sender and receiver must use the same password
3. **Leave blank for no encryption**: If no password is set, messages are sent unencrypted
4. **Messages are automatically encrypted/decrypted**: No additional steps needed when transmitting or receiving

## Building

### Prerequisites

- Android Studio (latest version recommended)
- Android NDK (version 28 or higher for 16 KB page size support)
- Android SDK with minimum API level support

### Build Steps

1. **Clone the repository**:
   ```bash
   git clone https://github.com/Rojman1984/rattlegramsec.git
   cd rattlegramsec
   ```

2. **Open in Android Studio**:
   - Launch Android Studio
   - Select "Open an existing project"
   - Navigate to the cloned repository

3. **Sync Gradle**:
   - Android Studio will automatically prompt to sync Gradle
   - Wait for sync to complete

4. **Build the project**:
   - Select "Build" → "Rebuild Project" from the menu
   - This will compile both the Java code and C++ native libraries

5. **Run on device or emulator**:
   - Connect an Android device or start an emulator
   - Click the "Run" button or press Shift+F10

### Build Variants

The project includes standard Android build variants:
- **Debug**: For development with debugging enabled
- **Release**: Optimized production build (requires signing)

## Usage Instructions

### Basic Operation

1. **Grant microphone permission**: Required for receiving messages
2. **Set your call sign**: Menu → "Call Sign" (9 characters max, base-37: space, 0-9, A-Z)
3. **Compose a message**: Tap the compose button or Menu → "Compose"
4. **Transmit**: The message is encoded as audio and played through the speaker
5. **Receive**: The app automatically listens and decodes incoming audio messages

### Message Capacity

Messages support up to 170 bytes with three protection levels:
- **Strong protection**: 0-85 bytes (best error correction)
- **Medium protection**: 86-128 bytes
- **Normal protection**: 129-170 bytes

### Configuration Options

#### Encoder Settings
- **Sample Rate**: 8/16/32/44.1/48 kHz
- **Channel Select**: Mono/Stereo/Analytic
- **Leading Noise**: 0-4 seconds of noise before message
- **Fancy Header**: Visual preamble style
- **Carrier Frequency**: Adjustable center frequency (ultrasonic mode available)

#### Decoder Settings
- **Sample Rate**: 8/16/32/44.1/48 kHz
- **Channel Select**: Mono/First/Second/Summation/Analytic
- **Audio Source**: Default/Microphone/Camcorder/Voice Recognition/Unprocessed
- **Spectrum Analyzer**: Real-time frequency visualization

#### Repeater/Parrot Mode
- **Enable/Disable**: Toggle automatic message retransmission
- **Delay**: 0-8 seconds before retransmitting
- **Debounce**: 0-2 minutes to prevent duplicate retransmissions

### Security Recommendations

1. **Use strong passwords**: Longer, complex passwords provide better protection
2. **Keep passwords secret**: Share passwords through secure channels only
3. **Consider this privacy, not security**: This cipher can be broken by determined attackers
4. **For sensitive communications**: Use a messaging app with proper end-to-end encryption
5. **Repeater mode**: Be aware that repeaters re-encrypt with their own password

## Technical Details

### Architecture

- **Native C++ layer**: OFDM modem, polar coding, DSP, and encryption
- **Java/Android layer**: UI, audio I/O, settings management
- **JNI interface**: Bridges Java and C++ components

### Libraries Used

- [OFDM modem implementation](https://github.com/aicodix/modem) - BSD Zero Clause License
- [Polar list and ordered statistics decoding](https://github.com/aicodix/code) - BSD Zero Clause License
- [Digital signal processing](https://github.com/aicodix/dsp) - BSD Zero Clause License

### Encryption Implementation

**File**: `app/src/main/cpp/native-lib.cpp`

The `simple_xor_cipher()` function:
1. Checks if password is provided (empty password = no encryption)
2. Hashes password using CRC32 with polynomial 0x8F6E37A0
3. Seeds Xorshift32 PRNG with the hash result
4. XORs each data byte with keystream bytes

Encryption occurs:
- **Before transmission**: `configureEncoder()` encrypts the payload
- **After reception**: `fetchDecoder()` decrypts the payload (only on successful decode)

## Contributing

Contributions are welcome! Please:
1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Submit a pull request

## License

See the original Rattlegram project for licensing information.

## Disclaimer

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

**Additional Security Disclaimer**: The XOR encryption feature is provided for privacy and educational purposes only. It does not provide cryptographic security and should not be relied upon for protecting sensitive information.

