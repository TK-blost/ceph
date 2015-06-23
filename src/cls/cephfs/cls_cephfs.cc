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


#include <string>
#include <errno.h>
#include <sstream>

#include "objclass/objclass.h"

CLS_VER(1,0)
CLS_NAME(cephfs_size_scan)

cls_handle_t h_class;
cls_method_handle_t h_accumulate_inode_metadata;

/**
 * Value class for the xattr we'll use to accumulate
 * the highest object seen for a given inode
 */
class ObjCeiling {
  public:
    uint64_t id;
    uint64_t size;

    ObjCeiling()
      : id(0), size(0)
    {}

    ObjCeiling(uint64_t id_, uint64_t size_)
      : id(id_), size(size_)
    {}

    bool operator >(ObjCeiling const &rhs) const
    {
      return id > rhs.id;
    }

    void encode(bufferlist &bl) const
    {
      ::encode(id, bl);
      ::encode(size, bl);
    }

    void decode(bufferlist::iterator &p)
    {
      ::decode(id, p);
      ::decode(size, p);
    }
};
WRITE_CLASS_ENCODER(ObjCeiling)

std::ostream &operator<<(std::ostream &out, ObjCeiling &in)
{
  out << "id: " << in.id << " size: " << in.size;
  return out;
}


/**
 * Set a named xattr to a given integer, if and only if the xattr
 * is not already set to a greater integer.
 *
 * If the xattr is missing, or does not encode an integer, then
 * it is set to the input integer.
 *
 * On success, the output buffer is populated with the resulting
 * integer contained in the xattr.  On failure, a nonzero value is
 * returned and the contents of the output buffer are undefined.
 *
 * @param in: encoded xattr name, uint64_t
 * @param out: the resulting value of the named xattr
 */
template <typename A>
static int set_if_greater(cls_method_context_t hctx,
    const std::string &xattr_name, const A input_val)
{
  bufferlist existing_val_bl;

  bool set_val = false;
  int r = cls_cxx_getxattr(hctx, xattr_name.c_str(), &existing_val_bl);
  if (r == -ENOENT || existing_val_bl.length() == 0) {
    set_val = true;
  } else if (r == 0) {
    bufferlist::iterator existing_p = existing_val_bl.begin();
    try {
      A existing_val;
      ::decode(existing_val, existing_p);
      if (!existing_p.end()) {
        // Trailing junk?  Consider it invalid and overwrite
        set_val = true;
      } else {
        // Valid existing value, do comparison
        set_val = input_val > existing_val;
      }
    } catch (const buffer::error &err) {
      // Corrupt or empty existing value, overwrite it
      set_val = true;
    }
  } else {
    return r;
  }

  // Conditionally set the new xattr
  if (set_val) {
    bufferlist set_bl;
    ::encode(input_val, set_bl);
    return cls_cxx_setxattr(hctx, xattr_name.c_str(), &set_bl);
  } else {
    return 0;
  }
}

static int accumulate_inode_metadata(cls_method_context_t hctx,
    bufferlist *in, bufferlist *out)
{
  assert(in != NULL);
  assert(out != NULL);

  int r = 0;

  // Decode `in`
  bufferlist::iterator q = in->begin();
  try {
    ::decode(obj_xattr_name, q);
    ::decode(mtime_xattr_name, q);
    ::decode(input_obj_id, q);
    ::decode(input_obj_size, q);
    ::decode(input_mtime, q);
  } catch (const buffer::error &err) {
    return -EINVAL;
  }

  ObjCeiling ceiling(input_obj_id, input_obj_size);
  r = set_if_greater(hctx, obj_xattr_name, ceiling);
  if (r < 0) {
    return r;
  }

  r = set_if_greater(hctx, mtime_xattr_name, input_mtime);
  if (r < 0) {
    return r;
  }

  r = set_if_greater(hctx, max_size_xattr_name, input_obj_size);
  if (r < 0) {
    return r;
  }

  return 0;
}

/**
 * initialize class
 *
 * We do two things here: we register the new class, and then register
 * all of the class's methods.
 */
void __cls_init()
{
  // this log message, at level 0, will always appear in the ceph-osd
  // log file.
  CLS_LOG(0, "loading cephfs_size_scan");

  cls_register("cephfs", &h_class);
  cls_register_cxx_method(h_class, "accumulate_inode_metadata",
			  CLS_METHOD_WR | CLS_METHOD_RD,
			  accumulate_inode_metadata, &h_accumulate_inode_metadata);
}

