#include "orion/Orion.h"
//#include "utils/Utilities.h"

using namespace std;

int main(int, char**) {
    bool usehdd = false;
    Orion orion(usehdd, 10);
    orion.insert("test1", 1);
    orion.insert("test1", 2);
    orion.insert("test1", 3);
    orion.insert("test1", 35);
    orion.insert("test1", 277);
    cout << orion.search("test1").size() << endl;
    //orion.remove("test1", 1);
    //cout << orion.search("test1")[0] << endl;
    return 0;
}

