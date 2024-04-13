#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "rijndael.h"

void print_128bit_block(unsigned char *block)
{
  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      unsigned char value = BLOCK_ACCESS(block, i, j);

      // Print spaces before small numbers to ensure that everything is aligned
      // and looks nice
      if (value < 10)
        printf(" ");

      if (value < 100)
        printf(" ");

      printf("%d", value);
    }
    printf("\n");
  }
}

int main()
{
  unsigned char plaintext[16] = "01234567ABCDEFGH"; // 16 character input string to encrypt
  unsigned char key[16] = {50, 20, 46, 86, 67, 9, 70, 27,
                           75, 17, 51, 17, 4,  8, 6,  99};

  for (int i = 0; i < 16; i++)
  {
    // Print characters in HEX format, 16 chars per line
    printf("%2.2x%c", plaintext[i], ((i + 1) % 16) ? ' ' : '\n');
  }

  unsigned char *ciphertext = aes_encrypt_block(plaintext, key);
  unsigned char *recovered_plaintext = aes_decrypt_block(ciphertext, key);

  printf("############ ORIGINAL PLAINTEXT ###########\n");
  print_128bit_block(plaintext);

  printf("\n\n################ CIPHERTEXT ###############\n");
  print_128bit_block(ciphertext);

  printf("\n\n########### RECOVERED PLAINTEXT ###########\n");
  print_128bit_block(recovered_plaintext);

  printf("\n\nTest plaintext recovery: \n");
  for(int charIndex = 0; charIndex < 16; charIndex++)
  {
    bool c = plaintext[charIndex] == recovered_plaintext[charIndex];
    printf("%c == %c: %s\n",
      plaintext[charIndex],
      recovered_plaintext[charIndex],
      c == true ? "true" : "false"
    );
  }

  free(ciphertext);
  free(recovered_plaintext);

  return 0;
}
