[[eosio::action]]
void FastEVM::updatecode(string evmCode)
{
    require_auth( get_self() );

    auto existing = _code.find(1);
    if (existing == _code.end()) {
        _code.emplace( get_self() , [&](auto &row) {
            row.code = evmCode;
        });
    } else {
        _code.modify( existing,  get_self() , [&](auto &row){
            row.code = evmCode;
        });
    }
}

[[eosio::action]]
void FastEVM::raw( string transaction, name caller)
{
    uint64_t i = 2;
    uint8_t *trx_bytes = string_to_code(transaction, 2);
    FastRLP decode(trx_bytes, transaction.size()/2 - 1);
    check(decode.data.u.list.size() == 9, "unsupported ETH transaction data.");
    uint8_t v = decode.data.u.list[6].u.value;
    uint8_t *r = decode.data.u.list[7].tobytes(32);
    uint8_t *s = decode.data.u.list[8].tobytes(32);

    vector<content_type> sliced(&decode.data.u.list[0], &decode.data.u.list[6]);
    decode.data.u.list = sliced;
    if(v >= 35) {
        uint8_t recovery = ((v - 35) >> 1);
        decode.addstring(&recovery, 1);
        decode.addstring(0, 0);
        decode.addstring(0, 0);
    }

    // print(" ", decode, " ");

    vector<uint8_t> buff;
    int len = decode.data.encode(buff);
    // print(" len=", buff.size(), " ", len, " ");
    // printhex(&buff[0], buff.size());

    FastHash fh;
    Fast256 hash = fh.keccak256(&buff[0], buff.size());

    // printhex(&std::get<0>(sig1)[0], 65);
    // print(" keccak256: ", hash.tochecksum256());
    auto publickey = recover(hash, r, s, v);
    print(" recovered publickey: 0x");printhex(&std::get<0>(publickey), 33);
    /* serialize eth transaction code goes here. */
}

[[eosio::action]]
void FastEVM::createcode( string evmCode, name caller )
{
    require_auth( get_self() );
    Fast256 addr = execute_code( evmCode, evmCode, caller );

    string realcode = evmCode.substr( _copiedoffset * 2, _copiedcodesize * 2 );

    auto existing = _code.find(1);
    if (existing == _code.end()) {
        _code.emplace(_self, [&](auto & row) {
            row.code = realcode;
        });
    } else {
        _code.modify(existing, _self, [&](auto & row){
            row.code = realcode;
        });
    }
}
