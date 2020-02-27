#include <eosio/eosio.hpp>
#include <eosio/crypto.hpp>

using namespace eosio;

#ifndef __FAST_256__
#define __FAST_256__

class Fast256
{
public:
	Fast256() {
		reset();
	}
	Fast256(uint64_t v){
		data[0] = v;
		data[1] = data[2] = data[3] = 0;
	}

	Fast256(uint8_t v){
		data[0] = v;
		data[1] = data[2] = data[3] = 0;
	}

	Fast256(uint32_t v)
	{
		data[0] = v;
		data[1] = data[2] = data[3] = 0;
	}

	Fast256( checksum256 v )
	{
		fromchecksum256(v);
	}

	Fast256(const Fast256 &other)
	{
		memcpy(data, other.data, sizeof(uint64_t) * 4);
	}

	Fast256(uint8_t *src, uint64_t size)
	{
		reset();
		from(src, size);
	}

	static const Fast256 One()
	{
		static Fast256 one((uint8_t)1);
		return one;
	}

	static const Fast256 Zero()
	{
		static Fast256 zero;
		return zero;
	}

	void print()
	{
		eosio::print("0x"); printhex(&data[3],8); printhex(&data[2],8); printhex(&data[1],8); printhex(&data[0],8);
	}

	uint64_t fastid()
	{
		checksum256 hash;
		Fast256 myself(*this);

        hash = sha256((const char *)&myself, 32);
        auto arr = hash.extract_as_byte_array();

        // eosio::print("\n dump fastid: ", hash, "\n");
        // printhex(arr.data(), 32);
        // eosio::print("\n");
        // printhex((uint8_t *)&myself, 32);

		uint64_t serial = ((uint64_t)arr.data()[0] << 56) +
            ((uint64_t)arr.data()[4] << 48) +
            ((uint64_t)arr.data()[8] << 40) +
            ((uint64_t)arr.data()[12] << 32) +
            ((uint64_t)arr.data()[16] << 24) +
            ((uint64_t)arr.data()[20] << 16) +
            ((uint64_t)arr.data()[24] << 8) +
            (uint64_t)arr.data()[28];

        return serial;
	}

	checksum256 tochecksum256()
	{
		checksum256 ret;// = *(checksum256 *)data;

		memcpy( (char*)ret.data(), data + 2, 16);
		memcpy( (char*)ret.data() + 16, data, 16);
		return ret;
	}

	checksum160 tochecksum160()
	{
		checksum160 ret;
		memcpy( (char*)ret.data(), data, 20);
		return ret;
	}

	size_t tosize()
	{
		return data[0];
	}

	std::string tostring(int offset)
	{
		char finalhash[67];
		char hexval[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
		for(int i = 3; i >= 0; i --)
		for(int j = 0; j < 16; j ++)
			finalhash[2 + 16 * (3 - i) + j] = hexval[(data[i] >> (60 - j * 4)) & 0xf];;
		finalhash[66] = '\0';finalhash[offset] = '0'; finalhash[offset + 1] = 'x';
		return std::string(finalhash + offset);
	}

	Fast256 fromchecksum256( checksum256 src )
	{
		reset();
		auto arr = src.extract_as_byte_array();
		from(arr.data(), 32);
		return *this;
	}

	bool iszero() const
	{
		int index = 4;
		while(index --)
		{
			if(data[index] != 0)
				return false;
		}
		return true;
	}

	void swap(Fast256 &other)
	{
		std::swap(data, other.data);
	}

	void reset()
	{
		data[0] = data[1] = data[2] = data[3] = 0;
	}

	void from(uint8_t *bytes, int len)
	{
		int templen = len;
		while(1)
		{
			uint64_t *p = &data[(len - templen) / 8];
			uint8_t needBreak = 0;
			for(int i = 1; i <= 8; i ++)
			{
				needBreak = 0;
				if(templen<i) {
					needBreak = 1;
					break;
				}
				uint64_t lastP = *p;
				*p = ((uint64_t)(bytes[templen-i] & 0xff) << (8 * (i - 1))) | *p;
			}
			if(needBreak)
				break;

			templen -= 8;
		}
	}

	int compare(const Fast256 &other) const
	{
		for(int i = 0 ;i < 4; i ++)
		{
			if(data[i] == other.data[i])
				continue;
			if(data[i] > other.data[i])
				return 1;
			if(data[i] < other.data[i])
				return -1;
		}
		return 0;
	}

	bool operator ==(const Fast256 &other) const { return compare(other) == 0	; }
	bool operator !=(const Fast256 &other) const { return compare(other) != 0	; }
	bool operator < (const Fast256 &other) const { return compare(other) == -1  ; }
	bool operator <=(const Fast256 &other) const { return compare(other) != 1	; }
	bool operator >=(const Fast256 &other) const { return compare(other) != -1  ; }
	bool operator > (const Fast256 &other) const { return compare(other) == 1	; }

	Fast256 operator*(const Fast256 &oprand) const
	{
		Fast256 ret[4];

		for(int i = 0; i < 4; i ++)
		{
			uint128_t carry = 0;
			for(int j = 0; j < 4; j ++)
			{
				uint128_t temp = (data[i] * oprand.data[j]) + carry;
				carry = (temp >> 64) & 0xffffffffffffffff;
				ret[i].data[(j + i) > 3 ? 3 : (j+i)] = (temp & 0xffffffffffffffff);
			}
		}
		return ret[0] + ret[1] + ret[2] + ret[3];
	}

	Fast256 operator/(const Fast256 &divider) const
	{
		check( !divider.iszero(), "divide by zero");
		if(*this < divider)
			return Fast256::Zero();

		Fast256 ret;

		for(int i =3; i >= 0 ; i --)
		{
			if(!data[i]) continue;
			for(int j = i; j >=0; j --)
			{
				if(!divider.data[j]) continue;
				uint64_t reminder = 0;
				for(int k = i; k >=j; k --)
				{
					ret.data[k - j] = (data[k] + reminder) / divider.data[j];
					reminder = (data[k] + reminder) % divider.data[j];
				}
			}
		}

		return ret;
	}

	void swapendian()
	{
		uint64_t _t = data[0];
		data[0] = data[3];
		data[3] = _t;
		_t = data[1];
		data[1] = data[2];
		data[2] = _t;
	}

	void rappend64(uint64_t v)
	{
		data[3] = data[2];
		data[2] = data[1];
		data[1] = data[0];
		data[0] = v;
	}

	void operator=(const Fast256 &other)
	{
		memcpy(data, other.data, sizeof(uint64_t) * 4);
	}

	Fast256 operator+(const Fast256 &other)
	{
		uint8_t carry = 0;
		Fast256 ret;

		ret.data[0] = data[0] + other.data[0];
		carry = ret.data[0] < data[0] ? 1 : 0;
		ret.data[1] = data[1] + other.data[1] + carry;
		carry = ret.data[1] < data[1] ? 1 : 0;
		ret.data[2] = data[2] + other.data[2] + carry;
		carry = ret.data[2] < data[2] ? 1 : 0;
		ret.data[3] = data[3] + other.data[3] + carry;
		return ret;
	}

	Fast256 operator-(const Fast256 &other)
	{
		uint8_t lent = 0;
		Fast256 ret;

		ret.data[0] = data[0] - other.data[0];
		lent = ret.data[0] > data[0] ? 1 : 0;
		ret.data[1] = data[1] - other.data[1] - lent;
		lent = ret.data[1] > data[1] ? 1 : 0;
		ret.data[2] = data[2] - other.data[2] - lent;
		lent = ret.data[2] > data[2] ? 1 : 0;
		ret.data[3] = data[3] - other.data[3] - lent;
		return ret;
	}

	Fast256 operator&(const Fast256 &mask)
	{
		Fast256 ret;
		ret.data[0] = data[0] & mask.data[0];
		ret.data[1] = data[1] & mask.data[1];
		ret.data[2] = data[2] & mask.data[2];
		ret.data[3] = data[3] & mask.data[3];
		return ret;
	}

	Fast256 operator|(const Fast256 &mask)
	{
		Fast256 ret;
		ret.data[0] = data[0] | mask.data[0];
		ret.data[1] = data[1] | mask.data[1];
		ret.data[2] = data[2] | mask.data[2];
		ret.data[3] = data[3] | mask.data[3];
		return ret;
	}

	Fast256 operator^(const Fast256 &mask)
	{
		Fast256 ret;
		ret.data[0] = data[0] ^ mask.data[0];
		ret.data[1] = data[1] ^ mask.data[1];
		ret.data[2] = data[2] ^ mask.data[2];
		ret.data[3] = data[3] ^ mask.data[3];
		return ret;
	}

	Fast256 operator~()
	{
		Fast256 ret;
		ret.data[0] = ~data[0];
		ret.data[1] = ~data[1];
		ret.data[2] = ~data[2];
		ret.data[3] = ~data[3];
		return ret;
	}
	uint64_t data[4];
};

#endif