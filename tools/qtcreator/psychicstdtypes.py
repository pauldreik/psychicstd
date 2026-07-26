"""Qt Creator debugging helpers for psychicstd types.

Select this file under:
Preferences > Debugger > Locals & Expressions > Extra Debugging Helper
"""

import stdtypes
from dumper import Children


def _dump_basic_string(d, value, fallback, char_type=None):
    try:
        data = value["data_"].pointer()
        size = int(value["size_"])
    except Exception:  # noqa: BLE001 - debugger backends use different exceptions.
        fallback(d, value)
        return

    if char_type is None:
        char_type = value.type[0]
    d.check(0 <= size)
    d.putCharArrayHelper(data, size, char_type, d.currentItemFormat())


def qdump__std__basic_string(d, value):
    _dump_basic_string(d, value, stdtypes.qdump__std__basic_string)


def qdump__std__string(d, value):
    _dump_basic_string(
        d,
        value,
        stdtypes.qdump__std__string,
        d.createType("char"),
    )


def qdump__std__wstring(d, value):
    _dump_basic_string(
        d,
        value,
        stdtypes.qdump__std__wstring,
        d.createType("wchar_t"),
    )


def qdump__std__vector(d, value):
    try:
        data = value["data_"].pointer()
        size = int(value["size_"])
        capacity = int(value["cap_"])
    except Exception:  # noqa: BLE001 - debugger backends use different exceptions.
        stdtypes.qdump__std__vector(d, value)
        return

    d.check(0 <= size <= capacity)
    if size:
        d.checkPointer(data)
    d.putItemCount(size)
    d.putPlotData(data, size, value.type[0])


def qdump__std__optional(d, value):
    try:
        initialized = bool(value["has_"].integer())
        payload = value["val_"]
    except Exception:  # noqa: BLE001 - debugger backends use different exceptions.
        stdtypes.qdump__std__optional(d, value)
        return

    if initialized:
        d.putItem(payload)
        d.putBetterType(value.type[0])
    else:
        d.putSpecialValue("empty")


def _dump_smart_pointer(d, value, fallback):
    try:
        pointer = value["ptr_"]
    except Exception:  # noqa: BLE001 - debugger backends use different exceptions.
        fallback(d, value)
        return

    if pointer.pointer() == 0:
        d.putValue("(null)")
    else:
        d.putItem(pointer.dereference())
        d.putBetterType(value.type)


def qdump__std__unique_ptr(d, value):
    _dump_smart_pointer(d, value, stdtypes.qdump__std__unique_ptr)


def qdump__std__shared_ptr(d, value):
    _dump_smart_pointer(d, value, stdtypes.qdump__std__shared_ptr)


def _member_or_base_member(value, name):
    try:
        return value[name]
    except Exception:  # noqa: BLE001 - debugger backends use different exceptions.
        return value.base()[name]


def qdump__std__map(d, value):
    try:
        node = _member_or_base_member(value, "first_")
        size = int(_member_or_base_member(value, "size_"))
    except Exception:  # noqa: BLE001 - debugger backends use different exceptions.
        stdtypes.qdump__std__map(d, value)
        return

    d.check(0 <= size <= 100 * 1000 * 1000)
    d.putItemCount(size)
    if d.isExpanded():
        with Children(d, size, maxNumChild=1000):
            for index in d.childRange():
                d.checkPointer(node.pointer())
                item = node.dereference()
                d.putPairItem(index, item["value"])
                node = item["next"]


def qdump__std__unordered_map(d, value):
    try:
        buckets = value["buckets_"]
        bucket_count = int(value["nbuckets_"])
        size = int(value["size_"])
        node_pointer_type = buckets.dereference().type
    except Exception:  # noqa: BLE001 - debugger backends use different exceptions.
        stdtypes.qdump__std__unordered_map(d, value)
        return

    d.check(0 <= size <= 100 * 1000 * 1000)
    d.check(0 < bucket_count <= 100 * 1000 * 1000)
    d.putItemCount(size)
    if d.isExpanded():
        item_index = 0
        buckets_address = buckets.pointer()
        with Children(d, size, maxNumChild=1000):
            child_limit = len(d.childRange())
            for bucket_index in range(bucket_count):
                node = d.createValueFromAddress(
                    buckets_address + bucket_index * d.ptrSize(),
                    node_pointer_type,
                )
                while node.pointer():
                    d.checkPointer(node.pointer())
                    item = node.dereference()
                    if item_index < child_limit:
                        d.putPairItem(item_index, item["kv"])
                    item_index += 1
                    if item_index >= child_limit:
                        return
                    node = item["next"]
