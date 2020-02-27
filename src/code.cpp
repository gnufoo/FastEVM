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

void FastEVM::create( name account, string identifier, string address )
{
    require_auth(account);

    check(identifier.size() > 0, "Please specify identifier string to generate your account.");
    check(address.size() == 42, "Please input correct ETH Address.");
    auto accidx = _account.get_index<"byacc"_n>();
    check(accidx.find(account.value) == accidx.end(), "Your account is already exist.");

    FastRLP rlpencode = FastRLP(2);
    rlpencode.addstring((uint8_t*)account.to_string().c_str(), account.to_string().size());
    rlpencode.addstring((uint8_t*)identifier.c_str(), identifier.size());
    vector<uint8_t> buff;
    rlpencode.data.encode(buff);
    Fast256 hash = _hash.keccak256(&buff[0], buff.size());
    print(" rlpencoded: "); printhex(&buff[0], buff.size()); print(" hashed: ", hash.tochecksum256());
    auto mask = Fast256((uint64_t)-1); mask.data[1] = (uint64_t)-1; mask.data[2] = 0xffffffff;
    hash = hash & mask ;

    std::transform(address.begin(), address.end(), address.begin(),[](unsigned char c){ return std::tolower(c); });

    _account.emplace( get_self(), [&](auto &row) {
        row.address = address;
        row.nonce = 1;
        row.balance = asset(0, symbol("EOS", 4));
        row.identifier = hash.tochecksum160();
        // row.fastsignature = hash;
        row.account = account;
    });
}

void FastEVM::raw( string transaction, name caller)
{
    require_auth(caller);
    uint64_t i = 2;
    uint8_t *trx_bytes = string_to_code(transaction, 2);
    FastRLP decode(trx_bytes, transaction.size()/2 - 1);
    check(decode.data.u.list.size() == 9, "unsupported ETH transaction data.");
    uint8_t *to     = decode.data.u.list[3].tobytes(20);
    uint8_t *data   = decode.data.u.list[5].tobytes(decode.data.u.list[5].u.list.size());
    uint8_t v       = decode.data.u.list[6].u.value;
    uint8_t *r      = decode.data.u.list[7].tobytes(32);
    uint8_t *s      = decode.data.u.list[8].tobytes(32);

    vector<content_type> sliced(&decode.data.u.list[0], &decode.data.u.list[6]);
    decode.data.u.list = sliced;
    if(v >= 35) {
        uint8_t recovery = ((v - 35) >> 1);
        decode.addstring(&recovery, 1);
        decode.addstring(0, 0);
        decode.addstring(0, 0);
    }

    vector<uint8_t> buff;
    int len = decode.data.encode(buff);

    Fast256 hash = _hash.keccak256(&buff[0], buff.size());

    auto publickey = recover(hash, r, s, v);
    // Fast256 fastsig(sha256(&std::get<0>(publickey)[0], 33));
    // auto sigidx = _account.get_index<"bysig"_n>();
    auto user = _account.find(publickey);
    // print("hello world1");

    check(user != _account.end(), "Not found avaiable signature.");
    check(user->account == caller, "Your don't have authorize to call this transaction.");

    // print("hello world2");
    Fast256 toAccount(to, 20);

    // print("hello world2"); printhex(to, 20);
    if(toAccount == Fast256::Zero())
    {
        /* create code from byte code here. */
    }
    else
    {
        print("toAccount: ", toAccount.tostring(24));
    }
    // print(" recovered publickey: ", fastsig.tochecksum256());
    /* serialize eth transaction code goes here. */
}

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
