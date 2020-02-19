/**
 * new/delete operator overloading for eosio smart contract ( on dawn2.x )
 *
 * @copyright Mithrilcoin.io
 */
#ifndef __EOS_MEM_WRAPPER_H__
#define __EOS_MEM_WRAPPER_H__

#include <eosiolib/memory.hpp>

void* operator new (unsigned int size);
void* operator new[] (unsigned int size);
void operator delete (void* ptr) _NOEXCEPT;
void operator delete[] (void* ptr) _NOEXCEPT;
#endif