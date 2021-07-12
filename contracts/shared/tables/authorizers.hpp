#pragma once

struct [[eosio::table, eosio::contract(CONTRACT_NAME)]] authaccount {
    name authorized;
    uint64_t primary_key() const {
        return authorized.value;
    }
};

typedef eosio::multi_index<"authaccounts"_n, authaccount> authaccounts;