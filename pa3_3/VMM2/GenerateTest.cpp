#include <iostream>
#include <cstdlib>

using std::cin;
using std::cout;
using std::endl;

int generate(int cases, int base, int size) ;

int main(int argc, char* argv[]) 
{
    int chunk = 65536/4;
    generate(200, 0, chunk); // 1
    generate(200, chunk, chunk);  // 2
    generate(200, 0, chunk);  // 1
    generate(200, 2*chunk, chunk); // 3
    generate(200, 0, chunk);  // 1
    generate(200, 3*chunk, chunk);  // 4
    
}

int generate(int cases, int base, int size) 
{
    for (int i=0; i<cases; i++) {
        cout << (rand() % size)+base << endl;
    }
    return 0;
}
