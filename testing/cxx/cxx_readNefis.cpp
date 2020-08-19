#include "hmdf.h"

int main(){
    Hmdf::HmdfData h1("../testing/test_files/trih-test.dat");
    h1.read();

    h1.readNefisValue("ZWL");

    for(auto &s : h1){
        s.show();
        std::cout << std::endl;
    }


    return 0;
}

