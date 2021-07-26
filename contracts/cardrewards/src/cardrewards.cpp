#include <cardrewards.hpp>

ACTION cardrewards::claim(name account)
{
    // Authority check
    check(has_auth(account), "Insufficient authority.");
    claim_m(account);
}

ACTION cardrewards::claimrholder(name claimer, name rholder)
{
    // Authority check
    check(has_auth(claimer), "Insufficient authority.");
    check_has_auth(claimer, "Claimer is not authorized.");
    claimrholder_m(claimer, rholder);
}

ACTION cardrewards::claimref(name claimer, name referral)
{
    // Authority check
    check(has_auth(claimer), "Insufficient authority.");
    check_has_auth(claimer, "Claimer is not authorized.");
    claimref_m(claimer, referral);
}

ACTION cardrewards::regcol(name collection_name)
{
    // Authority check
    check(has_auth(_self), "Insufficient authority.");
    regcol_m(collection_name);
}

ACTION cardrewards::delcol(name collection_name)
{
    // Authority check
    check(has_auth(_self), "Insufficient authority.");
    delcol_m(collection_name);
}

ACTION cardrewards::regauth(name authorizer)
{
    // Authority check
    check(has_auth(_self), "Insufficient authority.");
    regauth_m(authorizer);
}

ACTION cardrewards::delauth(name authorizer)
{
    // Authority check
    check(has_auth(_self), "Insufficient authority.");
    delauth_m(authorizer);
}

ACTION cardrewards::addmap(name player_name, name rholder, name referral)
{
    // Authority check
    check(has_auth(_self), "Insufficient authority.");
    addmap_m(player_name, rholder, referral);
}

ACTION cardrewards::delmap(name player_name)
{
    // Authority check
    check(has_auth(_self), "Insufficient authority.");
    delmap_m(player_name);
}

ACTION cardrewards::updatemap(name player_name, name rholder, name referral)
{
    // Authority check
    check(has_auth(_self), "Insufficient authority.");
    updatemap_m(player_name, rholder, referral);
}
