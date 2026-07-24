#include<iostream>
#include <memory_resource>

int main(){
    int place[1024];
    std::pmr::monotonic_buffer_resource pool(place,sizeof(place));

    std::pmr::vector<int> myspec(&pool);

    myspec.push_back(1);
}