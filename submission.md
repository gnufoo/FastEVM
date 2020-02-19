## Inspiration

I'm the CTO of team FAST @ leading blockchain/decentralization dev firm.
We've built more than 20 different dapps on EOS last year, and we don't do things that exists already.

So the challenge attracts me at the first sight.

## What it does

It takes only 24k RAM to deploy an EVM on EOSIO mainnet, which enables developers to fast deploying their existing smart contract's byte code on Ethereum to EOS without even touch the solidity source code.

It is a pure wasm smart contract implementation with no code modification to anything else, and it's fast, the ERC20 transfer only takes around 9000 us on EOS testnet.

It stores Ethereum persistence storage values to EOS multiindex table.

It has a toolset to map 64bits eos account_name to 256bits address, so basically speaking it uses EOS account authorization instead of Ethereums' public/private key pairs because I think EOS's account authorization is more advanced than public/private keypair, it provides multi-signature in a better way.

It uses EOS's sha256 instead of Ethereum's keccak256 hasing function to get better melting to the existing EOS eco-system, for instance, the generated hash can be easily verified by other EOS smart contracts.

## How I built it

I built it in 7 days from scratch.
**day 1**
_ Creating a simplest smart contract for only two actions, updatecode and execute for constructing the testing environment. The most difficult part is getting understand of how EVM works. _

**day 2**
_ Started real work with integrating evm with smart contract, the most difficult part is writing byte codes to assembly within the smart contract. _

**day 3**
_ Making custom 256 bits class, implementing features. _

**day 4 **
_ I was wrong about all the data storings, rewrite. _

**day 5**
_ Almost implemented ERC20 transfer from pure eosio smart contract code. _

**day 6**
_ Created Remix<->fastsolidity test & validation pipeline. Got ERC20 contract deploy working finally with all the data & code working. _

**day 7**
_ Making nodejs tools for data conversion and generation of input data. _

## Challenges I ran into

Implementing arithmetic & logistical algorithm for 256 bits data structure.
Get to understand of what exactly does each EVM opcode is doing, and all the boundry cases.
Create the test environment.

## Accomplishments that I'm proud of

I was doubt about if EOS's evm's performance can handle the intepreting the EVM opcode by high level languages, at the first built, the ERC20 transfer ate around 30000 us, and I'm able to optimize the execution time to 9000 us.

I was able to finish the first working build before the vacation ends, so that it will not affect my work. =)

## What I learned

I've learned the exactly role of a VM inside an public chain structure, and how it works. It brings me a lot of idea on how to improve this mechanism to make it more scalable.

## What's next for FastEVM

It will work seamlessly with all of our other dapps:
1, Transit^ integration, Transit^ is a cross chain token protocal which already implemented a bridge between ETH/EOS, once FastEVM integrated with Transit^ it will make ETH users running all of their product on EOS without a notice.
2, 