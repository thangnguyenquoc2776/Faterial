#pragma once
#include <cstdio>
#define CG_LOGF(path, fmt, ...) \
  do { FILE* f=nullptr; fopen_s(&f, (path), "a"); if(f){std::fprintf(f, fmt "\n", ##__VA_ARGS__); std::fclose(f);} } while(0)
