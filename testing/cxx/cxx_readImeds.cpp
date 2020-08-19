#include "hmdf.h"

int main(){
    Hmdf::HmdfData h("../testing/test_files/Observations_NOAA_8761305.imeds");
    h.read();
    h.station(0)->show();

    return 0;

}

