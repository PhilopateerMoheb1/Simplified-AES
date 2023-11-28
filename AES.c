#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define S-Box
static const int sBox[16] = {
    0x9, 0x4, 0xA, 0xB,
    0xD, 0x1, 0x8, 0x5,
    0x6, 0x2, 0x0, 0x3,
    0xC, 0xE, 0xF, 0x7};

// Define Inverse S-Box
static const int sBoxI[16] = {
    0xA, 0x5, 0x9, 0xB,
    0x1, 0x7, 0x8, 0xF,
    0x6, 0x0, 0x2, 0x3,
    0xC, 0x4, 0xD, 0xE};

// Function to substitute word
int sub_word(int word)
{
    return (sBox[word >> 4] << 4) + sBox[word & 0x0F];
}

// Function to rotate word for the expand key that occur in g function
// g function that takes input of ex: w1 and its ouput xored with w0 that gets w2
int rot_word(int word)
{
    return ((word & 0x0F) << 4) + ((word & 0xF0) >> 4);
}

// Function to convert integer to state
void int_to_state(int n, int state[4])
{
    /*
    Ex:89A8
    [
        8   A
        9   8
    ]

    */
    state[0] = n >> 12 & 0xF;
    state[1] = (n >> 4) & 0xF;
    state[2] = (n >> 8) & 0xF;
    state[3] = n & 0xF;
}
// Function for key expansion
void key_expansion(int key, int *pre_round_key, int *round1_key, int *round2_key)
{
    // Round constants
    const int Rcon1 = 0x80;
    const int Rcon2 = 0x30;

    // Calculating value of each word
    // converting it into states like each place has a value of w
    // why we convert it to states as to handle it easily i make it in form that each is xored with the corresponding but in state form
    int w[6];
    w[0] = (key & 0xFF00) >> 8;
    w[1] = key & 0x00FF;
    // momken a5ali di g function
    w[2] = w[0] ^ (sub_word(rot_word(w[1])) ^ Rcon1); // g function its output xored with w[0]
    w[3] = w[2] ^ w[1];
    w[4] = w[2] ^ (sub_word(rot_word(w[3])) ^ Rcon2);
    w[5] = w[4] ^ w[3];

    int_to_state((w[0] << 8) + w[1], pre_round_key);
    int_to_state((w[2] << 8) + w[3], round1_key);
    int_to_state((w[4] << 8) + w[5], round2_key);
}

// Function for Galois field multiplication
int gf_mult(int a, int b)
{
    int product = 0;
    a = a & 0x0F;
    b = b & 0x0F;

    while (a && b)
    {
        if (b & 1)
        {
            product = product ^ a;
        }

        a = a << 1;

        if (a & (1 << 4))
        {
            a = a ^ 0b10011;
        }

        b = b >> 1;
    }

    return product;
}

// Function to convert state to integer
int state_to_int(int state[4])
{
    // return to bits as input again

    return (state[0] << 12) + (state[2] << 8) + (state[1] << 4) + state[3];
}

// Function to add round key
void add_round_key(int s1[4], int s2[4])
{
    // xored with its corresponding in state matrix of the text with the round key
    for (int i = 0; i < 4; i++)
    {
        s1[i] = s1[i] ^ s2[i];
    }
}

// Function for sub nibbles transformation
void sub_nibbles(const int *sbox, int state[4])
{
    // it subsitute with state as the location of sbox 0-F and the hole range of hexa is 0-F
    // and i get the upper two bits for rows and lower two bits for column is equalivalent to put the value in sbox and see the result
    for (int i = 0; i < 4; i++)
    {
        state[i] = sbox[state[i]];
    }
}

// Function for shift rows transformation
void shift_rows(int state[4])
{
    // as we convert it EX:

    /*
    [
        8   A
        9   8
    ]
     0 1 2 3
    [8,A,2,3]
    swap the last two
    as i flaten the matrix
    */
    int temp = state[2];
    state[2] = state[3];
    state[3] = temp;
}

// Function for mix columns transformation
void mix_columns(int state[4])
{
    int temp[4];
    for (int i = 0; i < 4; i++)
    {
        temp[i] = state[i];
    }
    // multiplication according to
    /*
        [            [
            1   4      8   A
            4   1   *  9   8
        ]             ]
    */
    state[0] = temp[0] ^ gf_mult(4, temp[2]);
    state[2] = temp[2] ^ gf_mult(4, temp[0]);
    state[1] = temp[1] ^ gf_mult(4, temp[3]);
    state[3] = temp[3] ^ gf_mult(4, temp[1]);
}

// Function for inverse mix columns transformation
void inverse_mix_columns(int state[4])
{
    int temp[4];
    for (int i = 0; i < 4; i++)
    {
        temp[i] = state[i];
    }

    /*
        [            [
            9   2      S00   S01
            2   9   *  S10   S11
        ]             ]
    */
    state[0] = gf_mult(9, temp[0]) ^ gf_mult(2, temp[2]);
    state[2] = gf_mult(9, temp[2]) ^ gf_mult(2, temp[0]);
    state[1] = gf_mult(9, temp[1]) ^ gf_mult(2, temp[3]);
    state[3] = gf_mult(9, temp[3]) ^ gf_mult(2, temp[1]);
}

// Function to print the state matrix in 2x2 matrix format
void print_state(int array[4])
{
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            printf("%02X ", array[i * 2 + j]);
        }
        printf("\n");
    }
}
// Function to encrypt
int encrypt(int plaintext, int pre_round_key[4], int round1_key[4], int round2_key[4])
{
    int state[4];
    int_to_state(plaintext, state);
    // First round to make encryption of the first round as if without it
    // it can be reversied easily
    /*
     Only the AddRoundKey stage makes use of the key. For this reason, the cipher
    begins and ends with an AddRoundKey stage. Any other stage, applied at the
    beginning or end, is reversible without knowledge of the key and so would add
    no security(from ref p 171 )
    */
    printf("\nInitial State:\n");
    print_state(state);
    printf("--------------------------------------------------------");
    add_round_key(state, pre_round_key);
    printf("\nAfter AddRoundKey (PreRoundKey):\n");
    printf("State Matrix:\n");
    print_state(state);
    printf("\nPreRound Key: \n");
    print_state(pre_round_key);
    printf("--------------------------------------------------------");

    // round 1
    sub_nibbles(sBox, state);
    shift_rows(state);
    mix_columns(state);
    add_round_key(state, round1_key);
    printf("\nAfter SubNibbles, ShiftRows, MixColumns, AddRoundKey (Round 1):\n");
    printf("State Matrix:\n");
    print_state(state);
    printf("\nRound 1 Key: \n");
    print_state(round1_key);
    printf("--------------------------------------------------------");
    // round 2
    sub_nibbles(sBox, state);
    shift_rows(state);
    add_round_key(state, round2_key);
    printf("\nAfter SubNibbles, ShiftRows, AddRoundKey (Round 2):\n");
    printf("State Matrix:\n");
    print_state(state);
    printf("\nRound 2 Key: \n");
    print_state(round2_key);
    printf("--------------------------------------------------------\n");

    return state_to_int(state);
}

// Function to decrypt
int decrypt(int ciphertext, int pre_round_key[4], int round1_key[4], int round2_key[4])
{
    int state[4];
    int_to_state(ciphertext, state);
    printf("\nInitial State:\n");
    printf("State Matrix:\n");
    print_state(state);
    printf("--------------------------------------------------------");
    printf("\nAfter AddRoundKey (Round2 key):\n");
    add_round_key(state, round2_key);
    printf("State Matrix:\n");
    print_state(state);
    printf("\nRound 2 Key: \n");
    print_state(round2_key);
    printf("--------------------------------------------------------");
    // round 1
    sub_nibbles(sBoxI, state);
    shift_rows(state);
    add_round_key(state, round1_key);
    inverse_mix_columns(state);
    printf("\nAfter AddRoundKey (Round 1), Inverse SubNibbles, Inverse ShiftRows, AddRoundKey (Round 1), InvMixColumns:\n");
    printf("State Matrix:\n");
    print_state(state);
    printf("\nRound 1 Key: \n");
    print_state(round1_key);
    printf("--------------------------------------------------------");

    // round 2
    sub_nibbles(sBoxI, state);
    shift_rows(state);
    add_round_key(state, pre_round_key);
    printf("\nAfter InverseSubNibbles, Inverse ShiftRows, AddRoundKey (PreRoundKey):\n");
    printf("State Matrix:\n");
    print_state(state);
    printf("\nPre round key:\n");
    print_state(pre_round_key);
    printf("--------------------------------------------------------\n");

    return state_to_int(state);
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("Usage: %s [ENC|DEC] [key] [Text]\n", argv[0]);
        return 1;
    }

    char *operation = argv[1];
    int key = strtol(argv[2], NULL, 16);
    int text = strtol(argv[3], NULL, 16);
    int pre_round_key[4], round1_key[4], round2_key[4];
    printf("--------------------Simplified AES--------------------\n");

    // Generate round keys
    key_expansion(key, pre_round_key, round1_key, round2_key);

    // Example: Encrypt
    if (strcmp(operation, "ENC") == 0)
    {
        int ciphertext = encrypt(text, pre_round_key, round1_key, round2_key);
        printf("Encrypted: %04X\n", ciphertext);
        return 1;
    }
    // Example: Decrypt
    else if (strcmp(operation, "DEC") == 0)
    {
        int plaintext = decrypt(text, pre_round_key, round1_key, round2_key);
        printf("Decrypted: %04X\n", plaintext);
        return 1;
    }
    else
    {
        printf("Invalid operation. Use ENC or DEC.\n");
        return 1;
    }
    return 0;
}
