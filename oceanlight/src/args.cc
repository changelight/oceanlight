#include <iostream>
#include <cxxopts.hpp>
#include <liboceanlight/lol_debug_messenger.hpp>
#include <config.h>
#include "args.hpp"

void oceanlight::args::parse(int argc, char** argv)
{
	try
	{
		cxxopts::Options op("oceanlight", "Chase your star");
		op.add_options()("h,help", "Produce help message");
		op.add_options()("v,version", "Print version information");
		op.add_options()("x,width", "Window width", cxxopts::value<int>());
		op.add_options()("y,height", "Window height", cxxopts::value<int>());
		auto result {op.parse(argc, argv)};

		if (result.count("help"))
		{
			std::cout << op.help();
			exit_flag = true;
			return;
		}

		if (result.count("version"))
		{
			std::cout << PROJECT_NAME " Ver. " PROJECT_VER "\n"
					  << liboceanlight::version_string() << std::endl;
			exit_flag = true;
			return;
		}

		if (result.count("width"))
			width = result["width"].as<int>();

		if (result.count("height"))
			height = result["height"].as<int>();
	}

	catch (std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		throw;
	}

	catch (...)
	{
		std::cerr << "Error: unknown error" << std::endl;
		throw;
	}
}

bool oceanlight::args::should_exit()
{
	return exit_flag;
}
