#include "stdafx.h"
#include "decrypt.h"
#include <iostream>
#include "generated.h"
#include "table.h"
#include "native_utils.h"

DynTable dyn_table_r;
int32_t decrypt_rotator = 0x33ABAEBE;
int32_t decrypt_offsets[] = {
	-257, -1, 255, 511, 767, 1023, 1279
};

void init_decryption(void) {
	dyn_table_r.generate(decrypt_rotator, decrypt_offsets);
}

uint16_t decrypt_p1(int64_t left, int64_t right) {
	uint16_t decrypted = right ^ ~left ^ 0xD25;
	for (auto i = 0; i < 5; i++) {
		auto idx1 = (decrypted ^ 0x4400u) >> 8;
		auto idx2 = (decrypted ^ 0x55) & 0xFF;
		decrypted = byte_table[byte_table[idx1]] | (byte_table[byte_table[idx2]] << 8);
	}
	return decrypted;
}

uint64_t decrypt_p20(int64_t left, int64_t right, uint64_t a, uint64_t b, uint64_t c, uint64_t d_idx) {
	auto p1 = decrypt_p1(left, right);
	auto idx2 = ubyte3(p1 ^ b);
	auto idx3 = ubyte1(p1) ^ c + 0x200;
	auto idx1 = ubyte0(p1 ^ a) + 0x300;
	auto idx4 = d_idx + 0x100; /* 2nd byte of p1 is discarded for static obfuscation key :( */

	return ~(dyn_table_r.get(idx1) ^ dyn_table_r.get(idx2) ^ dyn_table_r.get(idx3) ^ dyn_table_r.get(idx4)) % 0x2B;
}

uint64_t decrypt_p21(int64_t left, int64_t right) {
	return decrypt_p20(left, right, 0xBC, 0xD7AF5ABC, 0x5A, 0xAF);
}

uint64_t decrypt_p22(int64_t left, int64_t right) {
	return decrypt_p20(left, right, 0xC, 0x5CE7E30Cu, 0xE3, 0xE7);
}

uint64_t decrypt_ptr(EncryptedBlock *block) {
	auto a = block->data[decrypt_p21(block->data[44], block->data[45])];
	auto b = decrypt_p22(block->data[46], block->data[47]);
	return a ^ b;
}