#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include "umaptest.h"

using namespace std;

void umt_openandmap(
    const umt_optstruct_t& testops,
    uint64_t numbytes, 
    int &fd,
    void*& region)
{
  int open_options = O_RDWR;

  if (testops.iodirect) 
    open_options |= O_DIRECT;

  if ( !testops.noinit )
    open_options |= O_CREAT;

#ifdef O_LARGEFILE
    open_options |= O_LARGEFILE;
#endif

  fd = open(testops.fn, open_options, S_IRUSR|S_IWUSR);
  if(fd == -1) {
    perror("open");
    exit(-1);
  }

  if (testops.noinit) {
    // If we are not initializing file, make sure that it is big enough
    struct stat sbuf;

    if (fstat(fd, &sbuf) == -1) {
      perror("fstat");
      exit(-1);
    }

    if ((uint64_t)sbuf.st_size < numbytes) {
      cerr << testops.fn 
        << " file is not large enough.  "  << sbuf.st_size 
        << " < size requested " << numbytes << endl;
      exit(-1);
    }
  }

  if(posix_fallocate(fd,0, numbytes) != 0) {
    perror("posix_fallocate");
    exit(-1);
  }

  int prot = PROT_READ|PROT_WRITE;
  int flags;
  int my_fd;

  if ( !testops.usemmap ) {
    flags = MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE;
    my_fd = -1;
  }
  else {
    my_fd = fd;
    flags = MAP_SHARED;
  }

  // allocate a memory region to be managed by userfaultfd
  region = mmap(NULL, numbytes, prot, flags, my_fd, 0);

  if (region == MAP_FAILED) {
    perror("mmap");
    exit(-1);
  }
}

