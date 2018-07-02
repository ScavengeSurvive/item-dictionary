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
#include <requests>
#include <YSI\y_hooks>


// -
// API
// -

// maps item IDs to dictionary map addresses
static Map:itemToDict;

// provides boilerplate for code used in every setter
#define ITEM_DICT_SETTER_BOILERPLATE \
    if(!IsValidItem(id)) return IDERR_INVALID_ITEM; \
    new Map:itemDict = MAP_NULL; \
    if(MAP_contains_val(itemToDict, _:id)) { \
        itemDict = Map:MAP_get_val_val(itemToDict, _:id); \
    }

// provides boilerplate for code used in every getter
#define ITEM_DICT_GETTER_BOILERPLATE \
    if(!IsValidItem(id)) return IDERR_INVALID_ITEM; \
    new Map:itemDict = MAP_NULL; \
    if(!MAP_contains_val(itemToDict, _:id)) return IDERR_NO_DATA; \
    itemDict = Map:MAP_get_val_val(itemToDict, _:id); \
    if(!MAP_contains_str(itemDict, field)) return IDERR_NO_FIELD;

enum {
    IDERR_NONE,
    IDERR_INVALID_ITEM,
    IDERR_NO_DATA,
    IDERR_NO_FIELD,
    IDERR_TYPE_MISMATCH,
    IDERR_INVALID_ARGS,
    IDERR_ALLOC_FAILURE,
}

static enum {
    _ID_FIELD_INT,
    _ID_FIELD_FLOAT,
    _ID_FIELD_ARRAY,
    _ID_FIELD_STRING,
}

static keyBuffer[2048];
static valBuffer[2048];


stock SetItemFieldInt(Item:id, field[], value) {
    ITEM_DICT_SETTER_BOILERPLATE

    new arr[2] = {_ID_FIELD_INT, 0};
    arr[1] = value;
    MAP_insert_str_arr(itemDict, field, arr);
    MAP_insert_val_val(itemToDict, _:id, _:itemDict);
    return 0;
}

stock GetItemFieldInt(Item:id, field[], &value) {
    ITEM_DICT_GETTER_BOILERPLATE

    new arr[2];
    MAP_get_str_arr(itemDict, field, arr);
    if(arr[0] != _ID_FIELD_INT) {
        return IDERR_TYPE_MISMATCH;
    }
    value = arr[1];
    return 0;
}

stock SetItemFieldFloat(Item:id, field[], Float:value) {
    ITEM_DICT_SETTER_BOILERPLATE

    new arr[2] = {_ID_FIELD_FLOAT, 0};
    arr[1] = _:value;
    MAP_insert_str_arr(itemDict, field, arr);
    MAP_insert_val_val(itemToDict, _:id, _:itemDict);
    return 0;
}

stock GetItemFieldFloat(Item:id, field[], &Float:value) {
    ITEM_DICT_GETTER_BOILERPLATE

    new arr[2];
    MAP_get_str_arr(itemDict, field, arr);
    if(arr[0] != _ID_FIELD_FLOAT) {
        return IDERR_TYPE_MISMATCH;
    }
    value = Float:arr[1];
    return 0;
}

stock SetItemFieldArray(Item:id, field[], value[], length = sizeof value, bool:stringType = false) {
    ITEM_DICT_SETTER_BOILERPLATE

    // element 0 will contain a type ID so increase the value size by 1
    length += 1;

    new keySize = strlen(field) + 1;
    if ((length == 0) || (keySize == 0)) {
        return IDERR_INVALID_ARGS;
    }

    new Pointer:keyPtr = MEM_new_arr(field, keySize);
    if (keyPtr == MEM_NULLPTR) {
        return IDERR_ALLOC_FAILURE;
    }

    // allocate an empty array
    new Pointer:valPtr = MEM_new(length);

    // set index 0 to the type
    if(stringType) {
        MEM_set_val(valPtr, 0, _ID_FIELD_STRING);
    } else {
        MEM_set_val(valPtr, 0, _ID_FIELD_ARRAY);
    }

    // copy value to array to index 1
    MEM_set_arr(valPtr, 1, value, length - 1);
    if (valPtr == MEM_NULLPTR) {
        err("failed to allocate value array");
        MEM_delete(keyPtr);
        return IDERR_ALLOC_FAILURE;
    }

    _item_dict_MAP_insert(itemDict, keyPtr, valPtr);
    MEM_delete(valPtr);
    MEM_delete(keyPtr);

    MAP_insert_val_val(itemToDict, _:id, _:itemDict);
    return 0;
}

stock GetItemFieldArray(Item:id, field[], value[], length = sizeof value, bool:stringType = false) {
    ITEM_DICT_GETTER_BOILERPLATE

    new
        keySize,
        Pointer:valPtr,
        valSize;

    keySize = strlen(field) + 1;
    valPtr = MAP_get_ptr_arr(itemDict, field, keySize);
    valSize = MEM_get_size(valPtr) - 1;

    new type = MEM_get_val(valPtr, 0);
    if(
        (stringType && type != _ID_FIELD_STRING) ||
        (!stringType && type != _ID_FIELD_ARRAY)
    ) {
        return IDERR_TYPE_MISMATCH;
    }

    MEM_get_arr(valPtr, 1, value, (valSize < length) ? valSize : length);

    return 0;
}

stock SetItemFieldString(Item:id, field[], value[]) {
    return SetItemFieldArray(id, field, value, strlen(value), true);
}

stock GetItemFieldString(Item:id, field[], value[], length = sizeof value) {
    return GetItemFieldArray(id, field, value, length, true);
}

stock GetItemFieldMapPointer(Item:id, &Map:ptr) {
    ITEM_DICT_GETTER_BOILERPLATE

    ptr = itemDict;
    return 0;
}

// JSON serialisation functions

stock GetItemFieldsAsJSON(Item:id, &Node:node) {
    if(!IsValidItem(id)) {
        return 1;
    }

    if(!MAP_contains_val(itemToDict, _:id)) {
        return 2;
    }

    new JSON_NODE:nodeType = JsonNodeType(node);
    if(nodeType != JSON_NODE_OBJECT) {
        if(nodeType == JSON_NODE_NULL) {
            node = JsonObject();
        } else {
            return 3;
        }
    }

    new
        Map:itemDict = Map:MAP_get_val_val(itemToDict, _:id),
        keySize,
        valSize,
        type;

    for (
        new Pointer:keyPtr, Pointer:valPtr, Map:tmpMap = MAP_iter_get(itemDict, keyPtr, valPtr);
        tmpMap != MAP_NULL;
        tmpMap = MAP_iter_next(tmpMap, MAP_NULL, keyPtr, valPtr)
    ) {
        keySize = MEM_get_size(keyPtr);
        valSize = MEM_get_size(valPtr);

        if(valSize == 1) {
            err("item field contains data of length 1",
                _i("itemid", _:id));
            continue;
        }

        MEM_UM_zero(UnmanagedPointer:MEM_UM_get_addr(keyBuffer[0]), sizeof keyBuffer);
        MEM_UM_zero(UnmanagedPointer:MEM_UM_get_addr(valBuffer[0]), sizeof valBuffer);
        MEM_get_arr(keyPtr, _, keyBuffer, (keySize < sizeof keyBuffer) ? keySize : sizeof keyBuffer);
        type = MEM_get_val(valPtr, 0);
        MEM_get_arr(valPtr, 1, valBuffer, (valSize < sizeof valBuffer) ? valSize - 1 : sizeof valBuffer);

        switch(type) {
        case _ID_FIELD_INT:
            JsonSetInt(node, keyBuffer, valBuffer[0]);
        case _ID_FIELD_FLOAT:
            JsonSetFloat(node, keyBuffer, Float:valBuffer[0]);
        // case _ID_FIELD_ARRAY:
        //     JsonSetInt(node, keyBuffer, valBuffer);
        case _ID_FIELD_STRING:
            JsonSetString(node, keyBuffer, valBuffer);
        default:
            err("unknown type identifier",
                _s("key", keyBuffer),
                _i("id", type));
        }
    }

    return 0;
}

// -
// Internal
// -

hook OnItemDestroyed(Item:id) {
    new Map:itemDict = MAP_NULL;
    if(MAP_contains_val(itemToDict, _:id)) {
        itemDict = Map:MAP_get_val_val(itemToDict, _:id);
        MAP_clear(itemDict);
        MAP_remove_val(itemToDict, _:id);
    }

    return Y_HOOKS_CONTINUE_RETURN_0;
}

// -
// Note:
//
// Map insert implementation has been copied from pawn-map because it's declared
// static and thus, inaccessible to this library.
// Hopefully this can be changed in a future update to pawn-map to allow for
// advanced use-cases such as that in this library.
// -

static stock _item_dict_MAP_insert(&Map:map, Pointer:key_ptr, Pointer:value_ptr)
{
    if (map == MAP_NULL)
    {
        new m[MAP_struct];
        m[MAP_struct_key_hash] = MAP_hash_key(key_ptr);
        m[MAP_struct_key_ptr] = MEM_clone(key_ptr);
        m[MAP_struct_value_ptr] = MEM_clone(value_ptr);
        map = Map:MEM_new_arr(m);
    }
    else
    {
        new Map:om = map, Map:nm, c, key_hash = MAP_hash_key(key_ptr);
        while (om != MAP_NULL)
        {
            c = MAP_node_compare(om, key_hash, key_ptr);
            if (c < 0)
            {
                nm = Map:MEM_get_val(Pointer:om, _:MAP_struct_left);
                if (nm == MAP_NULL)
                {
                    new m[MAP_struct];
                    m[MAP_struct_parent] = om;
                    m[MAP_struct_key_hash] = key_hash;
                    m[MAP_struct_key_ptr] = MEM_clone(key_ptr);
                    m[MAP_struct_value_ptr] = MEM_clone(value_ptr);
                    nm = Map:MEM_new_arr(m);
                    if (nm != MAP_NULL)
                    {
                        MEM_set_val(Pointer:om, _:MAP_struct_left, _:nm);
                    }
                    break;
                }
                else
                {
                    om = nm;
                }
            }
            else if (c > 0)
            {
                nm = Map:MEM_get_val(Pointer:om, _:MAP_struct_right);
                if (nm == MAP_NULL)
                {
                    new m[MAP_struct];
                    m[MAP_struct_parent] = om;
                    m[MAP_struct_key_hash] = key_hash;
                    m[MAP_struct_key_ptr] = MEM_clone(key_ptr);
                    m[MAP_struct_value_ptr] = MEM_clone(value_ptr);
                    nm = Map:MEM_new_arr(m);
                    if (nm != MAP_NULL)
                    {
                        MEM_set_val(Pointer:om, _:MAP_struct_right, _:nm);
                    }
                    break;
                }
                else
                {
                    om = nm;
                }
            }
            else
            {
                MEM_set_val(Pointer:om, _:MAP_struct_value_ptr, _:MEM_clone(value_ptr));
                break;
            }
        }
    }
}

// Map hash key
// From Java String hashCode(): s[0]*31^(n - 1) + s[1]*31^(n - 2) + ... + s[n - 1]
static stock MAP_hash_key(Pointer:key_ptr)
{
    new ret;
    for (new i, sz = MEM_get_size(key_ptr); i < sz; i++)
    {
        ret = (31 * ret) + MEM_get_val(key_ptr, i);
    }
    return ret;
}

// Map node compare
static stock MAP_node_compare(Map:map, key_hash, Pointer:key_ptr)
{
    new ret;
    if (map != MAP_NULL)
    {
        new m[MAP_struct], ok, nk;
        MEM_get_arr(Pointer:map, _, m);
        if (key_hash < m[MAP_struct_key_hash])
        {
            ret = -1;
        }
        else if (key_hash > m[MAP_struct_key_hash])
        {
            ret = 1;
        }
        else
        {
            new key_size = MEM_get_size(key_ptr), other_key_size = MEM_get_size(m[MAP_struct_key_ptr]);
            if (key_size < other_key_size)
            {
                ret = -1;
            }
            else if (key_size > other_key_size)
            {
                ret = 1;
            }
            else
            {
                for (new i = 0; i < key_size; i++)
                {
                    ok = MEM_get_val(m[MAP_struct_key_ptr], i);
                    nk = MEM_get_val(key_ptr, i);
                    if (nk < ok)
                    {
                        ret = -1;
                        break;
                    }
                    else if (nk > ok)
                    {
                        ret = 1;
                        break;
                    }
                }
            }
        }
    }
    return ret;
}
