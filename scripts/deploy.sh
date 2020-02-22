#!/bin/bash

# Config Constants

CYAN='\033[1;36m'
GREEN='\033[0;32m'
NC='\033[0m'

# load default key into local wallet
cleos wallet import --private-key 5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3

# create accounts
echo -e "${CYAN}-----------------------CREATING ACCOUNTS-----------------------${NC}"
cleos create account eosio fastevm EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio myaccount EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos set account permission fastevm active --add-code

# deploy smart contract
echo -e "${CYAN}-----------------------DEPLOYING CONTRACTS-----------------------${NC}"
cleos set contract fastevm ./dist fastevm.wasm fastevm.abi
