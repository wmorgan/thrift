// Copyright (c) 2006- Facebook
// Distributed under the Thrift Software License
//
// See accompanying file LICENSE or visit the Thrift site at:
// http://developers.facebook.com/thrift/

#include <iostream>
#include <vector>
#include <string>

#include "ThreadFactoryTests.h"
#include "TimerManagerTests.h"
#include "ThreadManagerTests.h"

int main(int argc, char** argv) {

  std::string arg;

  std::vector<std::string>  args(argc - 1 > 1 ? argc - 1 : 1);

  args[0] = "all";

  for (int ix = 1; ix < argc; ix++) {
    args[ix - 1] = std::string(argv[ix]);
  }

  bool runAll = args[0].compare("all") == 0;

  if (runAll || args[0].compare("thread-factory") == 0) {

    ThreadFactoryTests threadFactoryTests;
    
    std::cout << "ThreadFactory tests..." << std::endl;
    
    size_t count =  1000;

    std::cout << "\t\tThreadFactory reap N threads test: N = " << count << std::endl;

    assert(threadFactoryTests.reapNThreads(count));

    std::cout << "\t\tThreadFactory synchronous start test" << std::endl;

    assert(threadFactoryTests.synchStartTest());

    std::cout << "\t\tThreadFactory monitor timeout test" << std::endl;

    assert(threadFactoryTests.monitorTimeoutTest());
  }

  if (runAll || args[0].compare("util") == 0) {

    std::cout << "Util tests..." << std::endl;

    std::cout << "\t\tUtil minimum time" << std::endl;

    long long time00 = Util::currentTime();
    long long time01 = Util::currentTime();

    std::cout << "\t\t\tMinimum time: " << time01 - time00 << "ms" << std::endl;

    time00 = Util::currentTime();
    time01 = time00;
    size_t count = 0;
    
    while (time01 < time00 + 10) {
      count++;
      time01 = Util::currentTime();
    }

    std::cout << "\t\t\tscall per ms: " << count / (time01 - time00) << std::endl;
  }


  if (runAll || args[0].compare("timer-manager") == 0) {

    std::cout << "TimerManager tests..." << std::endl;

    std::cout << "\t\tTimerManager test00" << std::endl;

    TimerManagerTests timerManagerTests;

    assert(timerManagerTests.test00());
  }

  if (runAll || args[0].compare("thread-manager") == 0) {

    std::cout << "ThreadManager tests..." << std::endl;

    {

      size_t workerCount = 100;

      size_t taskCount = 100000;

      long long delay = 10LL;

      std::cout << "\t\tThreadManager load test: worker count: " << workerCount << " task count: " << taskCount << " delay: " << delay << std::endl;

      ThreadManagerTests threadManagerTests;

      assert(threadManagerTests.loadTest(taskCount, delay, workerCount));
    }
  }

  if (runAll || args[0].compare("thread-manager-benchmark") == 0) {

    std::cout << "ThreadManager benchmark tests..." << std::endl;

    {

      size_t minWorkerCount = 2;

      size_t maxWorkerCount = 512;

      size_t tasksPerWorker = 1000;

      long long delay = 10LL;

      for (size_t workerCount = minWorkerCount; workerCount < maxWorkerCount; workerCount*= 2) {

	size_t taskCount = workerCount * tasksPerWorker;

	std::cout << "\t\tThreadManager load test: worker count: " << workerCount << " task count: " << taskCount << " delay: " << delay << std::endl;

	ThreadManagerTests threadManagerTests;

	threadManagerTests.loadTest(taskCount, delay, workerCount);
      }
    }
  }
}
