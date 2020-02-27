#ifndef __Fast_RLP__
#define __Fast_RLP__

#include <eosio/eosio.hpp>
#include <eosio/crypto.hpp>

using namespace eosio;
using namespace std;

struct content_type
{
	content_type() {}
	content_type(uint8_t t, uint8_t v) : type(t), u(v) {}
	content_type(uint8_t t) : type(t), u(0) {}
	content_type(const content_type &other) : u(other.u)
	{
		type = other.type;
	}

	uint8_t type;
	struct value_t {
		value_t() {}
		value_t(uint8_t v) : value(v) {}
		value_t(const value_t &other)
		{
			value = other.value;
			list = other.list;
		}
		uint8_t value;
		vector<content_type> list;
	} u;

	void print()
	{
		if ( type == 0 ) {
			printhex(&u.value, 1);
		} else {
			eosio::print("[");
			for (auto ite = u.list.begin(); ite != u.list.end(); ite++)
			{
				if(ite != u.list.begin() && type == 2)
					eosio::print(",");
				ite->print();
			}
			eosio::print("]");
		}
	};

	uint8_t *tobytes(int len)
	{
		check(u.list.size() >= len, "can't to bytes for this element.");
		uint8_t *ret = new uint8_t[len];

		for (auto i = 0; i < len; i ++) {
			ret[i] = u.list[i].u.value;
		}

		return ret;
	}

	size_t encode(vector<uint8_t>& buff)
	{
		int ret = 0;
		vector<uint8_t> temp;
		if (type == 0) {
			temp.push_back(u.value);
			buff.insert(buff.end(), temp.begin(), temp.end());
			return 1;
		} else {
			for(auto ite = u.list.begin(); ite != u.list.end(); ite++)
				ret += ite->encode(temp);

			if (ret > 0x38) {
				buff.push_back(type == 2 ? 0xf8 : 0xb8);
				buff.push_back(ret);
				ret += 2;
			} else {
				buff.push_back(type == 2 ? (0xc0 + ret) : (0x80 + ret));
				ret ++;
			}

			buff.insert(buff.end(), temp.begin(), temp.end());
			return ret;
		}
	}
};

class FastRLP
{
public:
	FastRLP(uint8_t* bytes, int len)
	{
		uint8_t *p = bytes;
	    data = parse(&p);
	}
	FastRLP(uint8_t type) : data(type) {}

	void addstring(uint8_t *content, size_t len)
	{
		content_type item(len == 1 ? 0 : 1);
		if(len == 1)
			item.u.value = content[0];
		else
		{
			for(auto i = 0; i < len; i ++)
			{
				item.u.list.push_back(content_type(0, content[i]));
			}	
		}
		
		data.u.list.push_back(item);
	}

	content_type parse(uint8_t **p)
	{
		if((**p) <= 0x7f)
        {
        	content_type ret(0, (**p)); (*p)++;
            return ret;
        }
        else if((**p) <= 0xb7)
        {
            uint8_t len = (**p) - 0x80;
            content_type ret(1);
            for (auto i = 0; i < len; i ++) {
            	ret.u.list.push_back(content_type(0, (*p)[i + 1]));
			}
            (*p) += len + 1;
            return ret;
        }
        else if((**p) <= 0xbf)
        {
            uint8_t len_of_str_len = (**p) - 0xb7;
            Fast256 strlen(*(p) + 1, len_of_str_len);
            content_type ret(1);
            for (auto i = 0; i < strlen.tosize(); i ++) {
            	ret.u.list.push_back(content_type(0, (*p)[i + len_of_str_len + 1]));
			}
            (*p) += len_of_str_len + strlen.data[0] + 1;
            return ret;
        }

        else if((**p) <= 0xf7)
        {
            uint8_t list_len = (**p) - 0xc0;
            (*p) += list_len + 1;

            content_type ret(2);
            uint8_t *list_p = *p;
            while (list_p < *p + list_len) {
            	ret.u.list.push_back(parse(&list_p));
			}
            return ret;
        }
        else if((**p) <= 0xff)
        {
        	uint8_t len_of_list_len = (**p) - 0xf7;
        	Fast256 listlen((*p) + 1, len_of_list_len);
            (*p) += len_of_list_len + 1;

            content_type ret(2);
            uint8_t *list_p = *p;
            while (list_p < *p + listlen.tosize()) {
            	ret.u.list.push_back(parse(&list_p));
            }
            return ret;
        }
        check(0, "FastRLP parse error.");
        return content_type(0);
	}

	void print()
	{
		data.print();
	}

	content_type data;
};

#endif