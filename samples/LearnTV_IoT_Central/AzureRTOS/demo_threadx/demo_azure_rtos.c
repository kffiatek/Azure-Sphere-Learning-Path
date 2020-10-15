#include "hw/azure_sphere_learning_path.h"
#include "i2c.h"
#include "lsm6dso_driver.h"
#include "lsm6dso_reg.h"
#include "mt3620-intercore.h"
#include "os_hal_gpio.h"
#include "os_hal_uart.h"
#include "printf.h"
#include "tx_api.h"
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include "intercore_contract.h"

#define DEMO_STACK_SIZE 1024
#define DEMO_BYTE_POOL_SIZE 9120
#define DEMO_BLOCK_POOL_SIZE 100
#define DEMO_QUEUE_SIZE 100

// 1 tick = 10ms. It is configurable.
#define MS_TO_TICK(ms)  ((ms) * (TX_TIMER_TICKS_PER_SECOND) / 1000)

// resources for inter core messaging
static uint8_t buf[256];
static uint32_t dataSize;
static BufferHeader* outbound, * inbound;
static uint32_t sharedBufSize = 0;
static const size_t payloadStart = 20;

LP_INTER_CORE_BLOCK ic_control_block;

bool highLevelReady = false;

// Define the ThreadX object control blocks...
TX_THREAD tx_thread_inter_core;
TX_THREAD tx_thread_read_sensor;
TX_EVENT_FLAGS_GROUP event_flags_0;
TX_BYTE_POOL byte_pool_0;
TX_BLOCK_POOL block_pool_0;
UCHAR memory_area[DEMO_BYTE_POOL_SIZE];

// Define thread prototypes.
void thread_inter_core(ULONG thread_input);
void thread_read_sensor(ULONG thread_input);


int main()
{
	tx_kernel_enter(); // Enter the Azure RTOS kernel.
}

// Define what the initial system looks like.
void tx_application_define(void* first_unused_memory)
{
	CHAR* pointer;

	tx_byte_pool_create(&byte_pool_0, "byte pool 0", memory_area, DEMO_BYTE_POOL_SIZE); // Create a byte memory pool from which to allocate the thread stacks

	tx_byte_allocate(&byte_pool_0, (VOID**)&pointer, DEMO_STACK_SIZE, TX_NO_WAIT);	   // Allocate the stack for inter core thread
	tx_thread_create(&tx_thread_inter_core, "thread inter core", thread_inter_core, 0, // Create inter core message thread
		pointer, DEMO_STACK_SIZE, 4, 4, TX_NO_TIME_SLICE, TX_AUTO_START);

	tx_byte_allocate(&byte_pool_0, (VOID**)&pointer, DEMO_STACK_SIZE, TX_NO_WAIT);		  // Allocate the stack for read sensor thread
	tx_thread_create(&tx_thread_read_sensor, "thread read sensor", thread_read_sensor, 0, // Create read sensor thread */
		pointer, DEMO_STACK_SIZE, 4, 4, TX_NO_TIME_SLICE, TX_AUTO_START);

	tx_event_flags_create(&event_flags_0, "event flags 0"); // Create event flag for thread sync
}

// https://embeddedartistry.com/blog/2017/02/17/implementing-malloc-with-threadx/
// overrides for malloc and free required for srand and rand
void* malloc(size_t size)
{
	void* ptr = NULL;

	if (size > 0)
	{
		// We simply wrap the threadX call into a standard form
		uint8_t r = tx_byte_allocate(&byte_pool_0, &ptr, size,
			TX_WAIT_FOREVER);

		if (r != TX_SUCCESS)
		{
			ptr = NULL;
		}
	}
	//else NULL if there was no size

	return ptr;
}

void free(void* ptr)
{
	if (ptr)
	{
		//We simply wrap the threadX call into a standard form
		tx_byte_release(ptr);
	}
}

void send_inter_core_msg(void)
{
	if (highLevelReady)
	{
		memcpy((void*)&buf[payloadStart], (void*)&ic_control_block, sizeof(ic_control_block));
		dataSize = payloadStart + sizeof(ic_control_block);

		EnqueueData(inbound, outbound, sharedBufSize, buf, dataSize);
	}
}

// This thread monitors inter core messages.
void thread_inter_core(ULONG thread_input)
{
	UINT status;

	if (GetIntercoreBuffers(&outbound, &inbound, &sharedBufSize) == -1)
	{
		return; // kill the thread
	}

	while (1)
	{
		dataSize = sizeof(buf);
		int r = DequeueData(outbound, inbound, sharedBufSize, buf, &dataSize);

		if (r == 0 && dataSize > payloadStart)
		{
			highLevelReady = true;

			memcpy((void*)&ic_control_block, (void*)&buf[payloadStart], sizeof(ic_control_block));

			switch (ic_control_block.cmd)
			{
			case LP_IC_HEARTBEAT:
				break;
			case LP_IC_TEMPERATURE_PRESSURE_HUMIDITY:
				// Set event flag 0 to wakeup threads read sensor and blink led
				status = tx_event_flags_set(&event_flags_0, 0x1, TX_OR);

				if (status != TX_SUCCESS)
					break;
				break;
			default:
				break;
			}
		}
		tx_thread_sleep(MS_TO_TICK(500));
	}
}

void thread_read_sensor(ULONG thread_input)
{
	UINT status;
	ULONG actual_flags;
	int rand_number;

	mtk_os_hal_i2c_ctrl_init(i2c_port_num); // Initialize MT3620 I2C bus
	i2c_enum();								// Enumerate I2C Bus

	if (i2c_init())
	{
		return;
	}

	// LSM6DSO Init - the accelerometer calibration has been commented out for faster start up
	if (lsm6dso_init(i2c_write, i2c_read))
	{
		return;
	}

	srand((unsigned int)time(NULL)); // seed the random number generator for fake telemetry

	while (true)
	{
		// waits here until flag set in inter core thread
		status = tx_event_flags_get(&event_flags_0, 0x1, TX_OR_CLEAR, &actual_flags, TX_WAIT_FOREVER); // wait until event set in the thread_inter_core thread

		if ((status != TX_SUCCESS) || (actual_flags != 0x1))
			break;

		if (highLevelReady)
		{
			ic_control_block.cmd = LP_IC_TEMPERATURE_PRESSURE_HUMIDITY;
			ic_control_block.temperature = get_temperature();

			rand_number = (rand() % 20) - 10;
			ic_control_block.humidity = (float)(50.0 + rand_number);

			rand_number = (rand() % 50) - 25;
			ic_control_block.pressure = (float)(1000.0 + rand_number);

			send_inter_core_msg();
		}
	}
}