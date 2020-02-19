# Fast EVM

Fast EVM is a EOSIO based software that store & executes solidity byte codes that compatible with Ethereum network.

# How to build

## Make sure you have latest [eosio.cdt](https://github.com/EOSIO/eosio.cdt) installed.

## Fetching the code

`https://github.com/gnufoo/FastEVM.git`

## Building the code

`cd FastEVM`
`makedir build`
`cd build`
`cmake ..`
`make`

## Create contract account

`cleos create account eosio fastsolidity EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV`

## Deploying to the created account

`cleos set contract fastsolidity ./ FastEVM.wasm FastEVM.abi`

