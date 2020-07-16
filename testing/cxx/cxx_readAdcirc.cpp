#include "hmdf.h"

int main(){
    Hmdf h("../testing/test_files/fort.61",Date(2011,01,01,00,00,00),"../testing/test_files/stations.csv");
    h.read();
    h.station(1)->show();

    return 0;

}

