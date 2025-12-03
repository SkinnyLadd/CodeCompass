// DUMMY TEST CODE

#include "Sorters.h"
#include "Resource.h"
#include <vector>
#include <iostream>

int main() {
    std::vector<int> empty;
    std::vector<Resource*> resources;

    // Add disordered resources
    resources.push_back(new Resource(1, "Hard", "u", "t", 90, 5.0, empty));
    resources.push_back(new Resource(2, "Easy", "u", "t", 10, 5.0, empty));

    std::cout << "Before Sort: " << resources[0]->difficulty << ", " << resources[1]->difficulty << "\n";

    // Sort
    Sorters::quickSort(resources, 0, resources.size()-1);

    std::cout << "After Sort: " << resources[0]->difficulty << ", " << resources[1]->difficulty << "\n";
    // Expected: 10, 90

    return 0;
}
