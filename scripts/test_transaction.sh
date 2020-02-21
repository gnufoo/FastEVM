cd build
cmake ..
make
#./scripts/build.sh
#eosiocpp -g fastsolidity.abi fastsolidity.cpp
#eosiocpp -o fastsolidity.wasm fastsolidity.cpp

cd ..
../../eos2/eos/stopNodeos.sh
../../eos2/eos/startNodeos.sh
echo starting....
sleep 2
cleos wallet unlock --password PW5K1b58DN1k5CkA23jae9d74mNvyddG4d7hV9qmFuuxFxvMpX5WX
cleos create account eosio fastsolidity EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio gnufoognufoo EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos set contract fastsolidity ./build FastEVM.wasm FastEVM.abi
cleos push action fastsolidity raw '["0xf86b0485012a05f200825208940ecb379ccd62b24c9f24ee73770f625cad163d40870e35fa931a00008026a021daaf344846b38e5bb7f91922e7b9ba15ca49352ef977d15d7395ce11c477ffa07b2affdd2a62b6de75ad752cac1667d68cb81ad6e6e33010c424eeeaab4ca1e1", "eosio"]' -p fastsolidity
