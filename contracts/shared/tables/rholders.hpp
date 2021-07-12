#pragma once

struct [[eosio::table, eosio::contract(CONTRACT_NAME)]] rholder {
    name rholder;
    asset balance;

    uint64_t primary_key() const { return rholder.value; }
};

typedef eosio::multi_index<"rholders"_n, rholder> rholders;
