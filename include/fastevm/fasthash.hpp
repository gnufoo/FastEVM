#ifndef __FAST_HASH__
#define __FAST_HASH__

#include "sha3.hpp"

class FastHash
{
public:
	Fast256 keccak256(uint8_t *byte, size_t len)
	{
		sha3_Init256(&context);

		sha3_Update(&context, byte, len);
		uint8_t *result = (uint8_t *)sha3_Finalize(&context);
		Fast256 ret(result, 32);
		return ret;
	}

private:
	sha3_context context;
	bool init;
};

#endif