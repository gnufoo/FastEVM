#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <string>
#include <eosio/transaction.hpp>
#include <cmath>
#include "evmc.hpp"
#include "fast256.hpp"
#include "fastrlp.hpp"
#include "fasthash.hpp"
#include "uECC.h"

using namespace eosio;
using namespace std;

/**
 * ## TABLE `code`
 *
 * - `{string} code` - FastEVM code
 *
 * ### example
 *
 * ```json
 * {
 *   "code": "..."
 * }
 * ```
 */
struct [[eosio::table("code"), eosio::contract("fastevm")]] code_row {
    string code;

    uint64_t primary_key() const { return 1; }
};
typedef eosio::multi_index<"code"_n, code_row> code_table;

/**
 * ## TABLE `state`
 *
 * - `{checksum256} key` - Checksum256 key
 * - `{checksum256} value` - Checksum256 value
 *
 * ### example
 *
 * ```json
 * {
 *   "key": "...",
 *   "value": "..."
 * }
 * ```
 */
struct [[eosio::table("state"), eosio::contract("fastevm")]] state_row {
    // uint64_t fastid;
    checksum256 key;
    checksum256 value;

    uint64_t primary_key() const { return Fast256(key).fastid(); }
    // checksum256 byhash()const { return Fast256(key).fastid(); }
};
typedef eosio::multi_index<"state"_n, state_row> state_table;

/**
 * ## TABLE `account`
 *
 * - `{checksum256} address`  - Checksum256 key
 * - `{uint64_t}    nonce`    - transaction nonce
 * - `{asset}       balance`  - token balance
 * - `{checksum256} identifier- account identifier
 *
 * ### example
 *
 * ```json
 * {
 *   "key": "...",
 *   "value": "..."
 * }
 * ```
 */
struct [[eosio::table("account"), eosio::contract("fastevm")]] account_row {
    string      address;
    uint64_t    nonce;
    asset       balance;
    checksum160 identifier;
    name        account;

    uint64_t    primary_key()   const { return Fast256(sha256(address.c_str(), address.size())).fastid(); }
    checksum160 by_id()         const { return identifier; }
    uint64_t    by_acc()        const { return account.value;}
};
typedef eosio::multi_index<"account"_n, account_row, 
    // indexed_by<"byid"_n,  const_mem_fun<account_row, checksum160, &account_row::by_id>>,
    // indexed_by<"bysig"_n, const_mem_fun<account_row, checksum256, &account_row::by_sig>>,
    indexed_by<"byacc"_n, const_mem_fun<account_row, uint64_t ,   &account_row::by_acc>>
> 
account_table;

class [[eosio::contract("fastevm")]] FastEVM : public contract {

private:
    // local instances of the multi indexes
    code_table      _code;
    state_table     _state;
    account_table   _account;
    FastHash        _hash;

public:
    using contract::contract;

    /**
     * Construct a new contract given the contract name
     *
     * @param {name} receiver - The name of this contract
     * @param {name} code - The code name of the action this contract is processing.
     * @param {datastream} ds - The datastream used
     */
    FastEVM( name receiver, name code, eosio::datastream<const char*> ds )
        : contract( receiver, code, ds ),
            _code( get_self(), get_self().value ),
            _state( get_self(), get_self().value ),
            _account( get_self(), get_self().value )
    {
        _stack = new Fast256[1024];
        memset(_stack, 0, 1024 * sizeof(Fast256));
        _spp = &_stack[1023];
        _memory = new vector<Fast256>();
        _memory->reserve(256);
        _memorysize = 256;
    }


    /**
     * ## ACTION `updatecode`
     *
     * Update code
     *
     * - **authority**: `get_self()`
     * - **ram_payer**: `get_self()`
     *
     * ### params
     *
     * - `{string} code` - FastEVM code
     *
     * ### example
     *
     * ```bash
     * cleos push action fastevm updatecode '["..."]' -p fastevm
     * ```
     */
    [[eosio::action]]
    void updatecode( string code );

    /**
     * ## ACTION `execute`
     *
     * Execute code
     *
     * - **authority**: `*`
     * - **ram_payer**: N/A
     *
     * ### params
     *
     * - `{string} code` - FastEVM code
     * - `{name} caller` - caller account
     *
     * ### example
     *
     * ```bash
     * cleos push action fastevm execute '["...", "myaccount"]' -p myaccount
     * ```
     */
    [[eosio::action]]
    void execute( string input, name caller );

    /// @abi action
    [[eosio::action]]
    void createcode( string code, name caller );

    /// @abi action
    [[eosio::action]]
    void raw( string transaction, name caller );

    /// @abi action
    [[eosio::action]]
    void create( name account, string identifier, string address );

private:

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

    // execute
    Fast256 execute_code( string code, string input, name caller );
    bool execute_op( uint8_t **opcode );

    uint64_t recover( Fast256 hash, uint8_t *r, uint8_t *s, uint8_t v );

    // state
    void set_state( Fast256 addr, Fast256 content );
    Fast256 get_state( Fast256 addr );

    // utils
    uint8_t *validate( uint8_t *dest );
    uint8_t *string_to_code( string str, int offset = 0 );
    const uint8_t string_to_byte( char c );
    void calculate_memory( uint64_t newsize );
};



