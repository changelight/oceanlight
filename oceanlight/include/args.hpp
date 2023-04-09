#ifndef OCEANLIGHT_ARGS_HPP_INCLUDED
#define OCEANLIGHT_ARGS_HPP_INCLUDED
#include <iostream>
namespace oceanlight
{
	class args
	{
		bool exit_flag;

		public:
		int width, height;
		args() : width(640), height(480), exit_flag(false) {};
		void parse(int, char**);
		bool should_exit();
	};
}
#endif /* OCEANLIGHT_ARGS_HPP_INCLUDED */
