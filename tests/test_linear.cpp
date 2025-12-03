// DUMMY TEST CODE

#include "Resource.h"
#include "LRUCache.h"
#include <iostream>
#include <vector>

int main() {
    // 1. Setup Dummy Data
    std::vector<int> empty;
    Resource* r1 = new Resource(1, "A", "url", "Array", 10, 5.0, empty);
    Resource* r2 = new Resource(2, "B", "url", "Array", 10, 5.0, empty);
    Resource* r3 = new Resource(3, "C", "url", "Array", 10, 5.0, empty);

    // 2. Init Cache with Capacity 2
    LRUCache cache(2);

    // 3. Test Put
    std::cout << "Putting ID 1...\n";
    cache.put(1, r1);
    std::cout << "Putting ID 2...\n";
    cache.put(2, r2);

    // 4. Test Get (Should make 1 the most recent)
    std::cout << "Accessing ID 1 (Making it MRU)...\n";
    if(cache.get(1) != nullptr) std::cout << "Hit ID 1\n";

    // 5. Test Eviction (Should remove 2, not 1)
    std::cout << "Putting ID 3 (Should evict 2)...\n";
    cache.put(3, r3);

    if(cache.get(2) == nullptr) std::cout << "SUCCESS: ID 2 was evicted.\n";
    else std::cout << "FAIL: ID 2 is still there.\n";

    return 0;
}
