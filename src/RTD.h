#ifndef RTD_H   
#define RTD_H

class PT100
{
    public:
    PT100();
    void PT100_setup();
    void read_temperature();
    void PT100_error_check();
};


extern PT100 PT100_object;

#endif
