#ifndef OCEANLIGHT_ARGS_HPP_INCLUDED
#define OCEANLIGHT_ARGS_HPP_INCLUDED
namespace oceanlight
{
	class args
	{
		bool exit_flag {false};

		public:
		int width, height;
		args() : width(640), height(480) {};
		void parse(int, char**);
		bool should_exit();
	};
}
#endif /* OCEANLIGHT_ARGS_HPP_INCLUDED */
