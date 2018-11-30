// Small example of how to use Clara
//
// Save this snippet as `clara-sample.cpp` together with the simgle-header
// verion of Clara and call `g++` as shown below.
//
// g++ -Wall -std=c++11 -o clara-sample clara-sample.cpp

#include <string>
#include <iostream>
#include "clara.hpp"

// dont show the help by default. Use `-h or `-?` to enable it.
//
bool showHelp = false;

// this block of variables are changed by Clara with the corresponding options passed
// to the program.
//
int width = 0;
std::string name;
bool doIt = false;
std::string command;

// This is a special value which is set indirectly by an lamba function!
//
int index = 0;


int main ( int argc, char** argv )
{
    auto cli = clara::detail::Help(showHelp)
             | clara::detail::Opt( width, "width" )["-w"]["--width"]("How wide should it be?")
             | clara::detail::Opt( name, "name" )["-n"]["--name"]("By what name should I be known")
             | clara::detail::Opt( doIt )["-d"]["--doit"]("Do the thing" )
             | clara::detail::Opt( [&]( int i )
                  {
                    if (i < 0 || i > 10)
                        return clara::detail::ParserResult::runtimeError("index must be between 0 and 10");
                    else {
                        index = i;
                        return clara::detail::ParserResult::ok( clara::detail::ParseResultType::Matched );
                    }
                  }, "index" )
                  ["-i"]( "An index, which is an integer between 0 and 10, inclusive" )
             | clara::detail::Arg( command, "command" )("which command to run").required();
    auto result = cli.parse( clara::detail::Args( argc, argv ) );
    if( !result )
    {
	std::cerr << "Error in command line: " << result.errorMessage() << std::endl;
	return 1;
    }

    if ( showHelp )
    {
	std::cerr << cli << std::endl;
        return 0;
    }

    // show the results!
    std::cerr << "Show Help:" << showHelp << std::endl;
    std::cerr << "Index:" << index << std::endl;
    std::cerr << "Width:" << width << std::endl;
    std::cerr << "Name:" << name << std::endl;
    std::cerr << "Doit:" << doIt << std::endl;
    std::cerr << "Command:" << command << std::endl;

    return 0;
}
