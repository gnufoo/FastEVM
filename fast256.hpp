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

	Fast256(const Fast256 &other)
	{
		memcpy(data, other.data, sizeof(uint64_t) * 4);
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
		eosio::print("0x");
		// printhex(&data[3],8);
		// printhex(&data[2],8);
		// printhex(&data[1],8);
		// printhex(&data[0],8);
		printhex(&data[3],8);
		printhex(&data[2],8);
		printhex(&data[1],8);
		printhex(&data[0],8);
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
				// eosio::print("\n len=", len, " *p=", *p, " templen=", templen, " i=", i, " (bytes[templen-i] & 0xff)=", (bytes[templen-i] & 0xff), " (8 * (i - 1))=", (8 * (i - 1)), " lastP=", lastP, "\n");
				// printhex(p, templen);
				// eosio::print("\n");
				// print();
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

	uint64_t data[4];
};

#endif