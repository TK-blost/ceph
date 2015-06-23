
#include "include/rados/librados.hpp"
#include "mds/mdstypes.h"

#include "cls_cephfs.h"

class AccumulateArgs;

class ClsCephFSClient
{
  public:
  static int accumulate_inode_metadata(
      librados::IoCtx &ctx,
      inodeno_t inode_no,
      const AccumulateArgs &args);
};

