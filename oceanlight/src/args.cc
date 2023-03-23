#include <iostream>
#include <cxxopts.hpp>
#include <liboceanlight/util.hpp>
#include <config.h>
#include "args.hpp"

void oceanlight::parse_args(int argc,
							char** argv,
							struct oceanlight::args& args)
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
			args.exit_flag = true;
			return;
		}

		if (result.count("version"))
		{
			std::cout << PROJECT_NAME " Ver. " PROJECT_VER "\n"
					  << liboceanlight::version_string() << std::endl;
			args.exit_flag = true;
			return;
		}

		if (result.count("width"))
			args.width = result["width"].as<int>();

		if (result.count("height"))
			args.height = result["height"].as<int>();
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
