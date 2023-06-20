#include <iostream>
#include <vector>
#include <exception>
#include <config.h>
#include <liboceanlight/lol_engine.hpp>
#include <liboceanlight/lol_window.hpp>
#include <liboceanlight/lol_debug_messenger.hpp>
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

		static liboceanlight::engine engine;
		liboceanlight::window window(args.width, args.height);
		engine.init(window);
		engine.run(window);
	}

	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	catch (...)
	{
		std::cerr << "Error: unknown error" << std::endl;
		throw;
	}

	return EXIT_SUCCESS;
}
