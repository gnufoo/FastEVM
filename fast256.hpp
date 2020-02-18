#ifndef __FAST_256__
#define __FAST_256__
#include <eosiolib/print.hpp>

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

	Fast256(checksum256 v)
	{
		fromchecksum256(v);
	}

	Fast256(const Fast256 &other)
	{
		memcpy(data, other.data, sizeof(uint64_t) * 4);
	}

	Fast256(uint8_t *src, uint64_t size)
	{
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
		checksum256 hash, hash1;
		Fast256 myself(*this);
        
        sha256((const char *)&myself, 32, &hash);
        // eosio::print("\n========fast id dump start==============\n");
        // printhex((uint8_t *)&myself, 32);
        // eosio::print(" ");
        // printhex((uint8_t *)&hash, 32);

        // uint64_t num = 0;

        // for(int i = 0;i < 8; i ++)
        // {
        // 	int operand = 56 - i * 8;
        // 	uint64_t val = (uint64_t)hash.hash[i] << operand;
        // 	num += val;
        // 	eosio::print("\n ", i, " ", (uint64_t)hash.hash[i], " ((uint64_t)hash.hash[i] << ", operand, " ");
        // 	printhex(&val, 8); eosio::print(" "); printhex(&num, 8);
        // }

		uint64_t serial = ((uint64_t)hash.hash[0] << 56) + 
            ((uint64_t)hash.hash[1] << 48) + 
            ((uint64_t)hash.hash[2] << 40) + 
            ((uint64_t)hash.hash[3] << 32) + 
            ((uint64_t)hash.hash[4] << 24) + 
            ((uint64_t)hash.hash[5] << 16) + 
            ((uint64_t)hash.hash[6] << 8) + 
            (uint64_t)hash.hash[7];	
        //serial /= 1000;

        // eosio::print(" ", serial, " ");
        // printhex((uint8_t *)&serial, 8);
        // eosio::print("\n========fast id dump end==============\n");
        return serial;
	}

	checksum256 tochecksum256()
	{
		checksum256 ret = *(checksum256 *)data;
		int i = 32;
		uint8_t swap;
		while( -- i >= 16)
		{
			swap = ret.hash[i];
			ret.hash[i] = ret.hash[31 - i];
			ret.hash[31 - i] = swap;
		}
		return ret;
	}

	Fast256 fromchecksum256(checksum256 src)
	{
		checksum256 dest = src;
		int i = 32;
		uint8_t swap;
		while( -- i >= 16)
		{
			swap = dest.hash[i];
			dest.hash[i] = dest.hash[31 - i];
			dest.hash[31 - i] = swap;
		}
		memcpy(data, &dest, 32);
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

	Fast256 operator/(const Fast256 &divider) const 
	{
		eosio_assert(!divider.iszero(), "divide by zero.");
		if(*this < divider)
			return Fast256::Zero();

		if(data[3] && divider.data[3])
		{
			eosio::print(" mine:", data[3], " other:", divider.data[3]);
			return Fast256(data[3] / divider.data[3]);
		}

		return Fast256::One();
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