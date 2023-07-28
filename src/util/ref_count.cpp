#include "boltdb/util/ref_count.hpp"

using namespace boltdb;

RefCount::RefCount() : _count(new int(1)) {}

RefCount::RefCount(RefCount& other);
RefCount::RefCount& operator=(RefCount& other);
RefCount::RefCount(RefCount&& other);
RefCount::RefCount&& operator=(RefCount&& other);

void RefCount::copy(RefCount& other) {
	_count = other._count;

	if (_count) {
		*_count++;
	}
}

void RefCount::move(RefCount& other) {

}
