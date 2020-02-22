[[eosio::action]]
void FastEVM::execute( string input, name caller )
{
    require_auth( caller );

    auto existing = _code.find(1);
    check( existing != _code.end(), "you should upload code to execute first.");

    execute_code(existing->code, input, caller);
}

Fast256 FastEVM::execute_code( string codebase, string input, name caller )
{
    /* Initializ code segment. */
    uint64_t code_len = codebase.size();
    const char * code_str = codebase.c_str();
    _codelen = code_len / 2 - 1;
    _codebytes = string_to_code(codebase);

    _caller.from((uint8_t *)&caller.value, 8);

    for (uint64_t i = 0; i < code_len; i += 2)
    {
        const char code0 = string_to_byte(code_str[i + 0]);
        const char code1 = string_to_byte(code_str[i + 1]);

        uint8_t ch = ((code0 << 4) | code1);
        _codebytes[i / 2] = ch;
    }

    /* Initialize input segment. */
    _inputlen = input.size() / 2 - 1;
    _inputbytes = new uint8_t[_inputlen];
    const char * input_str = input.c_str();

    for(uint64_t i = 2; i < input.size(); i += 2)
    {
        const char code0 = string_to_byte(input_str[i + 0]);
        const char code1 = string_to_byte(input_str[i + 1]);

        _inputbytes[i / 2 - 1] = ((code0 << 4) | code1);

    }

    print("backtrace: \n");

    /* Evaluating bytecodes. */
    uint8_t *op = _codebytes;
    const struct evmc_instruction_metrics *matrics = evmc_get_instruction_metrics_table(EVMC_BERLIN);
    while (op - _codebytes <= _codelen)
    {
        int pos = op - _codebytes;
        print(" [", pos, "]", codenames[*op]);//, " ", matrics[*op].stack_height_change);
        int changed = matrics[*op].stack_height_change;

        bool returned = execute_op(&op);
        if (returned)
        {
            //print("\n return: ", *(_spp), *(_spp + 1));
            return *(_spp + 1);
        }
        op ++;

        _param = _spp;
        _spp -= changed;

        print(" [");
        for (Fast256 *it = (_spp + 1); it < _stack + 1024; it ++)
        {
            if (it == (_spp + 1)) print(*it);
            else print(", ", *it);

        }
        print("] \n");
    }
    return Fast256::Zero();
}

bool FastEVM::execute_op(uint8_t **opcode)
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
            calculate_memory(addr.data[0]);
            (*_memory)[addr.data[0]] = val;
            break;
        }
        case OP_MLOAD :
        {
            print("\n mload(", (_spp + 1)->data[0], ": ");
            printhex(&(*_memory)[(_spp + 1)->data[0]], 32);
            *(_spp + 1) = (*_memory)[(_spp + 1)->data[0]];
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
        case OP_LT :
        {
            *(_spp + 2) = *(_spp + 1) < *(_spp + 2) ? Fast256::One() : Fast256::Zero();
            break;
        }
        case OP_GT :
        {
            *(_spp + 2) = *(_spp + 1) > *(_spp + 2) ? Fast256::One() : Fast256::Zero();
            break;
        }
        case OP_EQ :
        {
            *(_spp + 2) = *(_spp + 1) == *(_spp + 2) ? Fast256::One() : Fast256::Zero();
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
            (_spp + 1)->reset();
            if(offset.data[0] + 31 < _inputlen)
            {
                // *(_spp + 1) = *(Fast256*)(_inputbytes + (size_t)offset.data[0]);
                (_spp + 1)->from(_inputbytes + (size_t)offset.data[0], 32);
                (_spp + 1)->swapendian();
            } else if(offset.data[0] >= _inputlen) {
                (_spp + 1)->reset();
            } else {
                print("\nshould not be here.\n");
                /* To be implemented. */
                // *(_spp + 1) = *(Fast256*)(_inputbytes + (size_t)offset.data[0]);
                // uint64_t valid = _inputlen - offset.data[0];
            }

            (_spp + 1)->swapendian();
            break;
        }

        case OP_CODECOPY :
        {
            uint8_t *dest = (uint8_t *)&(*_memory)[(_spp + 1)->data[0]];
            uint8_t *codesrc = _codebytes + (_spp + 2)->data[0];
            uint64_t len = (_spp + 3)->data[0];
            memcpy(dest, codesrc, len);
            print("\n 0:", *(_spp), "\n 1:", *(_spp + 1), "\n 2:", *(_spp + 2), "\n 3:", *(_spp + 3), "\n c:");
            printhex(&(*_memory)[(_spp + 1)->data[0]], (_spp + 3)->data[0]);
            // swap endian to makesure all the data in memory is little endian
            for(auto i = 0; i < len / 32; i ++)
            {
                Fast256 *p = (Fast256 *)(dest + i * 32);
                p->swapendian();
            }
            _copiedcodesize = len;
            _copiedoffset = (_spp + 2)->data[0];
            print("\n swap:");
            printhex(&(*_memory)[(_spp + 1)->data[0]], (_spp + 3)->data[0]);
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
        case OP_EXP :
        {
            if((_spp + 2)->iszero())
                *(_spp + 2) = Fast256::One();
            /* */
            print("\n exp:", *(_spp + 1), " ", *(_spp + 2));
            break;
        }
        case OP_SSTORE :
        {
            print("\n store: ", *(_spp + 1), " ", *(_spp + 2));
            set_state(*(_spp + 1), *(_spp + 2));
            break;
        }
        case OP_SLOAD :
        {
            *(_spp + 1) = get_state(*(_spp + 1));
            break;
        }
        case OP_MUL :
        {
            *(_spp + 2) = *(_spp + 1) * *(_spp + 2);
            break;
        }
        case OP_AND :
        {
            *(_spp + 2) = *(_spp + 1) & *(_spp + 2);
            break;
        }
        case OP_OR :
        {
            *(_spp + 2) = *(_spp + 1) | *(_spp + 2);
            break;
        }
        case OP_NOT :
        {
            *(_spp + 1) = ~*(_spp + 1);
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
            checksum256 hash;
            print("seed: ", (*_memory)[(_spp + 1)->data[0]], "\n");
            printhex(&(*_memory)[(_spp + 1)->data[0]], 32);
            hash = sha256((char *)&((*_memory)[(_spp + 1)->data[0]]), 32);
            print("\n");
            printhex(&hash, 32);
            (_spp + 2)->fromchecksum256(hash);
            print("\n");
            printhex((_spp + 2), 32);
            break;
        }

        case OP_LOG0:
        case OP_LOG1:
        case OP_LOG2:
        case OP_LOG3:
        case OP_LOG4:
        {
            /* Not Implemented Yet */
            break;
        }

        case OP_JUMPDEST :
        {
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
        case OP_RETURN :
        {
            return true;
        }
        default :
        {
            string err = "missing implementation on ";
            err += codenames[(int)**opcode];
            check(0, err.c_str());
        }
    }
    return false;
}
