# Simplified AES Implementation

![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)

This is a simplified implementation of the Advanced Encryption Standard (AES) algorithm in C. It supports both encryption and decryption operations.

## Usage

```bash
./aes [ENC|DEC] [key] [Text]
```

- `[ENC|DEC]`: Specify whether to encrypt (`ENC`) or decrypt (`DEC`).
- `[key]`: The encryption/decryption key in hexadecimal format.
- `[Text]`: The text to be encrypted or decrypted in hexadecimal format.

## Example

### Encryption

```bash
./aes ENC 2B7E151628AED2A6 3243F6A8885A308D
```

Output:
```
-------------------Simplified AES--------------------
Initial State:
32 43
F6 A8
--------------------------------------------------------
After AddRoundKey (PreRoundKey):
State Matrix:
32 43
F6 A8
PreRound Key:
2B 7E
--------------------------------------------------------
After SubNibbles, ShiftRows, MixColumns, AddRoundKey (Round 1):
State Matrix:
5F 45
85 24
Round 1 Key:
A0 88
--------------------------------------------------------
After SubNibbles, ShiftRows, AddRoundKey (Round 2):
State Matrix:
63 EB
AA 9A
Round 2 Key:
F8 47
--------------------------------------------------------

Encrypted: 63EBA3AA9AF847F8
```

### Decryption

```bash
./aes DEC 2B7E151628AED2A6 63EBA3AA9AF847F8
```

Output:
```
-------------------Simplified AES--------------------
Initial State:
63 EB
AA 9A
--------------------------------------------------------
After AddRoundKey (Round2 key):
State Matrix:
23 6C
7E 55
Round 2 Key:
F8 47
--------------------------------------------------------
After InverseSubNibbles, Inverse ShiftRows, AddRoundKey (Round 1), InvMixColumns:
State Matrix:
82 A8
5C A4
Round 1 Key:
A0 88
--------------------------------------------------------
After InverseSubNibbles, Inverse ShiftRows, AddRoundKey (PreRoundKey):
State Matrix:
32 43
F6 A8
Pre round key:
2B 7E
--------------------------------------------------------

Decrypted: 3243F6A8885A308D
