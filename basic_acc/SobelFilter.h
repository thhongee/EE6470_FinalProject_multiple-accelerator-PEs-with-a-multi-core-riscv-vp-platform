#ifndef SOBEL_FILTER_H_
#define SOBEL_FILTER_H_
#include <cmath>
#include <iomanip>
#include <iostream>
#include <systemc>
using namespace sc_core;
using namespace std;

#include <tlm_utils/simple_target_socket.h>

#include <tlm>

#include "filter_def.h"

struct SobelFilter : public sc_module {
	tlm_utils::simple_target_socket<SobelFilter> tsock;

	sc_fifo<int> in_value;
	sc_fifo<int> out_value;

	SC_HAS_PROCESS(SobelFilter);

	SobelFilter(sc_module_name n) : sc_module(n), tsock("t_skt"), base_offset(0) {
		tsock.register_b_transport(this, &SobelFilter::blocking_transport);
		SC_THREAD(do_filter);
	}

	~SobelFilter() {}

	unsigned int base_offset;

	void do_filter() {
		// Reset the interfaces
		// Also initialize any variables that are part of the module class here
		/*{
		    HLS_DEFINE_PROTOCOL("reset");
		    din.reset();
		    dout.reset();
		    wait();
		}*/

		while (true) {
			input_t in_val[arraySize][arraySize];

			for (unsigned int v = 0; v < arraySize; v++) {
				for (unsigned int u = 0; u < arraySize; u++) {
					in_val[v][u] = in_value.read();  // get the next value
					cout << in_val[v][u] << endl;
					wait();
				}
			}

			for (unsigned int k = 0; k < arraySize; k++) {
				// Pick all vertices as source one by one
				for (unsigned int i = 0; i < arraySize; i++) {
					// Pick all vertices as destination for the
					// above picked source
					for (unsigned int j = 0; j < arraySize; j++) {
						if ((in_val[i][j] > (in_val[i][k] + in_val[k][j])) &&
						    (in_val[i][k] != 99 && in_val[k][j] != 99)) {
							in_val[i][j] = in_val[i][k] + in_val[k][j];
							wait();
						}
					}
				}
			}

			for (unsigned int m = 0; m < arraySize; m++) {
				for (unsigned int n = 0; n < arraySize; n++) {
					out_value.write(in_val[m][n]);  // output the result
					wait();
				}
			}
		}
	}

	void blocking_transport(tlm::tlm_generic_payload &payload, sc_core::sc_time &delay) {
		wait(delay);
		// unsigned char *mask_ptr = payload.get_byte_enable_ptr();
		// auto len = payload.get_data_length();
		tlm::tlm_command cmd = payload.get_command();
		sc_dt::uint64 addr = payload.get_address();
		unsigned char *data_ptr = payload.get_data_ptr();

		addr -= base_offset;

		word buffer;

		switch (cmd) {
			case tlm::TLM_READ_COMMAND:
				// cout << "READ" << endl;
				switch (addr) {
					case SOBEL_FILTER_RESULT_ADDR:
						buffer.uint = out_value.read();
						break;
					default:
						std::cerr << "READ Error! SobelFilter::blocking_transport: address 0x" << std::setfill('0')
						          << std::setw(8) << std::hex << addr << std::dec << " is not valid" << std::endl;
				}
				data_ptr[0] = buffer.uc[0];
				data_ptr[1] = buffer.uc[1];
				data_ptr[2] = buffer.uc[2];
				data_ptr[3] = buffer.uc[3];
				break;
			case tlm::TLM_WRITE_COMMAND:
				// cout << "WRITE" << endl;
				switch (addr) {
					case SOBEL_FILTER_R_ADDR:
						in_value.write(data_ptr[0]);

						break;
					default:
						std::cerr << "WRITE Error! SobelFilter::blocking_transport: address 0x" << std::setfill('0')
						          << std::setw(8) << std::hex << addr << std::dec << " is not valid" << std::endl;
				}
				break;
			case tlm::TLM_IGNORE_COMMAND:
				payload.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
				return;
			default:
				payload.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
				return;
		}
		payload.set_response_status(tlm::TLM_OK_RESPONSE);  // Always OK
	}
};
#endif
