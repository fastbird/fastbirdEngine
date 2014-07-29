#include <UnitTest/StdAfx.h>
#include <Engine/DllMain.h>

// Verifies that the command line flag variables can be accessed
// in code once <gtest/gtest.h> has been #included.
// Do not move it after other #includes.
TEST(CommandLineFlagsTest, CanBeAccessedInCodeOnceGTestHIsIncluded) {
  bool dummy = testing::GTEST_FLAG(also_run_disabled_tests)
      || testing::GTEST_FLAG(break_on_failure)
      || testing::GTEST_FLAG(catch_exceptions)
      || testing::GTEST_FLAG(color) != "unknown"
      || testing::GTEST_FLAG(filter) != "unknown"
      || testing::GTEST_FLAG(list_tests)
      || testing::GTEST_FLAG(output) != "unknown"
      || testing::GTEST_FLAG(print_time)
      || testing::GTEST_FLAG(random_seed)
      || testing::GTEST_FLAG(repeat) > 0
      || testing::GTEST_FLAG(show_internal_stack_frames)
      || testing::GTEST_FLAG(shuffle)
      || testing::GTEST_FLAG(stack_trace_depth) > 0
      || testing::GTEST_FLAG(stream_result_to) != "unknown"
      || testing::GTEST_FLAG(throw_on_failure);
  EXPECT_TRUE(dummy || !dummy);  // Suppresses warning that dummy is unused.
}

#include "gtest/gtest-spi.h"

// Indicates that this translation unit is part of Google Test's
// implementation.  It must come before gtest-internal-inl.h is
// included, or there will be a compiler error.  This trick is to
// prevent a user from accidentally including gtest-internal-inl.h in
// his code.

int main(int argc, char** argv) 
{
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
		
	fastbird::IEngine* pEngine = ::Create_fastbird_Engine();
	
    testing::InitGoogleTest(&argc, argv); 
    RUN_ALL_TESTS(); 
    std::getchar(); // keep console window open until Return keystroke
	delete fastbird::gpTimer;

	::Destroy_fastbird_Engine();
}

namespace fastbird
{
void Error(const char* szFmt, ...)
{
	char buf[2048];
	va_list args;
	va_start(args, szFmt);
	vsprintf_s(buf, 2048, szFmt, args);
	va_end(args);
	std::cout << buf << std::endl;
	assert(0);
}

void Log(const char* szFmt, ...)
{
	char buf[2048];
	va_list args;
	va_start(args, szFmt);
	vsprintf_s(buf, 2048, szFmt, args);
	va_end(args);
	std::cout << buf << std::endl;
}
}