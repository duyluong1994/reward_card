#include <cardrewards.hpp>

ACTION cardrewards::claim(name account)
{
    // Authority check
    check(has_auth(account), "Insufficient authority.");
    claim_m(account);
}

ACTION cardrewards::regcol(name collection_name)
{
    // Authority check
//    check(has_auth(_self), "Insufficient authority.");
    regcol_m(collection_name);
}

ACTION cardrewards::delcol(name collection_name)
{
    // Authority check
//    check(has_auth(_self), "Insufficient authority.");
    delcol_m(collection_name);
}
