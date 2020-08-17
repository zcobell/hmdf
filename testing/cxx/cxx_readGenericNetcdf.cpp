#include "hmdf.h"

int main(){
    Hmdf h("../testing/test_files/florence_noaa_usace_waterlevel.nc");
    h.read();
    h.station(1)->show();

    return 0;

}

