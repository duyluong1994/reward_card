#pragma once

struct [[eosio::table, eosio::contract(CONTRACT_NAME)]] mapping {
    name player_name;
    name rholder;
    name referral;

    uint64_t primary_key() const {
        return player_name.value;
    }

    uint64_t by_rholder() const {
        return rholder.value;
    }

    uint64_t by_referral() const {
        return referral.value;
    }
};

typedef eosio::multi_index<"mappings"_n, mapping,
        eosio::indexed_by<"rholder"_n, eosio::const_mem_fun < mapping, uint64_t, &mapping::by_rholder>>,
eosio::indexed_by<"referral"_n, eosio::const_mem_fun<mapping, uint64_t, &mapping::by_referral>>>
mappings;