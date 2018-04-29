#include <math.h>
#include <stdlib.h>

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "application.h"


void test_pedometer_sensor_id(void **state)
{
    int input_sensor_id = 105;
	int output_sensor_id = 0;
	output_sensor_id = test_get_ped_ssid( );
	assert_int_equal(output_sensor_id, input_sensor_id);
}

void test_humidity_user_reg(void **state)
{
    int input_user_reg_value = 2;
	int output_user_reg_value = 0;
	output_user_reg_value = test_get_hum_usid( );
	assert_int_equal(output_user_reg_value, input_user_reg_value);
}

void test_pedometer_data_read(void **state)
{
    int step_count_ret_val = test_get_ped_data();
    assert_int_equal(step_count_ret_val, 0);    
}

void test_humidity_data_read(void **state)
{
    int hum_sensor_ret_val = test_get_hum_data();
    assert_int_equal(hum_sensor_ret_val, 0);    
}

int main(int argc, char **argv)
{
	/* Initialize for test functionality */
	int status = application_init();
	
	if(status == 0)
	{
		const struct CMUnitTest tests[] = {
			cmocka_unit_test(test_pedometer_sensor_id),
			cmocka_unit_test(test_humidity_user_reg),
			cmocka_unit_test(test_pedometer_data_read),
			cmocka_unit_test(test_humidity_data_read),
		};

		return cmocka_run_group_tests(tests, NULL, NULL);
	}
}
