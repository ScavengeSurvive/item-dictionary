#include "item-dictionary"

#define RUN_TESTS
#include <YSI\y_testing>
#include <test-boilerplate-items>


// main() {
//     new int;
//     SetItemFieldInt(Item:0, "", int);
//     GetItemFieldInt(Item:0, "", int);
//     new Float:f;
//     SetItemFieldFloat(Item:0, "", f);
//     GetItemFieldFloat(Item:0, "", f);
//     new a[1], len;
//     SetItemFieldArray(Item:0, "", a, len);
//     GetItemFieldArray(Item:0, "", a, len);
//     SetItemFieldString(Item:0, "", a);
//     GetItemFieldString(Item:0, "", a, len);
// }

Test:ItemFieldInt() {
    new Item:id = CreateItem(item_Medkit);
    new ret;

    ret = SetItemFieldInt(id, "Int1", 76);
    ASSERT(ret == 0);

    new got;
    ret = GetItemFieldInt(id, "Int1", got);
    ASSERT(ret == 0);
    ASSERT(got == 76);

    ret = GetItemFieldInt(id, "Int2", got);
    ASSERT(ret == 3);

    ret = SetItemFieldInt(Item:823095, "Int1", 76);
    ASSERT(ret == 1);

    ret = GetItemFieldInt(Item:823095, "Int1", got);
    ASSERT(ret == 1);

    new Item:id2 = CreateItem(item_Medkit);
    ret = GetItemFieldInt(id2, "Int1", got);
    ASSERT(ret == 2);
}

Test:ItemFieldFloat() {
    new Item:id = CreateItem(item_Medkit);
    new ret;

    ret = SetItemFieldFloat(id, "Float1", 76.53);
    ASSERT(ret == 0);

    new Float:got;
    ret = GetItemFieldFloat(id, "Float1", got);
    ASSERT(ret == 0);
    ASSERT(got == 76.53);

    ret = GetItemFieldFloat(id, "Float2", got);
    ASSERT(ret == 3);

    ret = SetItemFieldFloat(Item:823095, "Float1", 76.53);
    ASSERT(ret == 1);

    ret = GetItemFieldFloat(Item:823095, "Float1", got);
    ASSERT(ret == 1);

    new Item:id2 = CreateItem(item_Medkit);
    ret = GetItemFieldFloat(id2, "Float1", got);
    ASSERT(ret == 2);
}

Test:ItemFieldArray() {
    new Item:id = CreateItem(item_Medkit);
    new ret;

    ret = SetItemFieldArray(id, "Array1", {2, 4, 6, 8});
    ASSERT(ret == 0);


    new got[4];
    new want[4] = {2, 4, 6, 8};
    ret = GetItemFieldArray(id, "Array1", got);
    ASSERT(ret == 0);
    for(new i; i < 4; i++) {
        ASSERT(got[i] == want[i]);
    }

    ret = GetItemFieldArray(id, "Array2", got);
    ASSERT(ret == 3);

    ret = SetItemFieldArray(Item:823095, "Array1", {2, 4, 6, 8});
    ASSERT(ret == 1);

    ret = GetItemFieldArray(Item:823095, "Array1", got);
    ASSERT(ret == 1);

    new Item:id2 = CreateItem(item_Medkit);
    ret = GetItemFieldArray(id2, "Array1", got);
    ASSERT(ret == 2);
}

Test:ItemFieldString() {
    new Item:id = CreateItem(item_Medkit);
    new ret;

    ret = SetItemFieldString(id, "String1", "hello world!");
    ASSERT(ret == 0);


    new got[32];
    new want[32] = "hello world!";
    ret = GetItemFieldString(id, "String1", got);
    ASSERT(ret == 0);
    ASSERT(strlen(got) == strlen(want));
    for(new i; i < strlen(got); i++) {
        ASSERT(got[i] == want[i]);
    }

    ret = GetItemFieldString(id, "String2", got);
    ASSERT(ret == 3);

    ret = SetItemFieldString(Item:823095, "String1", "hello world!");
    ASSERT(ret == 1);

    ret = GetItemFieldString(Item:823095, "String1", got);
    ASSERT(ret == 1);

    new Item:id2 = CreateItem(item_Medkit);
    ret = GetItemFieldString(id2, "String1", got);
    ASSERT(ret == 2);
}

Test:GetItemFieldsAsJSON() {
    new Item:id = CreateItem(item_Medkit);
    new ret;

    ret += SetItemFieldInt(id, "key1", 32);
    ret += SetItemFieldFloat(id, "key2", 42.7);
    // TODO: JsonSetArray is needed for this
    // ret += SetItemFieldArray(id, "key3", {2, 4, 6, 8});
    ret += SetItemFieldString(id, "key4", "value");
    ASSERT(ret == 0);

    new Node:node;
    ret = GetItemFieldsAsJSON(id, node);
    ASSERT(ret == 0);

    new buf[512];
    JsonStringify(node, buf);
    print(buf);

    ASSERT(!strcmp(buf, "{\"key1\":32,\"key2\":42.700000762939453,\"key4\":\"value\"}"));
}
