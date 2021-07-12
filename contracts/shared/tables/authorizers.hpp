#pragma once

struct [[eosio::table, eosio::contract(CONTRACT_NAME)]] authorizer {
    name authorizer;
    uint64_t primary_key() const {
        return authorizer.value;
    }
};

typedef eosio::multi_index<"authorizers"_n, authorizer> authorizers;