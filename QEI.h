void QEI_set_PPR(int ppr_val);//sets the pulses/rev
int QEI_get_RPM(void);//returns rpm
void QEI_Init(void (*CB_on_timer_ellapse) (void), void (*CB_on_dirn_change) (void));//initialises the QEI peripherals
void QEI_set_VelTimer(uint32_t tim_ms);//sets the timer reload value
void QEI_set_timer_callback(void (*CB_on_timer_ellapse) (void));//callback everytime timer reload
void QEI_set_dirn_callback(void (*CB_on_dirn_change) (void));//callback everytime the direction changes
void QEI_set_config(int config_val);//sets the config register
void QEI_select_interrupts(int int_val);//selects interrupts
void QEI_disable(void);//disable QEI
