#include "maxc.h"

size_t ListObject::get_size() {
    return lselem.size();
}

value_t ListObject::get_item(size_t i) {
    return lselem[i];
}

value_t ListObject::set_item(size_t i, value_t item) {
    ;
}
