#!/usr/bin/env bash

SC_PATH="/Users/lupo/TheCapital/Colosseum_Card/contracts/cardrewards"
# CLEOS="cleos -u https://wax.greymass.com"
CLEOS="cleos -u https://waxtestnet.greymass.com"
$CLEOS set contract coloseumcard $SC_PATH cardrewards.wasm cardrewards.abi -p coloseumcard@active