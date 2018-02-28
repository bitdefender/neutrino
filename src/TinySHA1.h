/* 
 *
 * TinySHA1 - a header only implementation of the SHA1 algorithm in C++. Based
 * on the implementation in boost::uuid::details.
 * 
 * SHA1 Wikipedia Page: http://en.wikipedia.org/wiki/SHA-1
 * 
 * Copyright (c) 2012-22 SAURAV MOHAPATRA <mohaps@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _TINY_SHA1_HPP_
#define _TINY_SHA1_HPP_
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdint.h>

namespace sha1 {
	union Digest {
		uint32_t digest32[5];
		uint8_t digest8[20];

		Digest() {
			digest32[0] =
				digest32[1] =
				digest32[2] =
				digest32[3] =
				digest32[4] = 0;
		}

		Digest(const Digest &rhs) {
			digest32[0] = rhs.digest32[0];
			digest32[1] = rhs.digest32[1];
			digest32[2] = rhs.digest32[2];
			digest32[3] = rhs.digest32[3];
			digest32[4] = rhs.digest32[4];
		}

		Digest(Digest &&rhs) {
			digest32[0] = rhs.digest32[0];
			digest32[1] = rhs.digest32[1];
			digest32[2] = rhs.digest32[2];
			digest32[3] = rhs.digest32[3];
			digest32[4] = rhs.digest32[4];

			rhs.digest32[0] =
				rhs.digest32[1] =
				rhs.digest32[2] =
				rhs.digest32[3] =
				rhs.digest32[4] = 0;
		}

		Digest &operator=(const Digest &rhs) {
			digest32[0] = rhs.digest32[0];
			digest32[1] = rhs.digest32[1];
			digest32[2] = rhs.digest32[2];
			digest32[3] = rhs.digest32[3];
			digest32[4] = rhs.digest32[4];
		}

		Digest &operator=(Digest &&rhs) {
			digest32[0] = rhs.digest32[0];
			digest32[1] = rhs.digest32[1];
			digest32[2] = rhs.digest32[2];
			digest32[3] = rhs.digest32[3];
			digest32[4] = rhs.digest32[4];

			rhs.digest32[0] =
				rhs.digest32[1] =
				rhs.digest32[2] =
				rhs.digest32[3] =
				rhs.digest32[4] = 0;

			return *this;
		}

		bool operator==(const Digest &rhs) const {
			return
				(digest32[0] == rhs.digest32[0]) &&
				(digest32[1] == rhs.digest32[1]) &&
				(digest32[2] == rhs.digest32[2]) &&
				(digest32[3] == rhs.digest32[3]) &&
				(digest32[4] == rhs.digest32[4]);
		}
	};

	class SHA1 {
	public:
		inline static uint32_t LeftRotate(uint32_t value, size_t count) {
			return (value << count) | (value >> (32 - count));
		}

		template <int count> inline static uint32_t ROL(uint32_t value) {
			return (value << count) | (value >> (32 - count));
		}

		SHA1() { 
			Reset(); 
		}
		
		virtual ~SHA1() {}

		SHA1(const SHA1& s) { 
			*this = s; 
		}
		
		const SHA1& operator= (const SHA1& s) {
			memcpy(m_digest, s.m_digest, 5 * sizeof(uint32_t));
			memcpy(m_block, s.m_block, 64);
			m_blockByteIndex = s.m_blockByteIndex;
			m_byteCount = s.m_byteCount;
			return *this;
		}
		
		SHA1& Reset() {
			m_digest[0] = 0x67452301;
			m_digest[1] = 0xEFCDAB89;
			m_digest[2] = 0x98BADCFE;
			m_digest[3] = 0x10325476;
			m_digest[4] = 0xC3D2E1F0;
			m_blockByteIndex = 0;
			m_byteCount = 0;
			return *this;
		}
		
		SHA1& ProcessByte(uint8_t octet) {
			this->m_block[this->m_blockByteIndex++] = octet;
			++this->m_byteCount;
			if(m_blockByteIndex == 64) {
				this->m_blockByteIndex = 0;
				ProcessBlock();
			}
			return *this;
		}
		
		SHA1& ProcessBlock(const void* const start, const void* const end) {
			const uint8_t* begin = static_cast<const uint8_t*>(start);
			const uint8_t* finish = static_cast<const uint8_t*>(end);
			while(begin != finish) {
				ProcessByte(*begin);
				begin++;
			}
			return *this;
		}
		
		SHA1& ProcessBytes(const void* const data, size_t len) {
			const uint8_t* block = static_cast<const uint8_t*>(data);
			ProcessBlock(block, block + len);
			return *this;
		}
		
		void GetDigest(Digest &digest) {
			size_t bitCount = this->m_byteCount * 8;
			ProcessByte(0x80);
			if (this->m_blockByteIndex > 56) {
				while (m_blockByteIndex != 0) {
					ProcessByte(0);
				}
				while (m_blockByteIndex < 56) {
					ProcessByte(0);
				}
			} else {
				while (m_blockByteIndex < 56) {
					ProcessByte(0);
				}
			}
			ProcessByte(0);
			ProcessByte(0);
			ProcessByte(0);
			ProcessByte(0);
			ProcessByte( static_cast<unsigned char>((bitCount>>24) & 0xFF));
			ProcessByte( static_cast<unsigned char>((bitCount>>16) & 0xFF));
			ProcessByte( static_cast<unsigned char>((bitCount>>8 ) & 0xFF));
			ProcessByte( static_cast<unsigned char>((bitCount)     & 0xFF));
	
			memcpy(digest.digest32, m_digest, 5 * sizeof(uint32_t));
		}
		
	protected:
		void ProcessBlock() {
			uint32_t w[80];
			for (size_t i = 0; i < 16; i++) {
				w[i]  = (m_block[i*4 + 0] << 24);
				w[i] |= (m_block[i*4 + 1] << 16);
				w[i] |= (m_block[i*4 + 2] << 8);
				w[i] |= (m_block[i*4 + 3]);
			}
			for (size_t i = 16; i < 80; i++) {
				//w[i] = LeftRotate((w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16]), 1);
				w[i] = ROL<1>((w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16]));
			}
	
			uint32_t a = m_digest[0];
			uint32_t b = m_digest[1];
			uint32_t c = m_digest[2];
			uint32_t d = m_digest[3];
			uint32_t e = m_digest[4];
	
			for (std::size_t i = 0; i < 80; ++i) {
				uint32_t f = 0;
				uint32_t k = 0;
	
				if (i<20) {
					f = (b & c) | (~b & d);
					k = 0x5A827999;
				} else if (i<40) {
					f = b ^ c ^ d;
					k = 0x6ED9EBA1;
				} else if (i<60) {
					f = (b & c) | (b & d) | (c & d);
					k = 0x8F1BBCDC;
				} else {
					f = b ^ c ^ d;
					k = 0xCA62C1D6;
				}
				//uint32_t temp = LeftRotate(a, 5) + f + e + k + w[i];
				uint32_t temp = ROL<5>(a) + f + e + k + w[i];
				e = d;
				d = c;
				//c = LeftRotate(b, 30);
				c = ROL<30>(b);
				b = a;
				a = temp;
			}
	
			m_digest[0] += a;
			m_digest[1] += b;
			m_digest[2] += c;
			m_digest[3] += d;
			m_digest[4] += e;
		}
	private:
		uint32_t m_digest[5];
		uint8_t m_block[64];
		size_t m_blockByteIndex;
		size_t m_byteCount;
	};
}

#endif
