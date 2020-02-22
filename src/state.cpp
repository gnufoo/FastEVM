void FastEVM::setstate( Fast256 addr, Fast256 content )
{
    // print("\nset stat: ", addr, + " ", addr.fastid(), _self, _self.value);
    auto ite = _state.find(addr.fastid());
    if (ite != _state.end()) {
        _state.modify(ite, _self, [&](auto &s){
            s.value = content.tochecksum256();
        });
    } else {
        _state.emplace(_self, [&](auto &s){
            // s.fastid = addr.fastid();
            s.key = addr.tochecksum256();
            s.value = content.tochecksum256();
        });
    }
}

Fast256 FastEVM::getstate( Fast256 addr )
{
    auto ite = _state.find( addr.fastid() );
    if (ite == _state.end()) return Fast256::Zero();
    return Fast256(ite->value);
}