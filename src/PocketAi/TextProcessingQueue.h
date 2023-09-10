#pragma once

#include <string>
#include <tbb/concurrent_queue.h>

static tbb::concurrent_queue<std::string> requestQueue;
static tbb::concurrent_queue<std::string> responseQueue;
