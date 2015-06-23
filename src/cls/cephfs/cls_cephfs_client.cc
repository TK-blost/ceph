// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab
/*
 * Ceph - scalable distributed file system
 *
 * Copyright (C) 2015 Red Hat
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1, as published by the Free Software
 * Foundation.  See file COPYING.
 *
 */


#include "cls_cephfs_client.h"

#include "mds/CInode.h"

int ClsCephFSClient::accumulate_inode_metadata(
  librados::IoCtx &ctx,
  inodeno_t inode_no,
  const AccumulateArgs &args)
{
    // Generate 0th object name, where we will accumulate sizes/mtimes
    object_t zeroth_object = InodeStore::get_object_name(inode_no, frag_t(), "");

    // Construct a librados operation invoking our class method
    librados::ObjectReadOperation op;
    bufferlist inbl;
    args.encode(inbl);
    op.exec("cephfs", "accumulate_inode_metadata", inbl);

    // Execute op
    bufferlist outbl;
    return ctx.operate(zeroth_object.name, &op, &outbl);
}

