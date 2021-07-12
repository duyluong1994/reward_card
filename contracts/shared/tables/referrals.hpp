#pragma once

struct [[eosio::table, eosio::contract(CONTRACT_NAME)]] referral {
    name referral;
    asset balance;

    uint64_t primary_key() const { return referral.value; }
};

typedef eosio::multi_index<"referrals"_n, referral> referrals;
