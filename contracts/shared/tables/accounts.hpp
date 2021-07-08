#pragma once

struct [[eosio::table, eosio::contract(CONTRACT_NAME)]] account {
    asset balance;

    uint64_t primary_key() const { return balance.symbol.code().raw(); }
};

typedef eosio::multi_index<"accounts"_n, account> accounts;
