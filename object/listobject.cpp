#include "../maxc.h"

size_t ListObject::get_size() {
    return lselem.size();
}

void ListObject::set_item(std::vector<value_t> init) {
    lselem = init;
}
