#pragma once

#define CONTRACT_NAME "cardrewards"
#define ATOMICASSETS "atomicassets"_n
#define ATOMICMARKET "atomicmarket"_n
#define WAX_SYMBOL symbol("WAX", 8)

#define COMMISSION_FOR_FOUNDER 0.02
#define COMMISSION_FOR_RHOLDER 0.05

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/action.hpp>
#include <eosio/transaction.hpp>
#include <eosio/datastream.hpp>
#include <eosio/symbol.hpp>
#include <vector>
#include <atomicdata.hpp>

#include <tables/accounts.hpp>
#include <tables/cards.hpp>
#include <tables/whitecols.hpp>
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

ACTION regcol(name collection_name);

ACTION delcol(name collection_name);

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

    onmint_m(asset_id, collection_name, new_asset_owner, mutable_data);

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
    check(itr != whitecolsTable.end(), "This collection has not registed.");
    whitecolsTable.erase(itr);
}

void regcol_m(name collection_name) {
    whitecols whitecolsTable(_self, _self.value);
    auto itr = whitecolsTable.find(collection_name.value);
    check(itr == whitecolsTable.end(), "This collection has registed.");
    whitecolsTable.emplace(_self, [&](auto &a) {
        a.collection_name = collection_name;
    });
}

void claim_m(name account) {
    accounts accountTable(_self, account.value);
    const auto &itr = accountTable.get(WAX_SYMBOL.code().raw(),
                                       "no balance object found");
    action(
            permission_level{_self, "active"_n},
            "eosio.token"_n,
            "transfer"_n,
            make_tuple(_self, account, itr.balance, string("Credit for being founder or image right holder.")))
            .send();
    //Update internal balance after sent commission
    accountTable.modify(itr, _self, [&](auto &a) {
        a.balance = asset{(int64_t) 0, WAX_SYMBOL};
    });

}

void onmint_m(uint64_t asset_id, name collection_name, name new_asset_owner, ATTRIBUTE_MAP mutable_data) {
    //Check if collection_name is whitelisted
    whitecols whitecolsTable(_self, _self.value);
    auto itr = whitecolsTable.find(collection_name.value);
    check(itr != whitecolsTable.end(), "This collection has not registed.");

    //Check and create account row for new owner if not exist
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

    //read mutable data to get rholder
    auto rholder_itr = mutable_data.find("rholder");
    auto newRholder = name();
    if (rholder_itr != mutable_data.end()) {
        check(holds_alternative<string>(rholder_itr->second), "rholder must be string");
        check(is_account(name(std::get<string>(rholder_itr->second))),
              "rholder must be WAX account.");
        newRholder = name(std::get<string>(rholder_itr->second));
        //Create balance for rholder if need
        accounts rholderTable(_self, newRholder.value);
        auto itr = rholderTable.find(WAX_SYMBOL.code().raw());
        if (itr == rholderTable.end()) {
            rholderTable.emplace(_self, [&](auto &a) {
                a.balance = asset{(int64_t) 0, WAX_SYMBOL};
            });
        }
    }

    cardTable.emplace(_self, [&](auto &a) {
        a.asset_id = asset_id;
        a.founder = new_asset_owner;
        a.rholder = newRholder;
    });
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
            for (auto asset_id : asset_ids) {
                auto card_itr = cardTable.find(asset_id);
                // Only run if asset_id exist in cards table
                if (card_itr != cardTable.end()) {
                    char tx_buffer[transaction_size()];
                    read_transaction(tx_buffer, transaction_size());
                    const vector<char> trx_vector(tx_buffer, tx_buffer + sizeof tx_buffer / sizeof tx_buffer[0]);
                    transaction trx = unpack<transaction>(trx_vector);
                    for (auto item: trx.actions) {
                        if (item.name == "assertsale"_n) {
                            assertsale_action action_data = item.data_as<assertsale_action>();
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
                                        (int64_t)(action_data.listing_price_to_assert.amount * COMMISSION_FOR_RHOLDER),
                                        WAX_SYMBOL};
                                accounts rholderTable(_self, card_itr->rholder.value);
                                const auto &rholder_itr = rholderTable.get(WAX_SYMBOL.code().raw(),
                                                                           "no balance object found");
                                rholderTable.modify(rholder_itr, _self, [&](auto &a) {
                                    a.balance += rholderCommission;
                                });
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
        auto old_rholder_itr = old_data.find("rholder");
        auto new_rholder_itr = new_data.find("rholder");
        if (old_rholder_itr != old_data.end() && new_rholder_itr != new_data.end()) {
            check(holds_alternative<string>(old_rholder_itr->second), "rholder of old data must be string");
            check(holds_alternative<string>(new_rholder_itr->second), "rholder of new data must be string");
            check(is_account(name(std::get<string>(new_rholder_itr->second))),
                  "rholder of new data must be WAX account.");
            name newRholder = name(std::get<string>(new_rholder_itr->second));
            cardTable.modify(card_itr, _self, [&](auto &a) {
                a.rholder = newRholder;
            });

            //Create balance for rholder if need
            accounts rholderTable(_self, newRholder.value);
            auto itr = rholderTable.find(WAX_SYMBOL.code().raw());
            if (itr == rholderTable.end()) {
                rholderTable.emplace(_self, [&](auto &a) {
                    a.balance = asset{(int64_t) 0, WAX_SYMBOL};
                });
            }
        }
    }
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
