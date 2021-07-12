#pragma once

#define CONTRACT_NAME "cardrewards"
#define ATOMICASSETS "atomicassets"_n
#define ATOMICMARKET "atomicmarket"_n
#define WAX_SYMBOL symbol("WAX", 8)

#define COMMISSION_FOR_FOUNDER 0.02
#define COMMISSION_FOR_RHOLDER 0.05
#define COMMISSION_FOR_REFERRAL 0.01

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/action.hpp>
#include <eosio/transaction.hpp>
#include <eosio/datastream.hpp>
#include <eosio/symbol.hpp>
#include <vector>
#include <atomicdata.hpp>
#include <algorithm>

#include <tables/accounts.hpp>
#include <tables/cards.hpp>
#include <tables/whitecols.hpp>
#include <tables/authorizers.hpp>
#include <tables/mappings.hpp>
#include <tables/referrals.hpp>
#include <tables/rholders.hpp>
#include <tables/atomicassets-interface.hpp>

using namespace eosio;
using namespace std;
using namespace atomicdata;

CONTRACT cardrewards
: public contract
{
using contract::contract;

public:

ACTION claim(name account);

ACTION claimrholder(name claimer, name rholder);

ACTION claimref(name claimer, name referral);

ACTION regcol(name collection_name);

ACTION delcol(name collection_name);

ACTION regauth(name authorizer);

ACTION delauth(name authorizer);

ACTION addmap(name player_name, name rholder, name referral);

ACTION delmap(name player_name);

ACTION updatemap(name player_name, name rholder, name referral);


[[eosio::on_notify("atomicassets::logmint")]] void
onmint(uint64_t asset_id,
       name authorized_minter,
       name collection_name,
       name schema_name,
       int32_t template_id,
       name new_asset_owner,
       ATTRIBUTE_MAP immutable_data,
       ATTRIBUTE_MAP mutable_data,
       vector <asset> backed_tokens,
       ATTRIBUTE_MAP immutable_template_data) {

    onmint_m(asset_id, collection_name, new_asset_owner, immutable_data, mutable_data);

}

[[eosio::on_notify("atomicassets::logtransfer")]] void
ontransfer(name collection_name,
           name from,
           name to,
           vector <uint64_t> asset_ids,
           string memo) {
    ontransfer_m(collection_name, from, to, asset_ids, memo);
}

[[eosio::on_notify("atomicassets::logsetdata")]] void
onsetdata(name asset_owner,
          uint64_t asset_id,
          ATTRIBUTE_MAP old_data,
          ATTRIBUTE_MAP new_data) {
    onsetdata_m(asset_owner, asset_id, old_data, new_data);
}

private:
struct assertsale_action {
    uint64_t sale_id;
    vector <uint64_t> asset_ids_to_assert;
    asset listing_price_to_assert;
    symbol settlement_symbol_to_assert;
};

void delcol_m(name collection_name) {
    whitecols whitecolsTable(_self, _self.value);
    auto itr = whitecolsTable.find(collection_name.value);
    check(itr != whitecolsTable.end(), "The collection has not registed.");
    whitecolsTable.erase(itr);
}

void regcol_m(name collection_name) {
    whitecols whitecolsTable(_self, _self.value);
    auto itr = whitecolsTable.find(collection_name.value);
    check(itr == whitecolsTable.end(), "The collection has registed.");
    whitecolsTable.emplace(_self, [&](auto &a) {
        a.collection_name = collection_name;
    });
}

void delauth_m(name authorizer) {
    authorizers authorizersTable(_self, _self.value);
    auto itr = authorizersTable.find(authorizer.value);
    check(itr != authorizersTable.end(), "The authorizer has not registed.");
    authorizersTable.erase(itr);
}

void regauth_m(name authorizer) {
    authorizers authorizersTable(_self, _self.value);
    auto itr = authorizersTable.find(authorizer.value);
    check(itr == authorizersTable.end(), "The authorizer has registed.");
    authorizersTable.emplace(_self, [&](auto &a) {
        a.authorizer = authorizer;
    });
}

void delmap_m(name player_name) {
    mappings mappingsTable(_self, _self.value);
    auto itr = mappingsTable.find(player_name.value);
    check(itr != mappingsTable.end(), "The mapping has not existed.");
    mappingsTable.erase(itr);
}

void addmap_m(name player_name, name rholder, name referral) {
    mappings mappingsTable(_self, _self.value);
    auto itr = mappingsTable.find(player_name.value);
    check(itr == mappingsTable.end(), "The mapping has existed.");
    mappingsTable.emplace(_self, [&](auto &a) {
        a.player_name = player_name;
        a.rholder = rholder;
        a.referral = referral;
    });
}

void updatemap_m(name player_name, name rholder, name referral) {
    mappings mappingsTable(_self, _self.value);
    auto itr = mappingsTable.find(player_name.value);
    check(itr != mappingsTable.end(), "The mapping has not existed.");
    mappingsTable.modify(itr, _self, [&](auto &a) {
        a.player_name = player_name;
        a.rholder = rholder;
        a.referral = referral;
    });
}

void claim_m(name account) {
    accounts accountTable(_self, account.value);
    const auto &itr = accountTable.get(WAX_SYMBOL.code().raw(),
                                       "No balance object found");
    action(
            permission_level{_self, "active"_n},
            "eosio.token"_n,
            "transfer"_n,
            make_tuple(_self, account, itr.balance, string("Credit for being founder.")))
            .send();
    //Update internal balance after sent commission
    accountTable.modify(itr, _self, [&](auto &a) {
        a.balance = asset{(int64_t) 0, WAX_SYMBOL};
    });
}

void claimrholder_m(name claimer, name rholder) {
    rholders rholdersTb(_self, _self.value);
    auto itr = rholdersTb.find(rholder.value);
    check(itr != rholdersTb.end(), "No balance object found.");
    action(
            permission_level{_self, "active"_n},
            "eosio.token"_n,
            "transfer"_n,
            make_tuple(_self, claimer, itr->balance, string("Credit for rights holder: " + itr->rholder.to_string())))
            .send();
    //Update internal balance after sent commission
    rholdersTb.modify(itr, _self, [&](auto &a) {
        a.balance = asset{(int64_t) 0, WAX_SYMBOL};
    });

}

void claimref_m(name claimer, name referral) {
    referrals referralsTb(_self, _self.value);
    auto itr = referralsTb.find(referral.value);
    check(itr != referralsTb.end(), "No balance object found.");
    action(
            permission_level{_self, "active"_n},
            "eosio.token"_n,
            "transfer"_n,
            make_tuple(_self, claimer, itr->balance, string("Credit for referral: " + itr->referral.to_string())))
            .send();
    //Update internal balance after sent commission
    referralsTb.modify(itr, _self, [&](auto &a) {
        a.balance = asset{(int64_t) 0, WAX_SYMBOL};
    });

}

void onmint_m(uint64_t asset_id, name collection_name, name new_asset_owner, ATTRIBUTE_MAP immutable_data,
              ATTRIBUTE_MAP mutable_data) {
    //Check if collection_name is whitelisted
    whitecols whitecolsTable(_self, _self.value);
    auto itr = whitecolsTable.find(collection_name.value);
    check(itr != whitecolsTable.end(), "This collection has not registed.");

    //Check and create account balance for new founder if not exist
    accounts newOwnerTable(_self, new_asset_owner.value);
    auto newOwner = newOwnerTable.find(WAX_SYMBOL.code().raw());
    if (newOwner == newOwnerTable.end()) {
        newOwnerTable.emplace(_self, [&](auto &a) {
            a.balance = asset{(int64_t) 0, WAX_SYMBOL};
        });
    }

    //Create new asset row
    cards cardTable(_self, _self.value);
    auto newAsset = cardTable.find(asset_id);
    check(newAsset == cardTable.end(), "This asset's existed.");
    //read immutable data to get player_name then find rholder and referral via mapping table
    auto player_itr = immutable_data.find("player_name");
    check(player_itr != immutable_data.end(), "Missing player_name attribute in immutable_data.");
    check(holds_alternative<string>(player_itr->second), "player_name must be type of string");
    name player_name = name(get<string>(player_itr->second));

    mappings mappingsTable(_self, _self.value);
    auto map_itr = mappingsTable.find(player_name.value);
    check(map_itr != mappingsTable.end(),
          "player_name must be registered in mappings table of " + _self.to_string() + "smart contract.");

    //Send setassetdata to atomicassets
    mutable_data["rholder"] = map_itr->rholder.to_string();
    mutable_data["referral"] = map_itr->referral.to_string();
    action(
            permission_level{_self, "active"_n},
            ATOMICASSETS,
            "setassetdata"_n,
            std::make_tuple(_self, new_asset_owner, asset_id, mutable_data))
            .send();

    //Create card data
    cardTable.emplace(_self, [&](auto &a) {
        a.asset_id = asset_id;
        a.founder = new_asset_owner;
        a.rholder = map_itr->rholder;
        a.referral = map_itr->referral;
    });
    //Create balance rows in rholders and referrals if needs
    rholders rholdersTb(_self, _self.value);
    auto rholder_itr = rholdersTb.find(map_itr->rholder.value);
    if (rholder_itr == rholdersTb.end()) {
        rholdersTb.emplace(_self, [&](auto &a) {
            a.rholder = map_itr->rholder;
            a.balance = asset{(int64_t) 0, WAX_SYMBOL};
        });
    }

    referrals referralsTb(_self, _self.value);
    auto referral_itr = referralsTb.find(map_itr->referral.value);
    if (referral_itr == referralsTb.end()) {
        referralsTb.emplace(_self, [&](auto &a) {
            a.referral = map_itr->referral;
            a.balance = asset{(int64_t) 0, WAX_SYMBOL};
        });
    }
}

void ontransfer_m(name collection_name,
                  name from,
                  name to,
                  vector <uint64_t> asset_ids,
                  string memo) {
    // Only run if tranfer from ATOMICMARKET, asset_ids exist in cards table and having purchase memo
    if (from == ATOMICMARKET) {
        vector <string> memo_results;
        splitMemo(memo_results, memo);
        if (memo_results.size() == 2 && memo_results[0] == "AtomicMarket Purchased Sale - ID ") {
            cards cardTable(_self, _self.value);
            char tx_buffer[transaction_size()];
            read_transaction(tx_buffer, transaction_size());
            const vector<char> trx_vector(tx_buffer, tx_buffer + sizeof tx_buffer / sizeof tx_buffer[0]);
            transaction trx = unpack<transaction>(trx_vector);
            for (auto asset_id : asset_ids) {
                auto card_itr = cardTable.find(asset_id);
                // Only run if asset_id exist in cards table
                if (card_itr != cardTable.end()) {
                    for (auto item: trx.actions) {
                        if (item.name == "assertsale"_n && item.account == ATOMICMARKET) {
                            assertsale_action action_data = item.data_as<assertsale_action>();
                            if (find(action_data.asset_ids_to_assert.begin(), action_data.asset_ids_to_assert.end(),
                                     asset_id) != action_data.asset_ids_to_assert.end()) {
                                //Calculate reward for founder and rholder
                                asset founderCommission = asset{
                                        (int64_t)(action_data.listing_price_to_assert.amount * COMMISSION_FOR_FOUNDER),
                                        WAX_SYMBOL};
                                accounts founderTable(_self, card_itr->founder.value);
                                const auto &founder_itr = founderTable.get(WAX_SYMBOL.code().raw(),
                                                                           "no balance object found");
                                founderTable.modify(founder_itr, _self, [&](auto &a) {
                                    a.balance += founderCommission;
                                });

                                if (card_itr->rholder.value != 0) {
                                    asset rholderCommission = asset{
                                            (int64_t)(action_data.listing_price_to_assert.amount *
                                                      COMMISSION_FOR_RHOLDER),
                                            WAX_SYMBOL};
                                    rholders rholdersTb(_self, _self.value);
                                    auto rholder_itr = rholdersTb.find(card_itr->rholder.value);
                                    if (rholder_itr == rholdersTb.end()) {
                                        rholdersTb.emplace(_self, [&](auto &a) {
                                            a.rholder = card_itr->rholder;
                                            a.balance = rholderCommission;
                                        });
                                    } else {
                                        rholdersTb.modify(rholder_itr, _self, [&](auto &a) {
                                            a.balance += rholderCommission;
                                        });
                                    }
                                }
                                if (card_itr->referral.value != 0) {
                                    asset referralCommission = asset{
                                            (int64_t)(action_data.listing_price_to_assert.amount *
                                                      COMMISSION_FOR_REFERRAL),
                                            WAX_SYMBOL};
                                    referrals referralsTb(_self, _self.value);
                                    auto referral_itr = referralsTb.find(card_itr->referral.value);
                                    if (referral_itr == referralsTb.end()) {
                                        referralsTb.emplace(_self, [&](auto &a) {
                                            a.referral = card_itr->referral;
                                            a.balance = referralCommission;
                                        });
                                    } else {
                                        referralsTb.modify(referral_itr, _self, [&](auto &a) {
                                            a.balance += referralCommission;
                                        });
                                    }
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}

void onsetdata_m(name asset_owner,
                 uint64_t asset_id,
                 ATTRIBUTE_MAP old_data,
                 ATTRIBUTE_MAP new_data) {
    cards cardTable(_self, _self.value);
    auto card_itr = cardTable.find(asset_id);
    if (card_itr != cardTable.end()) {
        auto new_rholder_itr = new_data.find("rholder");
        auto new_referral_itr = new_data.find("referral");
        if (new_rholder_itr != new_data.end() || new_referral_itr != new_data.end()) {
            name newRholder = name(std::get<string>(new_rholder_itr->second));
            name newReferral = name(std::get<string>(new_referral_itr->second));
            cardTable.modify(card_itr, _self, [&](auto &a) {
                a.rholder = newRholder;
                a.referral = newReferral;
            });

            //Create balance rows in rholders and referrals if needs
            rholders rholdersTb(_self, _self.value);
            auto rholder_itr = rholdersTb.find(newRholder.value);
            if (rholder_itr == rholdersTb.end()) {
                rholdersTb.emplace(_self, [&](auto &a) {
                    a.rholder = newRholder;
                    a.balance = asset{(int64_t) 0, WAX_SYMBOL};
                });
            }

            referrals referralsTb(_self, _self.value);
            auto referral_itr = referralsTb.find(newReferral.value);
            if (referral_itr == referralsTb.end()) {
                referralsTb.emplace(_self, [&](auto &a) {
                    a.referral = newReferral;
                    a.balance = asset{(int64_t) 0, WAX_SYMBOL};
                });
            }
        }
    }
}

void check_has_auth(name account_to_check, string error_message) {
    authorizers authorizersTable(_self, _self.value);
    auto itr = authorizersTable.find(account_to_check.value);
    check(itr != authorizersTable.end(), error_message);
}

inline void splitMemo(std::vector <std::string> &results, std::string memo) {
    auto end = memo.cend();
    auto start = memo.cbegin();

    for (auto it = memo.cbegin(); it != end; ++it) {
        if (*it == '#') {
            results.emplace_back(start, it);
            start = it + 1;
        }
    }
    if (start != end)
        results.emplace_back(start, end);
}

};
