#pragma once

struct [[eosio::table, eosio::contract(CONTRACT_NAME)]] card {
    uint64_t asset_id;
    name founder;
    name rholder;
    name referral;

    uint64_t primary_key() const {
        return asset_id;
    }

    uint64_t by_founder() const {
        return founder.value;
    }

    uint64_t by_rholder() const {
        return rholder.value;
    }

    uint64_t by_referral() const {
        return referral.value;
    }
};

typedef eosio::multi_index<"cards"_n, card,
        eosio::indexed_by<"founder"_n, eosio::const_mem_fun < card, uint64_t, &card::by_founder>>,
eosio::indexed_by<"rholder"_n, eosio::const_mem_fun<card, uint64_t, &card::by_rholder>>,
eosio::indexed_by<"referral"_n, eosio::const_mem_fun<card, uint64_t, &card::by_referral>>>
cards;