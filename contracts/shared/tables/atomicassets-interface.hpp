#pragma once
using namespace atomicdata;
static constexpr name ATOMICASSETS_ACCOUNT = name("atomicassets");
//Scope: collection_name
struct schemas_s {
    name            schema_name;
    vector <FORMAT> format;

    uint64_t primary_key() const { return schema_name.value; }
};

typedef multi_index <name("schemas"), schemas_s> schemas_t;

schemas_t get_schemas(name collection_name) {
    return schemas_t(ATOMICASSETS_ACCOUNT, collection_name.value);
}