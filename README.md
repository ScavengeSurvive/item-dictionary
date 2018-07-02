# item-dictionary

[![sampctl](https://shields.southcla.ws/badge/sampctl-item--dictionary-2f2f2f.svg?style=for-the-badge)](https://github.com/ScavengeSurvive/item-dictionary)

A successor to item-array-data, this package provides items with a dynamically
allocated dictionary-style data storage.

Store arbitrary data to items using strings as keys. Backed by BigETI/pawn-map
for dynamically allocated hash tables.

## Installation

Simply install to your project:

```bash
sampctl package install ScavengeSurvive/item-dictionary
```

Include in your code and begin using the library:

```pawn
#include <item-dictionary>
```

## Usage

The data accessors are very simple:

### `SetItemFieldT(Item:id, field[], value)`

Where T is one of `Int`, `Float`, `Array` or `String`.

Sets the specified `field` to `value`. Optional `length` argument is provided
for `Array` variant.

Returns 0 on success, 1 if the item ID is invalid.

### `GetItemFieldT(Item:id, field[], &value)`

Where T is one of `Int`, `Float`, `Array` or `String`..

Stores to `value` the data held in the item's dictionary addressed by `field`.
Optional `length` argument is provided for `Array` and `String` variants.

Returns 0 on success, 1 if the item ID is invalid, 2 if the item exists but does
not have any data stored with it and 3 if the item exists, has data but does not
contain an entry for the specified `field`.

## Testing

To test, simply run the package:

```bash
sampctl package run
```
