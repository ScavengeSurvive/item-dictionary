// built-in include guard removal
// just in case the user has a local dependency with the same file name
#if defined _inc_item_dictionary
	#undef _inc_item_dictionary
#endif
// custom include-guard to ensure we don't duplicate
#if defined _item_dictionary_included
	#endinput
#endif
#define _item_dictionary_included


#include <a_samp>
#include <logger>
#include <item>
#include <map>


// -
// API
// -

// maps item IDs to dictionary map addresses
static Map:itemToDict;

// provides boilerplate for code used in every setter
#define ITEM_DICT_SETTER_BOILERPLATE \
    if(!IsValidItem(id)) return 1; \
    new Map:itemDict = MAP_NULL; \
    if(MAP_contains_val(itemToDict, _:id)) { \
        itemDict = Map:MAP_get_val_val(itemToDict, _:id); \
    }

// provides boilerplate for code used in every getter
#define ITEM_DICT_GETTER_BOILERPLATE \
    if(!IsValidItem(id)) return 1; \
    new Map:itemDict = MAP_NULL; \
    if(!MAP_contains_val(itemToDict, _:id)) return 2; \
    itemDict = Map:MAP_get_val_val(itemToDict, _:id); \
    if(!MAP_contains_str(itemDict, field)) return 3;

stock SetItemFieldInt(Item:id, field[], value) {
    ITEM_DICT_SETTER_BOILERPLATE

    MAP_insert_str_val(itemDict, field, value);
    MAP_insert_val_val(itemToDict, _:id, _:itemDict);
    return 0;
}

stock GetItemFieldInt(Item:id, field[], &value) {
    ITEM_DICT_GETTER_BOILERPLATE

    value = MAP_get_str_val(itemDict, field);
    return 0;
}

stock SetItemFieldFloat(Item:id, field[], Float:value) {
    ITEM_DICT_SETTER_BOILERPLATE

    MAP_insert_str_val(itemDict, field, _:value);
    MAP_insert_val_val(itemToDict, _:id, _:itemDict);
    return 0;
}

stock GetItemFieldFloat(Item:id, field[], &Float:value) {
    ITEM_DICT_GETTER_BOILERPLATE

    value = Float:MAP_get_str_val(itemDict, field);
    return 0;
}

stock SetItemFieldArray(Item:id, field[], value[], length = sizeof value) {
    ITEM_DICT_SETTER_BOILERPLATE

    MAP_insert_str_arr(itemDict, field, value, length);
    MAP_insert_val_val(itemToDict, _:id, _:itemDict);
    return 0;
}

stock GetItemFieldArray(Item:id, field[], value[], length = sizeof value) {
    ITEM_DICT_GETTER_BOILERPLATE

    MAP_get_str_arr(itemDict, field, value, length);
    return 0;
}

stock SetItemFieldString(Item:id, field[], value[]) {
    ITEM_DICT_SETTER_BOILERPLATE

    MAP_insert_str_str(itemDict, field, value);
    MAP_insert_val_val(itemToDict, _:id, _:itemDict);
    return 0;
}

stock GetItemFieldString(Item:id, field[], value[], length = sizeof value) {
    ITEM_DICT_GETTER_BOILERPLATE

    MAP_get_str_arr(itemDict, field, value, length);
    return 0;
}
