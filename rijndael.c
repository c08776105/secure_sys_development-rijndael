/*
 * Eamonn McClelland - C08776105
 *
 * This file contains two implementations of the aes_encrypt_block and aes_decrypt_block functions
 * defined in rijndael.h as well associated functions required for both functions to complete encrypt/decrypt
 * operations of tha AES algorithm.
 *
 * Encryption:
 * Create expanded key
 * Create and add round key
 *
 * create round key
 * sub bytes
 * mix rows
 * shift columns
 * add round key
 *
 *
 * Decryption:
 * Create expanded key
 * Create and add round key
 *
 * For each round:
 * create round key
 * inverted shift rows
 * inverted sub bytes
 * add round key
 * inverted mix columns
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
// TODO: Any other files you need to include should go here

#include "rijndael.h"

#define KEY_SIZE 16 // the key size for 128bit AES encryption
#define DEBUG_ACTIVE 1
#define VECTOR_SIZE_BYTES 176 // vector size, which is == key size * total number of encryption rounds
#define ROUNDS_NUMBER 10 // number of rounds (excluding the final round) during encryption
#define MOST_SIG_BIT_MASK 0x80 // most significant bit index for bitwise operations
#define IRR_POLYNOMIAL 0x1B // fixed polynomial for finite field calculations, used in galois multiplication

// Round constant values, or rcon
unsigned char round_constant[] = {
    0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8,
    0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3,
    0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f,
    0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d,
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab,
    0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d,
    0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25,
    0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01,
    0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d,
    0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa,
    0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a,
    0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02,
    0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a,
    0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef,
    0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94,
    0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04,
    0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f,
    0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5,
    0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33,
    0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb};

// S-box value constant
unsigned char SBox[] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16};

// Inverse S-Box values
unsigned char SBoxInv[] = {
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d};

/**
 * Pretty printing the supplied label and value array
 */
void pprint(char *label, unsigned char *val)
{
  printf("%s:\n", label);
  for (int i = 0; i < VECTOR_SIZE_BYTES; i++)
  {
    printf("%02X ", val[i]);
    if ((i + 1) % 16 == 0)
    {
      printf("\n");
    }
  }

  printf("\n\n");
}

/**
 * Implements Galois multiplication, taking 2x8-bit values and multiplying them within
 * the finite field.
 */
unsigned char galois_multiplication(unsigned char a, unsigned char b)
{
  unsigned char result = 0;
  unsigned char sig_bit_set;

  for (int bitIndex = 0; bitIndex < 8; bitIndex++)
  {
    // If the least significant bit of b is set, XOR with a
    if ((b & 1) == 1)
    {
      result ^= a;
    }

    // Check if the most significant bit is set, on a
    sig_bit_set = (a & MOST_SIG_BIT_MASK);

    // Shift a left by 1 bit (multiplies by 2 in the Galois field)
    a <<= 1;

    // If the most significant bit for a was set before shifting, XOR a with 0x1B (irreductable polynomial)
    // This ensures the variable a remains in the finite field
    if (sig_bit_set == MOST_SIG_BIT_MASK)
    {
      a ^= IRR_POLYNOMIAL;
    }

    // Shift b to the right by 1 bit before beginning the next iteration
    b >>= 1;
  }

  return result;
}

/*
 * Operations used when encrypting a block
 */

/**
 * Each byte of the block is replaced with a corresponding byte
 * from a substitution box (S-box), using a pre-defined substitution table.
 */
void sub_bytes(unsigned char *block)
{
  for (int i = 0; i < 16; i++)
  {
    block[i] = SBox[block[i]];
  }
}

/**
 * The bytes in each row of the block are shifted cyclically to the left.
 * The first row is not shifted, the second row is shifted by one byte to the left,
 * the third row by two bytes, and the fourth row by three bytes.
 *
 * The variable block represents a 4x4 matrix as a one dimensional array
 */
void shift_rows(unsigned char *block)
{
  // For each of the 4 rows
  for (int i = 0; i < 4; i++)
  {
    unsigned char *b = block + i * 4;
    unsigned char tmp;

    // Shift the row to the left by 1
    for (int j = 0; j < i; j++)
    {
      tmp = b[0];
      for (int k = 0; k < 3; k++)
      {
        b[k] = b[k + 1];
      }
      b[3] = tmp;
    }
  }
}

/**
 * Each column of the block is transformed using a matrix multiplication operation.
 *
 * Adopted from:
 * https://github.com/m3y54m/aes-in-c/blob/46096b255cffda73f2fc04c3ea04b68a4a7f056f/src/main.c#L357
 */
void mix_columns(unsigned char *block)
{
  // Create four columns to iterate over
  unsigned char column[4];

  // Loop through each column we want to create
  for (int colIndex = 0; colIndex < 4; colIndex++)
  {
    // Loop over each row and create one for the current column
    for (int rowIndex = 0; rowIndex < 4; rowIndex++)
    {
      column[rowIndex] = block[(rowIndex * 4) + colIndex];
    }

    // Perform actual column mixing. First copy the col data for the
    // galois multiplication operations
    unsigned char cpy[4];
    for (int colCpyIndex = 0; colCpyIndex < 4; colCpyIndex++)
    {
      cpy[colCpyIndex] = column[colCpyIndex];
    }

    // Each column is multiplied by a fixed matrix in the galois field, followed
    // by an XOR (^) of the results to calculate the new column state.
    // The following is the fixed matrix each column is multipled by: {2, 1, 1, 3}
    for (int gindex = 0; gindex < 4; ++gindex)
    {
      column[gindex] =
          galois_multiplication(cpy[gindex], 2) ^
          galois_multiplication(cpy[(gindex + 3) % 4], 1) ^
          galois_multiplication(cpy[(gindex + 2) % 4], 1) ^
          galois_multiplication(cpy[(gindex + 1) % 4], 3);
    }

    // Set the values back in the variable at the block pointer
    for (int j = 0; j < 4; j++)
    {
      block[(j * 4) + colIndex] = column[j];
    }
  }
}

/*
 * Operations used when decrypting a block
 */
void invert_sub_bytes(unsigned char *block)
{
  for (int byteIndex = 0; byteIndex < 16; byteIndex++)
  {
    block[byteIndex] = SBoxInv[block[byteIndex]];
  }
}

void invert_shift_rows(unsigned char *block)
{
  for (int rowIndex = 0; rowIndex < 4; rowIndex++)
  {
    unsigned char *block_segment = block + rowIndex * 4;
    unsigned char previousElement;

    for (int row = 0; row < rowIndex; row++)
    {
      previousElement = block_segment[3];
      for (int col = 3; col > 0; col--)
      {
        block_segment[col] = block_segment[col - 1];
      }
      block_segment[0] = previousElement;
    }
  }
}

void invert_mix_columns(unsigned char *block)
{
  unsigned char column[4];

  // iterate over the 4 columns
  for (int col = 0; col < 4; col++)
  {
    // construct one column by iterating over the 4 rows
    for (int row = 0; row < 4; row++)
    {
      column[row] = block[(row * 4) + col];
    }

    // Create a temporary copy of the column before
    // performing galois multiplication
    unsigned char tempCol[4];
    memcpy(tempCol, column, 4);

    // Perform galois multiplication over all values, and combine using XOR
    for (int i = 0; i < 4; i++) {
      column[i] = galois_multiplication(tempCol[(0 + i) % 4], 14) ^
                  galois_multiplication(tempCol[(3 + i) % 4], 9) ^
                  galois_multiplication(tempCol[(2 + i) % 4], 13) ^
                  galois_multiplication(tempCol[(1 + i) % 4], 11);
    }

    // Restore the values in the block array
    for (int rowIndex = 0; rowIndex < 4; rowIndex++)
    {
      int blockIndex = (rowIndex * 4) + col;
      block[blockIndex] = column[rowIndex];
    }
  }
}

/*
 * This operation is shared between encryption and decryption
 *
 * The block is XORed with the current round key to introduce diffusion,
 * enhancing cryptographic security of the algorithm.
 */
void add_round_key(unsigned char *block, unsigned char *round_key)
{
  for (int i = 0; i < 16; i++)
  {
    block[i] = block[i] ^ round_key[i];
  }
}

/*
 * This function should expand the round key. Given an input,
 * which is a single 128-bit key, it should return a 176-byte
 * vector, containing the 11 round keys one after the other
 */
unsigned char *expand_key(unsigned char *cipher_key)
{
  unsigned char *expanded_key = (unsigned char *)malloc(VECTOR_SIZE_BYTES);
  if (expanded_key == NULL)
  {
    // Handle memory allocation failure
    return NULL;
  }

  memcpy(expanded_key, cipher_key, BLOCK_SIZE);

  int bytes_generated = BLOCK_SIZE;
  int rcon_iteration = 1;
  unsigned char expansion_block[4]; // a 4 byte block for each block the key processed

  // Core key expansion operation. bytes_generated stores the bytes generated so far in
  // the expanded key. When its size hits 176, exit the loop.
  while (bytes_generated < VECTOR_SIZE_BYTES)
  {
    // Copy the previous 4 bytes into expansion_block
    memcpy(expansion_block, &expanded_key[bytes_generated - 4], 4);

    if (bytes_generated % BLOCK_SIZE == 0)
    {
      // Create a pointer of the 32bit word
      unsigned int *word_ptr = (unsigned int *)expansion_block;

      // Perform left rotation of the word
      unsigned char rightmost_byte = (unsigned char)(*word_ptr);
      *word_ptr <<= 8; // shift the pointer by 1 byte
      *word_ptr |= rightmost_byte; // set the rightmost byte to the extracted value

      // Perform sbox substitution on all 4 parts of the word
      // This is performed as a linear transformation, used to
      // increase resistance to analysis techniques
      for(int i = 0; i < 4; i++) {
        expansion_block[i] = SBox[expansion_block[i]];
      }

      // XOR the output of the rcon value from the current iteration index
      expansion_block[0] ^= round_constant[rcon_iteration++];
    }

    // XOR temp with the 4-byte block N bytes before the new expanded key.
    // This becomes the next 4 bytes in the expanded key.
    for (unsigned char i = 0; i < 4; ++i)
    {
      int indexToXor = bytes_generated - BLOCK_SIZE;
      expanded_key[bytes_generated] = expanded_key[indexToXor] ^ expansion_block[i];
      bytes_generated++;
    }
  }

  return expanded_key;
}

/**
 * Create a round key, which is a 4x4 matrix stored in a one dimensional array,
 * using the expanded key as its source.
 */
void create_round_key(unsigned char *expanded_key, unsigned char *round_key)
{
  for (int rowIndex = 0; rowIndex < 4; rowIndex++)
  {
    for (int colIndex = 0; colIndex < 4; colIndex++)
    {
      int roundKeyIndex = rowIndex + (colIndex * 4);
      int expandedKeyIndex = (rowIndex * 4) + colIndex;
      round_key[roundKeyIndex] = expanded_key[expandedKeyIndex];
    }
  }
}

/*
 * The implementations of the functions declared in the
 * header file should go here
 */
unsigned char *aes_encrypt_block(unsigned char *plaintext, unsigned char *key)
{
  unsigned char *output =
      (unsigned char *)malloc(sizeof(unsigned char) * BLOCK_SIZE);

  // Block to hold the temporarily transposed matrix to encrypt
  unsigned char block[16];

  // Copy the plaintext into the block, transposing the order.
  int plainTextIndex = 0;
  for (int col = 0; col < 4; col++)
  {
    for (int row = 0; row < 4; row++)
    {
      int blockIndex = col + (row * 4); // index to transpose to
      block[blockIndex] = plaintext[plainTextIndex];
      plainTextIndex++; // Sequential index to copy from
    }
  }

  // Expand key
  unsigned char *expanded_key = expand_key(key);

  if (DEBUG_ACTIVE == 1)
  {
    pprint("Expanded Key", expanded_key);
  }

  // Initial round
  unsigned char round_key[KEY_SIZE];
  create_round_key(expanded_key, round_key);

  add_round_key(block, round_key);

  // Rounds (start i at 1 because initial round is already complete)
  for (int roundIndex = 1; roundIndex < ROUNDS_NUMBER; roundIndex++)
  {
    create_round_key(expanded_key + 16 * roundIndex, round_key);

    //  Sub Bytes - sub_bytes
    sub_bytes(block);

    //  Shift Rows - shift_rows
    shift_rows(block);

    //  Mix Columns - mix_columns
    mix_columns(block);

    //  Add Round Key - add_round_key
    add_round_key(block, round_key);
  }

  // Final Round
  create_round_key(expanded_key + 16 * ROUNDS_NUMBER, round_key);
  sub_bytes(block);
  shift_rows(block);
  add_round_key(block, round_key);

  // Transpose the block back to output
  int outputIndex = 0;
  for (int col = 0; col < 4; col++)
  {
    for (int row = 0; row < 4; row++)
    {
      int blockIndex = col + (row * 4);
      output[outputIndex] = block[blockIndex];
      outputIndex++;
    }
  }

  // de-allocate memory for expandedKey
  free(expanded_key);

  return output;
}

unsigned char *aes_decrypt_block(unsigned char *ciphertext,
                                 unsigned char *key)
{
  unsigned char *output =
      (unsigned char *)malloc(sizeof(unsigned char) * BLOCK_SIZE);

  // Block to hold the temporarily transposed matrix to encrypt
  unsigned char block[16];

  // Copy the plaintext into the block, transposing the order.
  int cipherTextIndex = 0;
  for (int col = 0; col < 4; col++)
  {
    for (int row = 0; row < 4; row++)
    {
      int blockIndex = col + (row * 4); // index to transpose to
      block[blockIndex] = ciphertext[cipherTextIndex];
      cipherTextIndex++; // Sequential index to copy from
    }
  }

  // Expand key
  unsigned char *expanded_key = expand_key(key);

  // Initial round
  unsigned char round_key[KEY_SIZE];
  create_round_key(expanded_key + (16 * ROUNDS_NUMBER), round_key);
  add_round_key(block, round_key);

  for (int roundIndex = ROUNDS_NUMBER - 1; roundIndex > 0; roundIndex--)
  {
    create_round_key(expanded_key + 16 * roundIndex, round_key);
    invert_shift_rows(block);
    invert_sub_bytes(block);
    add_round_key(block, round_key);
    invert_mix_columns(block);
  }

  // Final round
  create_round_key(expanded_key, round_key);
  invert_shift_rows(block);
  invert_sub_bytes(block);
  add_round_key(block, round_key);

  // Transpose the block back to output
  int outputIndex = 0;

  for (int col = 0; col < 4; col++)
  {
    // iterate over the rows
    for (int row = 0; row < 4; row++)
    {
      int blockIndex = col + (row * 4);
      output[outputIndex] = block[blockIndex];
      outputIndex++;
    }
  }

  // Free expanded key memory
  free(expanded_key);

  return output;
}
