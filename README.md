# FastEVM

FastEVM is a EOSIO based software that store & executes solidity byte codes that is compatible with Ethereum network.

# How to build

Use the latest [eosio.cdt](https://github.com/EOSIO/eosio.cdt)

```
git clone https://github.com/gnufoo/FastEVM.git
cd FastEVM
./scripts/build.sh
```

## Deploy

```
cleos set contract fastevm ./dist FastEVM.wasm FastEVM.abi
```

