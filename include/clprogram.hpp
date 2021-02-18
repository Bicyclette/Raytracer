#ifndef CLPROGRAM_HPP
#define CLPROGRAM_HPP

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <memory>
#include <utility>
#include <algorithm>

#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>

class CLProgram
{
	public:

		CLProgram(const std::string & file);
		cl::Kernel & getKernel();
		cl::CommandQueue & getCommandQueue();
		cl::Context & getContext();

	private:

		std::vector<cl::Device> devices;
		cl::Device device;
		cl::Context context;
		cl::CommandQueue queue;
		cl::Program program;
		cl::Kernel kernel;
};

#endif
