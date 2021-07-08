#!/usr/bin/env bash

eosio-cpp src/cardrewards.cpp -o cardrewards.wasm -abigen -I include -I ./src -I ../shared -R resources -R ricardian