#include "fastsolidity.hpp"

/// @abi action
void FastEVM::updatecode(string evmCode)
{
    require_auth(_self);

    code theCode(_self, _self);
    auto existing = theCode.find(1);
    if(existing == theCode.end())
    {
        theCode.emplace(_self, [&](auto &s) {
            s.code = evmCode;
        });
    }
    else
    {
        theCode.modify(existing, _self, [&](auto &s){
            s.code = evmCode;
        });
    }
}

void FastEVM::createcode(string evmCode, account_name caller)
{
    require_auth(_self);
    _execute(evmCode, evmCode, caller);
}

void FastEVM::_execute(string codebase, string input, account_name caller)
{
    /* Initializ code segment. */
    uint64_t code_len = codebase.size();
    const char * code_str = codebase.c_str();
    _codelen = code_len / 2 - 1;
    _codebytes = new uint8_t[_codelen];

    _caller.from((uint8_t *)&caller, 8);
    //print(code_str);

    for(uint64_t i = 0; i < code_len; i += 2)
    {
        const char code0 = stringtobyte(code_str[i + 0]);
        const char code1 = stringtobyte(code_str[i + 1]);

        uint8_t ch = ((code0 << 4) | code1);
        _codebytes[i / 2] = ch;
    }
    
    /* Initialize input segment. */
    _inputlen = input.size() / 2 - 1;
    _inputbytes = new uint8_t[_inputlen];
    const char * input_str = input.c_str();

    for(uint64_t i = 2; i < input.size(); i += 2)
    {
        const char code0 = stringtobyte(input_str[i + 0]);
        const char code1 = stringtobyte(input_str[i + 1]);

        _inputbytes[i / 2 - 1] = ((code0 << 4) | code1);

    }

    print("backtrace: \n");

    /* Evaluating bytecodes. */
    uint8_t *op = _codebytes;
    const struct evmc_instruction_metrics *matrics = evmc_get_instruction_metrics_table(EVMC_BERLIN);
    while(op - _codebytes <= _codelen)
    {
        int pos = op - _codebytes;
        print(" [", pos, "]", codenames[*op], " ", (int)*op);//, " ", matrics[*op].stack_height_change);
        int changed = matrics[*op].stack_height_change;

        executeop(&op);
        op ++;

        _param = _spp;
        _spp -= changed;

        print(" [");
        for(Fast256 *it = (_spp + 1); it < _stack + 1024; it ++)
        {
             print(", ", *it);

        }
        print("] \n");
    }
}

void FastEVM::execute(string input, uint64_t caller)
{
    require_auth(_self);

    code theCode(_self, _self);
    auto existing = theCode.find(1);
    eosio_assert(existing != theCode.end(), "you should upload code to execute first.");

    _execute(existing->code, input, caller);
}

void FastEVM::executeop(uint8_t **opcode)
{
    switch(**opcode)
    {
        case OP_PUSH1 :
        {
            uint8_t *val =  ++ (*opcode);
            print(" ", "0x");
            printhex(val, 1);
            *_spp = *val;
            break;
        } 
        case OP_PUSH2 : 
        case OP_PUSH3 : 
        case OP_PUSH4 : 
        case OP_PUSH5 : 
        case OP_PUSH6 : 
        case OP_PUSH7 : 
        case OP_PUSH8 : 
        case OP_PUSH9 : 
        case OP_PUSH10 : 
        case OP_PUSH11 : 
        case OP_PUSH12 : 
        case OP_PUSH13 : 
        case OP_PUSH14 : 
        case OP_PUSH15 : 
        case OP_PUSH16 : 
        case OP_PUSH17 : 
        case OP_PUSH18 : 
        case OP_PUSH19 : 
        case OP_PUSH20 : 
        case OP_PUSH21 :
        case OP_PUSH22 :
        case OP_PUSH23 :
        case OP_PUSH24 :
        case OP_PUSH25 :
        case OP_PUSH26 :
        case OP_PUSH27 :
        case OP_PUSH28 :
        case OP_PUSH29 :
        case OP_PUSH30 :
        case OP_PUSH31 :
        case OP_PUSH32 :
        {
            int numBytes = (int)**opcode - (int)OP_PUSH1 + 1;
            uint64_t *val = (uint64_t *)(++(*opcode));
            uint8_t *val8 = (uint8_t *)(*opcode);
            int numQwords = (numBytes % 8 == 0) ? (numBytes / 8) : (numBytes / 8 + 1);
            print(" ", numBytes, "-", numQwords,":0x");
            printhex(val8, numBytes);
            (*opcode) += numBytes - 1;

            uint64_t temp = numBytes;
            _spp->reset();
            _spp->from(val8, numBytes);
            // for(int i = 0; i < numQwords; i ++)
            // {
            //     uint64_t offset = (8 - (temp % 8));
            //     uint64_t bits = temp % 8;
            //     uint64_t v = (val[i] & (uint64_t)(((uint64_t)0x1 << (temp * 8)) - 1));
            //     // uint64_t v = val[i] >> offset 
            //     // if(temp < 8)
            //     //      v = v >> ((8 - temp) * 8);
            //     print(" (", v, ", ", val[i], ", ", (uint64_t)(((uint64_t)0x1 << (temp * 8)) - 1), ", ", temp, ") ");
            //     _spp->rappend64(v);
            //     //_spp->data[i] = v;
            //     temp -= 8;
            // }
            break;
        }
        case OP_CODESIZE : {
            *(_spp) = _codelen;
            break;
        }
        case OP_MSTORE : 
        {
            Fast256 addr(*(_spp + 1));
            Fast256 val(*(_spp + 2));
            calculateMemory(addr.data[0]);
            (*_memory)[addr.data[0]] = val;
            //print(*_spp, " ", (*_memory)[addr.data[0]], " ", addr.data[0]);
            break;
        }
        case OP_CALLER : 
        {
            *(_spp) = _caller;
            break;
        }
        case OP_CALLDATASIZE :
        {
            *_spp = _inputlen;
            break;
        }
        case OP_ISZERO :
        {
            Fast256 operand(*(_spp + 1));
            *(_spp + 1) = operand.iszero() ? Fast256::One() : Fast256::Zero();
            break;
        }
        case OP_JUMPI :
        {
            Fast256 shouldJump(*(_spp + 2));
            if(shouldJump.iszero()) { break; }
            Fast256 source((uint32_t)(*opcode - _codebytes));
            // print(" dest: ", _param[0], " current: ", source);
            (*opcode) = validate(_codebytes + (_spp + 1)->data[0]);
            break;
        }
        case OP_JUMP : 
        {
            (*opcode) = validate(_codebytes + (_spp + 1)->data[0]);
            break;
        }
        case OP_CALLDATALOAD :
        {
            Fast256 offset(*(_spp + 1));
            if(offset.data[0] + 31 < _inputlen)
            {
                // *(_spp + 1) = *(Fast256*)(_inputbytes + (size_t)offset.data[0]);
                (_spp + 1)->from(_inputbytes + (size_t)offset.data[0], 32);
                (_spp + 1)->swapendian();
            } else if(offset.data[0] >= _inputlen) {
                (_spp + 1)->reset();
            } else {
                /* To be implemented. */
                // *(_spp + 1) = *(Fast256*)(_inputbytes + (size_t)offset.data[0]);
                // uint64_t valid = _inputlen - offset.data[0];
            }

            (_spp + 1)->swapendian();         
            break;
        }

        case OP_CODECOPY : 
        {
            print(" 0:", *(_spp), " 1:", *(_spp + 1), " 2:", *(_spp + 2))
            break;
        }
        case OP_DIV :
        {
            *(_spp + 2) = (_spp + 2)->iszero() ? Fast256::Zero() : *(_spp + 1) / *(_spp + 2);
            break;
        }
        case OP_CALLVALUE : 
        {
            *(_spp) = Fast256::Zero();
            break;
        }
        case OP_ADD : 
        {
            *(_spp + 2) = *(_spp + 1) + *(_spp + 2);
            break;
        }
        case OP_AND : 
        {
            break;
        }
        case OP_SUB : 
        {
            *(_spp + 2) = *(_spp + 1) - *(_spp + 2);
            break;
        }
        case OP_POP :
        {
            print(" pop:", *_spp);
            //_spp++; 
            break;
        }
        case OP_SHA3 : 
        {
            print(" sha3:", *(_spp + 1), " ", *(_spp + 2));
            print("\n mem:", (*_memory)[(_spp + 1)->data[0]]);
            checksum256 hash;
            sha256((char *)&((*_memory)[(_spp + 1)->data[0]]), 32, &hash);
            (_spp + 2)->from((uint8_t *)&hash, 32);
            break;
        }

        case OP_SWAP1 :
        case OP_SWAP2 :
        case OP_SWAP3 :
        case OP_SWAP4 :
        case OP_SWAP5 :
        case OP_SWAP6 :
        case OP_SWAP7 :
        case OP_SWAP8 :
        case OP_SWAP9 :
        case OP_SWAP10 :
        case OP_SWAP11 :
        case OP_SWAP12 :
        case OP_SWAP13 :
        case OP_SWAP14 :
        case OP_SWAP15 :
        case OP_SWAP16 :
        {
            int slot = (int)**opcode - (int)OP_SWAP1 + 1;
            (_spp + 1)->swap(*(_spp + slot + 1));
            break;
        }

        case OP_DUP1 :
        case OP_DUP2 :
        case OP_DUP3 :
        case OP_DUP4 :
        case OP_DUP5 :
        case OP_DUP6 :
        case OP_DUP7 :
        case OP_DUP8 :
        case OP_DUP9 :
        case OP_DUP10 :
        case OP_DUP11 :
        case OP_DUP12 :
        case OP_DUP13 :
        case OP_DUP14 :
        case OP_DUP15 :
        case OP_DUP16 :
        {

            int slot = (int)**opcode - (int)OP_DUP1 + 1;
            *(_spp) = *(_spp + slot);
            break;
        }

        case OP_EQ : 
        {
            //print(*(_spp + 1), *(_spp + 2));
            *(_spp + 2) = ((_spp + 1)->compare(*(_spp + 2)) == 0) ? Fast256::One() : Fast256::Zero();
            break;
        }

    }
}

#define EOSIO_ABI_EX(TYPE, MEMBERS)                                                                             \
    extern "C"                                                                                                  \
    {                                                                                                           \
        void apply(uint64_t receiver, uint64_t code, uint64_t action)                                           \
        {                                                                                                       \
            auto self = receiver;                                                                               \
            if (code == self || code == N(eosio.token) || code == N(fastecoadmin))                              \
            {                                                                                                   \
                TYPE thiscontract(self, code);                                                                  \
                if (action == N(transfer) && code == N(eosio.token))                                            \
                {                                                                                               \
                    /*eosio::execute_action( &thiscontract, &FTS10::xfernotice );*/                             \
                    return;                                                                                     \
                }                                                                                               \
                switch (action)                                                                                 \
                {                                                                                               \
                    EOSIO_API(TYPE, MEMBERS)                                                                    \
                }                                                                                               \
                /* does not allow destructor of thiscontract to run: eosio_exit(0); */                          \
            }                                                                                                   \
            else                                                                                                \
            {                                                                                                   \
                /*require_recipient(N(fastshieldio));*/                                                         \
            }                                                                                                   \
        }                                                                                                       \
    }

EOSIO_ABI_EX(FastEVM, (updatecode)(execute)(createcode))
