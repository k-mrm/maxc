#include "../maxc.h"

size_t ListObject::get_size() {
    return lselem.size();
}

value_t ListObject::get_item(int i) {
    return lselem[i];
}

void ListObject::set_item(std::vector<value_t> init) {
    lselem = init;
}
