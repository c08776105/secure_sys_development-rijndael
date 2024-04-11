/*
 * TODO: Add your name and student number here, along with
 *       a brief description of this code.
 *
 * TODO: add getRconValue function
 * TODO: add getSBoxValue function
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
// TODO: Any other files you need to include should go here

#include "rijndael.h"

#define KEY_SIZE 16
#define DEBUG_ACTIVE 1
#define VECTOR_SIZE_BYTES 176
#define ROUNDS_NUMBER 10

unsigned char round_constant[] = {
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36};

unsigned char SBox[] = {
    /* S-box */
};

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

unsigned char galois_multiplication(unsigned char a, unsigned char b)
{
  unsigned char p = 0;
  unsigned char counter;
  unsigned char hi_bit_set;
  for (counter = 0; counter < 8; counter++)
  {
    if ((b & 1) == 1)
      p ^= a;
    hi_bit_set = (a & 0x80);
    a <<= 1;
    if (hi_bit_set == 0x80)
      a ^= 0x1b;
    b >>= 1;
  }
  return p;
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
  // TODO: Test me!
  for (int i = 0; i < 16; i++)
  {
    block[i] = SBox[block[i]];
  }
}

/**
 * The bytes in each row of the block are shifted cyclically to the left.
 * The first row is not shifted, the second row is shifted by one byte to the left,
 * the third row by two bytes, and the fourth row by three bytes.
 */
void shift_rows(unsigned char *block)
{
  // TODO: Test me!
  // iterare over each of the four rows
  for (int i = 0; i < 4; i++)
  {
    unsigned char tmp;
    // each iteration shifts the row to the left by 1
    for (int j = 0; j < i; j++)
    {
      tmp = block[0];
      for (int k = 0; k < 3; k++)
      {
        block[k] = block[k + 1];
      }
      block[3] = tmp;
    }
  }
}

/**
 * Each column of the block is transformed using a matrix multiplication operation.
 */
void mix_columns(unsigned char *block)
{
  // TODO: Implement me!
  unsigned char column[4];

  // iterate over the 4 columns
  for (int i = 0; i < 4; i++)
  {
    // construct one column by iterating over the 4 rows
    for (int j = 0; j < 4; j++)
    {
      column[j] = block[(j * 4) + i];
    }

    // apply the mixColumn on one column

    unsigned char cpy[4];
    for (int k = 0; k < 4; k++)
    {
      cpy[k] = column[k];
    }
    column[0] = galois_multiplication(cpy[0], 2) ^
                galois_multiplication(cpy[3], 1) ^
                galois_multiplication(cpy[2], 1) ^
                galois_multiplication(cpy[1], 3);

    column[1] = galois_multiplication(cpy[1], 2) ^
                galois_multiplication(cpy[0], 1) ^
                galois_multiplication(cpy[3], 1) ^
                galois_multiplication(cpy[2], 3);

    column[2] = galois_multiplication(cpy[2], 2) ^
                galois_multiplication(cpy[1], 1) ^
                galois_multiplication(cpy[0], 1) ^
                galois_multiplication(cpy[3], 3);

    column[3] = galois_multiplication(cpy[3], 2) ^
                galois_multiplication(cpy[2], 1) ^
                galois_multiplication(cpy[1], 1) ^
                galois_multiplication(cpy[0], 3);

    // put the values back into the state
    for (int j = 0; j < 4; j++)
    {
      block[(j * 4) + i] = column[j];
    }
  }
}

/*
 * Operations used when decrypting a block
 */
void invert_sub_bytes(unsigned char *block)
{
  // TODO: Implement me!
}

void invert_shift_rows(unsigned char *block)
{
  // TODO: Implement me!
}

void invert_mix_columns(unsigned char *block)
{
  // TODO: Implement me!
}

/*
 * This operation is shared between encryption and decryption
 *
 * The block is XORed with the current round key.
 */
void add_round_key(unsigned char *block, unsigned char *round_key)
{
  // TODO: Test me!
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

  while (bytes_generated < VECTOR_SIZE_BYTES)
  {
    // Copy the previous 4 bytes into expansion_block
    memcpy(expansion_block, &expanded_key[bytes_generated - 4], 4);

    // Core operation
    if (bytes_generated % BLOCK_SIZE == 0)
    {
      // Create a pointer of the 32bit word
      unsigned int *word_ptr = (unsigned int *)expansion_block;

      // perform left rotation of the word
      *word_ptr = (*word_ptr >> 8) | ((*word_ptr & 0xFF) << 24);

      // Perform sbox substitution on all 4 parts of the word
      expansion_block[0] = SBox[expansion_block[0]];
      expansion_block[1] = SBox[expansion_block[1]];
      expansion_block[2] = SBox[expansion_block[2]];
      expansion_block[3] = SBox[expansion_block[3]];

      // XOR the output of the rcon operation
      expansion_block[0] ^= round_constant[rcon_iteration++];
    }

    // XOR temp with the 4-byte block N bytes before the new expanded key.
    // This becomes the next 4 bytes in the expanded key.
    for (unsigned char i = 0; i < 4; ++i)
    {
      expanded_key[bytes_generated] = expanded_key[bytes_generated - BLOCK_SIZE] ^ expansion_block[i];
      bytes_generated++;
    }
  }

  return expanded_key;
}

// TODO: document
void create_round_key(unsigned char *expanded_key, unsigned char *round_key)
{
  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      round_key[(i + (j * 4))] = expanded_key[(i * 4) + j];
    }
  }
}

/*
 * The implementations of the functions declared in the
 * header file should go here
 */
unsigned char *aes_encrypt_block(unsigned char *plaintext, unsigned char *key)
{
  // TODO: Implement me!
  unsigned char *output =
      (unsigned char *)malloc(sizeof(unsigned char) * BLOCK_SIZE);

  memcpy(output, plaintext, BLOCK_SIZE);

  // Expand key
  unsigned char *expanded_key = expand_key(key);

  if (DEBUG_ACTIVE == 1)
  {
    pprint("Expanded Key", expanded_key);
  }

  pprint("Input", plaintext);

  // Initial round
  unsigned char round_key[KEY_SIZE];
  create_round_key(expanded_key, round_key);
  add_round_key(output, expanded_key);

  pprint("After Initial Round", output);

  // Rounds (start i at 1 because initial round is already complete)
  for (int i = 1; i < ROUNDS_NUMBER; i++)
  {
    create_round_key(expanded_key + 16 * i, round_key);

    //  Sub Bytes - sub_bytes
    sub_bytes(output);

    //  Shift Rows - shift_rows
    shift_rows(output);

    //  Mix Columns - mix_columns
    mix_columns(output);

    //  Add Round Key - add_round_key
    add_round_key(output, round_key);

  char buf[20];
  snprintf(buf, 20, "Round iteration %d", i);
  pprint(buf, output);
  }

  // Final Round
  create_round_key(expanded_key + 16 * ROUNDS_NUMBER, round_key);
  sub_bytes(output);
  shift_rows(output);
  add_round_key(output, round_key);

  return output;
}

unsigned char *aes_decrypt_block(unsigned char *ciphertext,
                                 unsigned char *key)
{
  // TODO: Implement me!
  unsigned char *output =
      (unsigned char *)malloc(sizeof(unsigned char) * BLOCK_SIZE);
  return output;
}
