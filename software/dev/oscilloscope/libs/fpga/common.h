#pragma once
#define CONFIG_ADDR 0x40000000
#define CLOCK_FREQ 125000000
#include <stdio.h>
#include <algorithm>
#include <cmath>

template <class fpga_cfg, class sample>
class fpga {

    public:
    int fd;
    volatile uint32_t *slcr, *hp0;
    volatile fpga_cfg* cfg;
    volatile uint8_t* ring_buf;
    volatile uint32_t* ring_wptr; 
    int32_t ring_size;
    fpga();
    ~fpga();

    //Obtain n raw samples starting now
    void capture_n_raw(sample* data, unsigned int n);

    void capture_n_raw(sample* data, unsigned int n, unsigned int rate);
    
    //Obtain raw samples for t seconds
    void capture_t_raw(sample* data, float t);

    //Obtain n events starting now
    void capture_n_events(sample* data, unsigned int n);

    //Obtain events for t seconds
    void capture_t_events(sample* data, float t);

    int reset();
};

template <class fpga_cfg, class sample>
fpga<fpga_cfg,sample>::fpga(){
    if((fd = open("/dev/mem", O_RDWR)) < 0){
        perror("open");
        exit(EXIT_FAILURE);
    }
    slcr = (volatile uint32_t*) mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0xF8000000);
    hp0  = (volatile uint32_t*) mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0xF8008000);
    cfg  = (volatile fpga_cfg*) mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, fd, CONFIG_ADDR);
    slcr[2] = 0xDF0D;
    slcr[144] = 0;
    hp0[0] &= ~1;
    hp0[5] &= ~1;
}

template <class fpga_cfg, class sample>
fpga<fpga_cfg,sample>::~fpga(){
    munmap((void*)slcr, sysconf(_SC_PAGESIZE));
    munmap((void*)hp0, sysconf(_SC_PAGESIZE));
    munmap((void*)cfg, sysconf(_SC_PAGESIZE));
    close(fd);
}

//Note: This segfaults, check copy bounds
template <class fpga_cfg, class sample>
void fpga<fpga_cfg,sample>::capture_n_raw(sample* data, unsigned int n){
    if ((n * sizeof(sample)) > ring_size) return;
    uint8_t* dest = (uint8_t*)data;
    volatile uint8_t* end = *ring_wptr / 8 * 8 + ring_buf;
    volatile uint8_t* start = end - (n * sizeof(sample));
    volatile uint8_t* ring_end = ring_buf + ring_size;
    // printf("Ring_start:%x\nRing_end:%x\nStart:%x\nEnd:%x\nOffset:%x\n",ring_buf, ring_end, start, end, *ring_wptr);
    if (start < ring_buf){
        start += ring_size;
        std::copy(start, ring_end, dest);
        std::copy(ring_buf, end, dest + (start - ring_end));
    }
    else{
        std::copy(start, end, dest);
    }

    // for (int i = 0; i < n; i++){
    //     data[i].ch_a = i;
    // }

}
//Implement averaging
template <class fpga_cfg, class sample>
void fpga<fpga_cfg,sample>::capture_n_raw(sample* data, unsigned int n, unsigned rate){
    double period = CLOCK_FREQ / (double)rate;
    if ((n * sizeof(sample)) > ring_size) return;
    double end = *ring_wptr;
    double start = ceil(end - n * period);
    int index = 0;
    int ring_samples = ring_size / sizeof(sample);
    // printf("Start: %f\n", start);
    // printf("End: %f\n", end);
    // printf("Total: %d\n", ring_samples);
    if(start < 0){
        // printf("A\n");
        start += ring_samples;
        double i;
        for (i = start; i < ring_samples; i += period)
            data[index++] = ((sample*)ring_buf)[(int)i];
        // printf("Index: %d\n", index);
        // printf("i = %f\n", i);
        for (i = i - ring_samples; i < end; i += period){
            // if(i > (end - period))
            //     printf("i = %f\n", i);
            data[index++] = ((sample*)ring_buf)[(int)i];
    }
            

    }
    else{
        // printf("B\n");
        for (float i = start; i < end; i+= period)
            data[index++] = ((sample*)ring_buf)[(int)i];
    }
    // printf("Index: %d\n", index);
    // static int test = 0;
    // for (int i = 0; i < n; i+= 1)
    //     data[i].ch_a = test;
    // test++;
    

}


template <class fpga_cfg, class sample>
void fpga<fpga_cfg,sample>::capture_t_raw(sample* data, float t){
    fpga::capture_n_raw(data, t * CLOCK_FREQ);
}

//TODO: Implement this
template <class fpga_cfg, class sample>
void fpga<fpga_cfg,sample>::capture_n_events(sample* data, unsigned int n){
}

template <class fpga_cfg, class sample>
void fpga<fpga_cfg,sample>::capture_t_events(sample* data, float t){
    fpga::capture_n_events(data, t * CLOCK_FREQ);
}
