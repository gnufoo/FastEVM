/**
 * BigInteger for EOS.IO smart contract by Mithrilcoin.io .
 */
#include <eosiolib/memory.hpp>

using namespace eosio;

// new/delete operator overloading for eos smart contract by mithrilcoin.io
void* operator new (unsigned int size){
    return malloc( size );
}

void* operator new[] (unsigned int size){
    return malloc( size );
}

void operator delete (void* ptr) _NOEXCEPT {
    free( ptr);
}

void operator delete[] (void* ptr) _NOEXCEPT {
    free( ptr);
}