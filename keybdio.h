void k_enable(void);
void k_disable(void);
void k_wait(void);
int k_getScan(void);
void k_putScan(unsigned char);
int k_getStat(void);
void k_putStat(unsigned char);
int k_getKey(void);
int k_init(void);
void (*k_set_putKey(void(*)(void)))(void);
void (*k_set_CtrlAltDel(void(*)(void)))(void);
void (*k_set_pause(void(*)(void)))(void);

union TKeyData {
	struct {
		unsigned char
			charCode,
			scanCode;
		unsigned short
			shifts;
	} codes;
	unsigned long
		keyData;
};

extern union TKeyData key_data;
