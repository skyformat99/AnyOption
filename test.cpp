#define CATCH_CONFIG_MAIN
/*
 * To compile get the latest version of catch2 from
 * https://github.com/catchorg/Catch2
 * $ wget https://github.com/catchorg/Catch2/releases/download/v2.2.3/catch.hpp
 * $ g++ -g -std=c++11 -Wall -Wextra -Werror test.cpp anyoption.cpp -o test
 *
 */
#include "anyoption.h"
#include "catch.hpp"

#include <fstream>
#include <stdarg.h>
#include <string>

using namespace std;
using namespace Catch::Matchers;

char **buildArgv(int size, ...) {
  va_list args;
  char **argv = (char **)malloc(size * sizeof(char *));
  va_start(args, size);
  for (int i = 0; i < size; i++) {
    const char *tmp = va_arg(args, char *);
    argv[i] = (char *)malloc((strlen(tmp) + 1) * sizeof(char));
    strcpy(argv[i], tmp);
  }
  va_end(args);
  return argv;
}

void clearArgv(int size, char **_argv) {
  for (int i = 0; i < size; i++)
    free(_argv[i]);
  free(_argv);
}

void writeOptions(string options) {
  std::ofstream out("test.options");
  out << options;
  out.close();
}

TEST_CASE("Test long options") {
  const int argc = 5;
  char **argv =
      buildArgv(argc, "test", "--long_flag_in_args", "--long_option_with_value",
                "long_option_value", "--long_option_with_out_value");
  AnyOption *opt = new AnyOption();

  opt->setFlag("long_flag_in_args");
  opt->setFlag("long_flag_not_in_args");
  opt->setOption("long_option_with_value");
  opt->setOption("long_option_with_out_value");
  opt->setOption("long_option_not_in_args");

  opt->processCommandArgs(argc, argv);

  REQUIRE(opt->getFlag("long_flag_in_args") == true);
  REQUIRE(opt->getFlag("long_flag_not_in_args") == false);
  REQUIRE(opt->getFlag("not_defined") == false);

  REQUIRE_THAT(opt->getValue("long_option_with_value"),
               Equals("long_option_value"));
  REQUIRE(opt->getValue("long_option_with_out_value") == NULL);
  REQUIRE(opt->getValue("long_option_not_in_args") == NULL);
  REQUIRE(opt->getValue("not_defined") == NULL);

  delete opt;
  clearArgv(argc, argv);
}

TEST_CASE("Test short options") {
  const int argc = 5;
  char **argv = buildArgv(argc, "test", "-x", "-a", "a_value", "-b");
  AnyOption *opt = new AnyOption();

  opt->setFlag('x'); // flag set in args
  opt->setFlag('y'); // flag not set in args

  opt->setOption('a'); // option with value in args
  opt->setOption('b'); // option with out value in args
  opt->setOption('c'); // option not in args

  opt->processCommandArgs(argc, argv);

  REQUIRE(opt->getFlag('x') == true);
  REQUIRE(opt->getFlag('y') == false);
  REQUIRE(opt->getFlag('z') == false); // not defined

  REQUIRE_THAT(opt->getValue('a'), Equals("a_value"));
  REQUIRE(opt->getValue('b') == NULL);
  REQUIRE(opt->getValue('c') == NULL);
  REQUIRE(opt->getValue('d') == NULL); // not defined

  delete opt;
  clearArgv(argc, argv);
}

TEST_CASE("Test file options") {
  writeOptions("x\nlong_flag\na : a_value\nlong_option : long_option_value\n");
  AnyOption *opt = new AnyOption();

  opt->useFiileName("test.options");

  opt->setFlag('x');                 // short flag set in file
  opt->setFlag('y');                 // short flag not set in file
  opt->setFlag("long_flag");         // long flag set in file
  opt->setFlag("long_flag_not_set"); // long flag not set in file

  opt->setOption('a');                       // short option with value in file
  opt->setOption('b');                       // short option not in file
  opt->setOption("long_option");             // long option with value in file
  opt->setOption("long_option_not_in_file"); // long option not file

  opt->processFile();

  REQUIRE(opt->getFlag('x') == true);
  REQUIRE(opt->getFlag('y') == false);
  REQUIRE(opt->getFlag('z') == false); // not defined
  REQUIRE(opt->getFlag("long_flag") == true);
  REQUIRE(opt->getFlag("not_defined") == false);

  REQUIRE_THAT(opt->getValue('a'), Equals("a_value"));
  REQUIRE(opt->getValue('b') == NULL);
  REQUIRE(opt->getValue('c') == NULL); // not defined
  REQUIRE_THAT(opt->getValue("long_option"), Equals("long_option_value"));
  REQUIRE(opt->getValue("not_defined") == NULL);

  delete opt;
}
