/* 
 * MD5 hash in C
 * 
 * Copyright (c) 2014 Project Nayuki
 * http://www.nayuki.io/page/fast-md5-hash-implementation-in-x86-assembly
 * 
 * (MIT License)
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * - The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 * - The Software is provided "as is", without warranty of any kind, express or
 *   implied, including but not limited to the warranties of merchantability,
 *   fitness for a particular purpose and noninfringement. In no event shall the
 *   authors or copyright holders be liable for any claim, damages or other
 *   liability, whether in an action of contract, tort or otherwise, arising from,
 *   out of or in connection with the Software or the use or other dealings in the
 *   Software.
 */

#ifndef MD5_H
#define	MD5_H

#include <stdint.h>

void md5_compress(uint32_t state[4], const uint32_t block[16]);

/* Full message hasher */
void md5_hash_Full(const char *message, uint32_t len, uint32_t hash[4]);

/* Incremental message hasher */
void md5_hash_Append(const char *message, uint32_t len, uint32_t hash[4]);


#endif	/* MD5_H */
