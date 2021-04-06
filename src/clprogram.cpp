#include "clprogram.hpp"

CLProgram::CLProgram(const std::string & file)
{
	std::fstream stream;
	stream.open(file, std::fstream::in);

	// get code length
	stream.seekg(0, stream.end);
	long length{stream.tellg()};
	stream.seekg(0, stream.beg);

	// create char array
	std::unique_ptr<char[]> code{std::make_unique<char[]>(length+1)};
	code[length] = '\0';
	stream.read(code.get(), length);

	if(!stream)
	{
		std::cerr << "Error while trying to read file (OpenCL code)" << std::endl;
		std::exit(-1);
	}

	// get list of opencl platforms
	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);
	if(platforms.empty())
	{
		std::cerr << "OpenCL platforms not found." << std::endl;
		std::exit(-1);
	}

	// get first available GPU device
	for(auto p{platforms.begin()}; p != platforms.end(); ++p)
	{
		std::vector<cl::Device> platformDevices;
		p->getDevices(CL_DEVICE_TYPE_GPU, &platformDevices);

		for(auto d{platformDevices.begin()}; d != platformDevices.end(); ++d)
		{
			if(!d->getInfo<CL_DEVICE_AVAILABLE>())
				continue;
			else
			{
				devices.push_back(*d);
			}
		}
	}
	device = devices.at(0);
	context = cl::Context(device);

	// print device name
	std::cout << "Found device : " << device.getInfo<CL_DEVICE_NAME>() << std::endl;

	// command queue
	queue = cl::CommandQueue(context, device);

	// compile OpenCL program for found device
	program = cl::Program(context, cl::Program::Sources(1, std::make_pair(code.get(), strlen(code.get()))));

	try
	{
		program.build(devices);
	}
	catch(cl::Error& e)
	{
		std::cerr << "OpenCL compilation error." << std::endl
		<< program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << std::endl;
		std::exit(-1);
	}

	// get kernel
	kernel = cl::Kernel(program, "raytrace");
}

cl::Kernel & CLProgram::getKernel()
{
	return kernel;
}

cl::CommandQueue & CLProgram::getCommandQueue()
{
	return queue;
}

cl::Context & CLProgram::getContext()
{
	return context;
}
