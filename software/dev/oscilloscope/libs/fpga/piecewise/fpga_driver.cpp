#include "fpga_driver.h"
#include "common.h"

pwp_fpga::pwp_fpga(){
    ring_buf  = (volatile uint8_t*)  mmap(NULL, DATA_RING_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, DATA_RING_ADDR);
    ring_wptr = (volatile uint32_t*)  mmap(NULL, 4, PROT_READ|PROT_WRITE, MAP_SHARED, fd, DATA_RING_WPTR);
    ring_size = DATA_RING_SIZE;
}

pwp_fpga::~pwp_fpga(){
    munmap((void*)ring_buf, DATA_RING_SIZE);
}

int pwp_fpga::set_coef(int32_t *coef, size_t size){
    for (unsigned long i = 0; i < size/sizeof(coef); i++)
        cfg->coef[i] = coef[i];
    return 0;
}

int pwp_fpga::set_delay(uint16_t *delay, size_t size){
    for (unsigned long i = 0; i < size/sizeof(delay); i++)
        cfg->delay[i] = delay[i];
    return 0;
}

int pwp_fpga::reset(){
    cfg->reset &= ~RESET_DAQ;
    //DMA will deadlock without wait
    //No idea why
    usleep(1000); 
    cfg->reset &= ~RESET_WRITER;
    cfg->reset |= RESET_WRITER;
    cfg->reset |= RESET_DAQ;
    return 0;
}