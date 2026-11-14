#include "stm32f446xx.h"
#include "can_types.h"
#include "can.h"

int main(){
    can_config_t abc = {1,0,16000000,500000,875,1,true,true,false,false,false,false,false};

    volatile status status1 = can_init(&abc) ;

    while(1){
        //do nothing
    }
}