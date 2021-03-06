/*******************************************************************************
 * keygen.c
 *	WEP Key Generators
 *
 * This program generates WEP keys using de facto standard key
 * generators for 40 and 128 bit keys.
 *
 * Link against OpenSSL's libcrypto.a
 *
 * May 2001, Tim Newsham
 * June 2006, toast
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version. See http://www.fsf.org/copyleft/gpl.txt.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 ********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <openssl/md5.h>

#define WEPKEYSIZE              5
#define WEPSTRONGKEYSIZE        13
#define WEPKEYS                 4
#define WEPKEYSTORE             (WEPKEYSIZE * WEPKEYS)

//
// generate four subkeys from a seed using the defacto standard
//
void wep_seedkeygen(int val, u_char *keys) {
  int i;

  for(i = 0; i < WEPKEYSTORE; i++) {
    val *= 0x343fd;
    val += 0x269ec3;
    keys[i] = val >> 16;
  }
  return;
}

//
// generate one key from a string using the de facto standard
//
// resultant key is stored in
//   one 128 bit key: keys[0-15]
//
void wep_keygen104(const char *str, u_char *keys) {
  MD5_CTX ctx;
  u_char buf[64];
  int i, j;

  // repeat str until buf is full
  j = 0;
  for(i = 0; i < 64; i++) {
    if(str[j] == 0)
      j = 0;
    buf[i] = str[j++];
  }

  MD5_Init(&ctx);
  MD5_Update(&ctx, buf, sizeof buf);
  MD5_Final(buf, &ctx);

  for(i = 0; i < WEPSTRONGKEYSIZE; i++) {
    keys[i] = buf[i];
  }
  return;
}

//
// generate four subkeys from a string using the defacto standard
//
// resultant keys are stored in
//   four 40 bit keys: keys[0-4], keys[5-9], keys[10-14] and keys[15-20]
//
void wep_keygen40(const char *str, u_char *keys) {
  int val, i, shift;

  //
  // seed is generated by xor'ing in the keystring bytes
  // into the four bytes of the seed, starting at the little end
  ///
  val = 0;
  for(i = 0; str[i]; i++) {
    shift = i & 0x3;
    val ^= (str[i] << (shift * 8));
  }

  wep_seedkeygen(val, keys);
  return;
}

void wep_40keyprint(u_char *keys) {
  int i;
  char sepchar;

  for(i = 0; i < WEPKEYSTORE; i++) {
    if(i % WEPKEYSIZE == 0)
      printf("%d: ", i/WEPKEYSIZE);
    sepchar = (i % WEPKEYSIZE == WEPKEYSIZE - 1) ? '\n' : ':';
    printf("%02x%c", keys[i], sepchar);
  }
  return;
}

void wep_nkeyprint(u_char *key, u_int nbytes){
  int i;
  char sepchar;

  for(i = 0; i < nbytes; i++) {
    sepchar = (i == nbytes - 1) ? '\n' : ':';
    printf("%02x%c", key[i], sepchar);
  }
}

int main(int argc, char **argv){
  u_char key104[WEPSTRONGKEYSIZE];
  u_char key40[WEPKEYSTORE];
  if(argc != 2){
    printf("usage: %s <passphrase>\n", argv[0]);
    return 1;
  }
  wep_keygen40(argv[1], key40);
  wep_keygen104(argv[1], key104);

  printf("\n40-bit keys:\n");
  wep_40keyprint(key40);
  printf("\n104-bit key:\n");
  wep_nkeyprint(key104, WEPSTRONGKEYSIZE);
  printf("\n");

  if(strlen(argv[1]) == 5){
    printf("40-bit ASCII key:\n");
    wep_nkeyprint((u_char*)argv[1], 5);
    printf("\n");
  } else if(strlen(argv[1]) == 13){
    printf("104-bit ASCII key:\n");
    wep_nkeyprint((u_char*)argv[1], 13);
    printf("\n");
  }

  return 0;
}
