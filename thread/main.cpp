#include "thread_pool.hpp"

#include <iostream>

using namespace std;
using namespace cpputils;

int main(int argc, char const *argv[]) {
    int k[] = {1, 2, 3, 4, 5};
    char cs[] = {'1', '2', '3', '4'};
    char css[] = {'a', 'b', 'b', 'f'};
    ThreadPool pool(2);
    auto p = pool.submit([&k]() {
        for (int i = 0; i < 5; i++)
            k[i] += 10;
        this_thread::sleep_for(1s);
    });
    auto p1 = pool.submit([&cs]() {
        for (int i = 0; i < 4; i++)
            cs[i] += 3;
        this_thread::sleep_for(1s);
    });
    auto p2 = pool.submit([&css]() {
        for (int i = 0; i < 4; i++)
            css[i] += 3;
        this_thread::sleep_for(1s);
    });

    p.get();
    for (int i : k)
        cout << i << " ";
    cout << '\n';
    p1.get();
    for (char i : cs)
        cout << i << " ";
    cout << '\n';
    p2.get();
    for (char i : css)
        cout << i << " ";
    cout << endl;

    return 0;
}
