#include <iostream>
#include <exception>
#include <config.h>
#include <liboceanlight/lol_window.hpp>
#include <liboceanlight/lol_engine_init.hpp>
#include <liboceanlight/lol_engine.hpp>
#include <liboceanlight/lol_engine_shutdown.hpp>
#include "args.hpp"

int main(int argc, char** argv)
{
	try
	{
		oceanlight::args args;
		args.parse(argc, argv);

		if (args.should_exit())
		{
			return EXIT_SUCCESS;
		}

		liboceanlight::window window(args.width, args.height);
		liboceanlight::engine::engine_data engine_data;
		liboceanlight::engine::start(window, engine_data);
		liboceanlight::engine::shutdown(engine_data);
	}

	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	catch (...)
	{
		std::cerr << "Error: unknown error" << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
