#include "hmdf.h"

int main(){
    Hmdf h1("../testing/test_files/fort.61.nc");
    Hmdf h2("../testing/test_files/fort.72.nc");
    h1.read();
    h2.read();
    h2.station(1)->show();
    h1.station(1)->show();

    return 0;
}
