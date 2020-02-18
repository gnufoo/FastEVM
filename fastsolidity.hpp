#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <string>
#include <eosiolib/transaction.hpp>
#include <eosiolib/crypto.h>
#include <cmath>
#include "evmc.hpp"
#include "fast256.hpp"

using std::string;
using namespace eosio;

class FastEVM : public contract 
{
public:
    FastEVM(account_name self, account_name code) :contract(self), _code(code) {
        _stack = new Fast256[1024];
        memset(_stack, 0, 1024 * sizeof(Fast256));
        _spp = &_stack[1023];
        _memory = new vector<Fast256>();
        _memory->reserve(256);
        _memorysize = 256;
    }

    /// @abi action
    // for the first purpose testing envorinment.
    void updatecode(string code);

    /// @abi action
    void execute(string input, account_name caller);

    /// @abi action
    void createcode(string code, account_name caller);

private:
    account_name _code;

    Fast256 _execute(string code, string input, account_name caller);

    Fast256 *_spp;
    Fast256 *_param;
    Fast256 *_stack;
    vector<Fast256> *_memory;
    uint64_t _memorysize;
    uint8_t *_inputbytes;
    uint64_t _inputlen;
    uint64_t _codelen;
    uint64_t _copiedcodesize;
    uint64_t _copiedoffset;
    uint8_t *_codebytes;

    Fast256 _caller;
    /// @abi table code i64
    struct code_t {
        string code;

        uint64_t primary_key()const { return 1; }
        EOSLIB_SERIALIZE( code_t, (code) )
    };

    /// @abi table state i64
    struct states_t {
        // uint64_t fastid;
        checksum256 key;
        checksum256 value;

        uint64_t primary_key()const { return Fast256(key).fastid(); }
        // checksum256 byhash()const { return Fast256(key).fastid(); }
        // EOSLIB_SERIALIZE( states_t, (fastid)(key)(value) )
        EOSLIB_SERIALIZE( states_t, (key)(value) )
    };

    // /// @abi table account i64
    // struct states_t {
    //     uint64_t fastid;
    //     checksum256 key;
    //     checksum256 value;

    //     uint64_t primary_key()const { return Fast256(key).fastid(); }
    //     // checksum256 byhash()const { return Fast256(key).fastid(); }
    //     EOSLIB_SERIALIZE( states_t, (fastid)(key)(value) )
    // };    

    typedef eosio::multi_index<N(code), code_t>     code;
    typedef eosio::multi_index<N(state), states_t>  states;
    // typedef   multi_index<N(records), st_record, indexed_by<N(byissue), const_mem_fun<st_record, uint64_t, &st_record::issue_key>>> records;
    // typedef eosio::multi_index<N(state), states_t, indexed_by<N(secondkey), const_mem_fun<states_t, checksum256, &states_t::byhash>>> states;

    void setstate(Fast256 addr, Fast256 content)
    {
        print("\nset stat: ", addr, + " ", addr.fastid());
        states state(_self, _self);
        auto ite = state.find(addr.fastid());
        if(ite != state.end())
        {
            state.modify(ite, _self, [&](auto &s){
                s.value = content.tochecksum256();
            });
        }
        else
        {
            state.emplace(_self, [&](auto &s){
                // s.fastid = addr.fastid();
                s.key = addr.tochecksum256();
                s.value = content.tochecksum256();
            });
        }
    }

    Fast256 getstate(Fast256 addr)
    {
        states state(_self, _self);
        auto ite = state.find(addr.fastid());
        if(ite == state.end())
            return Fast256::Zero();

        return Fast256(ite->value);
    }

    bool executeop(uint8_t **opcode);
    void calculateMemory(uint64_t newsize)
    {
        uint64_t realnewsize = (newsize + 31) / 32 * 32;
        if(realnewsize > _memorysize)
        {
            print("fuck ", realnewsize);
            _memory->resize(realnewsize);
            _memorysize = realnewsize;
        }
    }

    uint8_t *validate(uint8_t *dest)
    {
        uint64_t offset = dest - _codebytes;
        print("offset :", offset, " _codelen: ", _codelen);
        eosio_assert(offset < _codelen, "invalid jumpi instruction.");
        /* Need to check if it is actually landed on the destination, could be just easily the see if the destination is JUMPDEST or not. */
        return dest;
    }

    const uint8_t stringtobyte(char c)
    {
        const uint8_t char2hex[] = {
            ['0'] = 0x0, ['1'] = 0x1, ['2'] = 0x2, ['3'] = 0x3,
            ['4'] = 0x4, ['5'] = 0x5, ['6'] = 0x6, ['7'] = 0x7,
            ['8'] = 0x8, ['9'] = 0x9, ['a'] = 0xa, ['b'] = 0xb,
            ['c'] = 0xc, ['d'] = 0xd, ['e'] = 0xe, ['f'] = 0xf
        };
      return char2hex[c];
    }
};



