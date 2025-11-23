#include <gccore.h>
#include <ogc/lwp.h>
#include <stdio.h>

#include "bluetooth.h"
#include "hci.h"
#include "btmemb.h"
#include "physbusif.h"
#include "l2cap.h"

struct bt_state
{
	err_t last_err;

	syswd_t timer_svc;
	lwpq_t hci_cmdq;
	u8_t hci_cmddone;
	u8_t hci_inited;

	btecallback cb;
	void *usrdata;
};

static struct bt_state btstate;

static void bt_reset_all()
{
	btstate.hci_inited = 0;
	btstate.hci_cmddone = 0;
	btstate.last_err = ERR_OK;
}

static inline s32 __bt_waitcmdfinish(struct bt_state *state)
{
	u32 level;
	s32 ret;

	if(!state) return ERR_VAL;

	_CPU_ISR_Disable(level);
	while(!state->hci_cmddone)
		LWP_ThreadSleep(state->hci_cmdq);
	ret = state->last_err;
	_CPU_ISR_Restore(level);

	return ret;
}

static inline s32 __bt_cmdfinish(struct bt_state *state,err_t err)
{
	u32 level;

	if(!state) return ERR_VAL;

	_CPU_ISR_Disable(level);
	state->last_err = err;
	state->hci_cmddone = 1;
	if(state->cb!=NULL)
	{
		btecallback cb = state->cb;
		state->cb = NULL;
		cb(err,state->usrdata);
	}
	else
	{
		LWP_ThreadSignal(state->hci_cmdq);
	}
	_CPU_ISR_Restore(level);

	return err;
}

static err_t __bt_shutdown_finished(void *arg,struct hci_pcb *pcb,u8_t ogf,u8_t ocf,u8_t result)
{
	err_t err;
	struct bt_state *state = (struct bt_state*)arg;
	
	if(state==NULL) return ERR_OK;

	state->hci_inited = 0;
	hci_cmd_complete(NULL);
	if(result==HCI_SUCCESS)
		err = ERR_OK;
	else
		err = ERR_CONN;

	physbusif_close();
	return __bt_cmdfinish(state,err);
}

void BT_Shutdown(void)
{
	u32 level;

	if(btstate.hci_inited==0) return;

	_CPU_ISR_Disable(level);
	SYS_RemoveAlarm(btstate.timer_svc);
	btstate.cb = NULL;
	btstate.usrdata = NULL;
	btstate.hci_cmddone = 0;
	hci_arg(&btstate);
	hci_cmd_complete(__bt_shutdown_finished);
	hci_reset();
	__bt_waitcmdfinish(&btstate);
	LWP_CloseQueue(btstate.hci_cmdq);
	_CPU_ISR_Restore(level);

	physbusif_shutdown();
}

err_t bt_hci_initcore_complete(void *arg,struct hci_pcb *pcb,u8_t ogf,u8_t ocf,u8_t result)
{
	err_t err = ERR_OK;
	u8_t dev_cod[] = {0x04, 0x25, 0x00};
	struct bt_state *state = (struct bt_state*)arg;

	switch(ogf) {
		case HCI_INFO_PARAM_OGF:
			if(ocf==HCI_R_BUF_SIZE_OCF) { // 2
				if(result==HCI_SUCCESS) {
					hci_write_cod(dev_cod);
				} else
					err = ERR_CONN;
			} else if(ocf==HCI_R_LOC_VERS_INFO_OCF) { // 7
				if(result==HCI_SUCCESS) {
					hci_read_bd_addr();
				} else
					err = ERR_CONN;
			} else if(ocf==HCI_R_BD_ADDR_OCF) { // 8
				if(result==HCI_SUCCESS) {
					hci_read_local_features();
				} else
					err = ERR_CONN;
			} else if(ocf==HCI_R_LOC_FEAT_OCF) { // 9
				if(result==HCI_SUCCESS) {
					hci_write_inquiry_mode(0x01);
				} else
					err = ERR_CONN;
			}
			break;
		case HCI_HC_BB_OGF:
			if(ocf==HCI_RESET_OCF) { // 1
				if(result==HCI_SUCCESS) {
					hci_read_buffer_size();
				} else 
					err = ERR_CONN;
			} else if(ocf==HCI_W_COD_OCF) { // 3
				if(result==HCI_SUCCESS) {
					hci_write_local_name((u8_t*)"Nintendo RVL-CNT-01-BB",23);
				} else
					err = ERR_CONN;
			} else if(ocf==HCI_W_LOCAL_NAME_OCF) { // 4
				if(result==HCI_SUCCESS) {
					hci_write_pin_type(0x00);
				} else
					err = ERR_CONN;
			} else if(ocf==HCI_W_PIN_TYPE_OCF) { // 5
				if(result==HCI_SUCCESS) {
					hci_write_current_lap(0x009E8B00);
				} else
					err = ERR_CONN;
			} else if(ocf==HCI_W_CUR_IACLAP_OCF) { // 5
				if(result==HCI_SUCCESS) {
					hci_host_buffer_size();
				} else
					err = ERR_CONN;
			} else if(ocf==HCI_HOST_BUF_SIZE_OCF) { // 6
				if(result==HCI_SUCCESS) {
					hci_read_local_version();
				} else
					err = ERR_CONN;
			} else if(ocf==HCI_W_INQUIRY_MODE_OCF) { // 10
				if(result==HCI_SUCCESS) {
					hci_write_page_scan_type(0x01);
				} else
					err = ERR_CONN;
			} else if(ocf==HCI_W_PAGE_SCAN_TYPE_OCF) { // 11
				if(result==HCI_SUCCESS) {
					hci_write_inquiry_scan_type(0x01);
				} else
					err = ERR_CONN;
			} else if(ocf==HCI_W_INQUIRY_SCAN_TYPE_OCF) { // 12
				if(result==HCI_SUCCESS) {
					hci_write_page_timeout(0x8000);
				} else
					err = ERR_CONN;
			} else if(ocf==HCI_W_PAGE_TIMEOUT_OCF) { // 13
				if(result==HCI_SUCCESS) {
					hci_write_scan_enable(0x03);
				} else
					err = ERR_CONN;
			} else if(ocf==HCI_W_SCAN_EN_OCF) { // 14
				if(result==HCI_SUCCESS) {
					state->hci_inited = 1;
					hci_cmd_complete(NULL);
					return __bt_cmdfinish(state,ERR_OK);
				} else
					err = ERR_CONN;
			}
			break;
		default:
			fprintf(stderr, "Error initializing Bluetooth subsystem. OGF = 0x%x OCF = 0x%x\n", ogf, ocf);
			err = ERR_CONN;
			break;
	}

	if(err!=ERR_OK) __bt_cmdfinish(state,err);
	return err;
}

void BT_Init(btecallback cb, struct l2cap_pcb **pcbptr)
{
	u32 level;

	LOG("BT_Init()\n");

	memset(&btstate,0,sizeof(struct bt_state));

	hci_init();
	l2cap_init();
	physbusif_init();

	LWP_InitQueue(&btstate.hci_cmdq);
	SYS_CreateAlarm(&btstate.timer_svc);

	_CPU_ISR_Disable(level);
	bt_reset_all();
	hci_reset_all();
	l2cap_reset_all();
	physbusif_reset_all();
	_CPU_ISR_Restore(level);

	btstate.cb = cb;
	btstate.usrdata = (void *)pcbptr;
	btstate.hci_cmddone = 0;
	hci_arg(&btstate);
	hci_cmd_complete(bt_hci_initcore_complete);
	hci_reset();
}
