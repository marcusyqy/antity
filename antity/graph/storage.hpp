#ifndef _ANTITY_GRAPH_STORAGE_HPP_
#define _ANTITY_GRAPH_STORAGE_HPP_

#include <stdint.h>
#include <stdlib.h>

namespace ant {

struct StorageHeader {
  size_t size;
  size_t alignment;
};

template<typename T>
class Storage {
};

}

#endif // _ANTITY_GRAPH_STORAGE_HPP_
