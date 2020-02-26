void FastEVM::calculate_memory( uint64_t newsize )
{
    uint64_t realnewsize = (newsize + 31) / 32 * 32;
    if (realnewsize > _memorysize) {
        print("fuck ", realnewsize);
        _memory->resize(realnewsize);
        _memorysize = realnewsize;
    }
}

uint8_t *FastEVM::string_to_code( string str, int offset )
{
    const char * code_str = str.c_str();
    size_t len = str.size();

    auto retlen = len / 2 - 1;
    uint8_t *ret = new uint8_t[retlen];

    for (uint64_t i = offset; i < len; i += 2)
    {
        const char code0 = string_to_byte(code_str[i + 0]);
        const char code1 = string_to_byte(code_str[i + 1]);

        uint8_t ch = ((code0 << 4) | code1);
        ret[(i - offset) / 2] = ch;
    }
    return ret;
}

uint8_t *FastEVM::validate( uint8_t *dest )
{
    uint64_t offset = dest - _codebytes;
    print("offset :", offset, " _codelen: ", _codelen);
    check(offset < _codelen, "invalid jumpi instruction.");
    check(*dest == OP_JUMPDEST, "invalid jump destination.");
    return dest;
}

eosio::public_key FastEVM::recover(Fast256 hash, uint8_t *r, uint8_t *s, uint8_t v)
{
    ecc_signature ecc;

    v = v < 2 ? v : 1 - v % 2;

    ecc[0] = (v - 1) + 27;
    memcpy(&ecc[1], r, 32);
    memcpy(&ecc[33], s, 32);

    signature sig;
    sig.emplace<0>(ecc);
    // print(" hash: ", hash.tochecksum256(), " "); printhex(r, 32); print(" "); printhex(s, 32); print(" ", v - 1);
    auto pubkey = recover_key(hash.tochecksum256(), sig);

    return pubkey;
}

const uint8_t FastEVM::string_to_byte( char c )
{
    const uint8_t char2hex[] = {
        ['0'] = 0x0, ['1'] = 0x1, ['2'] = 0x2, ['3'] = 0x3,
        ['4'] = 0x4, ['5'] = 0x5, ['6'] = 0x6, ['7'] = 0x7,
        ['8'] = 0x8, ['9'] = 0x9, ['a'] = 0xa, ['b'] = 0xb,
        ['c'] = 0xc, ['d'] = 0xd, ['e'] = 0xe, ['f'] = 0xf
    };
    return char2hex[c];
}