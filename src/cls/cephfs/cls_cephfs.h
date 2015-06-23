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

#include "include/encoding.h"

class AccumulateArgs
{
protected:
  uint64_t obj_id;
  uint64_t obj_size;
  time_t mtime;
  std::string obj_xattr_name;
  std::string mtime_xattr_name;
  std::string obj_size_xattr_name;

public:
  AccumulateArgs(
      uint64_t obj_id_,
      uint64_t obj_size_,
      time_t mtime_,
      std::string obj_xattr_name_,
      std::string mtime_xattr_name_,
      std::string obj_size_xattr_name_)
   : obj_id(obj_id_),
     obj_size(obj_size_),
     mtime(mtime_),
     obj_xattr_name(obj_xattr_name_),
     mtime_xattr_name(mtime_xattr_name_),
     obj_size_xattr_name(obj_size_xattr_name_)
  {}
      
  void encode(bufferlist &bl) const
  {
    ENCODE_START(1, 1, bl);
    ::encode(obj_xattr_name, bl);
    ::encode(mtime_xattr_name, bl);
    ::encode(obj_size_xattr_name, bl);
    ::encode(obj_id, bl);
    ::encode(obj_size, bl);
    ::encode(mtime, bl);
    ENCODE_FINISH(bl);
  }

  void decode(bufferlist::iterator &bl)
  {
    DECODE_START(1, bl);
    ::decode(obj_xattr_name, bl);
    ::decode(mtime_xattr_name, bl);
    ::decode(obj_size_xattr_name, bl);
    ::decode(obj_id, bl);
    ::decode(obj_size, bl);
    ::decode(mtime, bl);
    DECODE_FINISH(bl);
  }
};

