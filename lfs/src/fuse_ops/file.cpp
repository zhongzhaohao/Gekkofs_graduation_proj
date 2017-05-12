//
// Created by evie on 4/6/17.
//

#include "../main.h"
#include "../fuse_ops.h"
#include "../adafs_ops/metadata_ops.h"
#include "../adafs_ops/dentry_ops.h"
#include "../adafs_ops/access.h"
#include "../adafs_ops/io.h"

using namespace std;

/**
 * Get file attributes.
 *
 * If writeback caching is enabled, the kernel may have a
 * better idea of a file's length than the FUSE file system
 * (eg if there has been a write that extended the file size,
 * but that has not yet been passed to the filesystem.n
 *
 * In this case, the st_size value provided by the file system
 * will be ignored.
 *
 * Valid replies:
 *   fuse_reply_attr
 *   fuse_reply_err
 *
 * @param req request handle
 * @param ino the inode number
 * @param fi for future use, currently always NULL
 */
void adafs_ll_getattr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi) {
    ADAFS_DATA->spdlogger()->debug("adafs_ll_getattr() enter: inode {}", ino);

    auto attr = make_shared<struct stat>();
    auto err = get_attr(*attr, ino);
    if (err == 0) {
        // XXX take a look into timeout value later
        fuse_reply_attr(req, attr.get(), 1.0);
    } else {
        fuse_reply_err(req, err);
    }
}

/**
 * Set file attributes
 *
 * In the 'attr' argument only members indicated by the 'to_set'
 * bitmask contain valid values.  Other members contain undefined
 * values.
 *
 * Unless FUSE_CAP_HANDLE_KILLPRIV is disabled, this method is
 * expected to reset the setuid and setgid bits if the file
 * size or owner is being changed.
 *
 * If the setattr was invoked from the ftruncate() system call
 * under Linux kernel versions 2.6.15 or later, the fi->fh will
 * contain the value set by the open method or will be undefined
 * if the open method didn't set any value.  Otherwise (not
 * ftruncate call, or kernel version earlier than 2.6.15) the fi
 * parameter will be NULL.
 *
 * Valid replies:
 *   fuse_reply_attr
 *   fuse_reply_err
 *
 * @param req request handle
 * @param ino the inode number
 * @param attr the attributes
 * @param to_set bit mask of attributes which should be set
 * @param fi file information, or NULL
 */
void adafs_ll_setattr(fuse_req_t req, fuse_ino_t ino, struct stat *attr, int to_set, struct fuse_file_info *fi) {
    ADAFS_DATA->spdlogger()->debug("adafs_ll_setattr() enter: inode {} to_set {}", ino, to_set);
    // TODO to be implemented if required

    // Temporary. Just to know what is already implemented and what is called
    if (to_set & FUSE_SET_ATTR_MODE) {
        ADAFS_DATA->spdlogger()->debug("FUSE_SET_ATTR_MODE");
    }
    if (to_set & FUSE_SET_ATTR_UID) {
        ADAFS_DATA->spdlogger()->debug("FUSE_SET_ATTR_UID");

    }
    if (to_set & FUSE_SET_ATTR_GID) {
        ADAFS_DATA->spdlogger()->debug("FUSE_SET_ATTR_GID");

    }
    if (to_set & FUSE_SET_ATTR_SIZE) {
        ADAFS_DATA->spdlogger()->debug("FUSE_SET_ATTR_SIZE");

    }
    if (to_set & FUSE_SET_ATTR_ATIME) {
        ADAFS_DATA->spdlogger()->debug("FUSE_SET_ATTR_ATIME");

    }
    if (to_set & FUSE_SET_ATTR_ATIME_NOW) {
        ADAFS_DATA->spdlogger()->debug("FUSE_SET_ATTR_ATIME_NOW");

    }
    if (to_set & FUSE_SET_ATTR_MTIME) {
        ADAFS_DATA->spdlogger()->debug("FUSE_SET_ATTR_MTIME");

    }
    if (to_set & FUSE_SET_ATTR_MTIME_NOW) {
        ADAFS_DATA->spdlogger()->debug("FUSE_SET_ATTR_MTIME_NOW"s);

    }
    if (to_set & FUSE_SET_ATTR_CTIME) {
        ADAFS_DATA->spdlogger()->debug("FUSE_SET_ATTR_CTIME"s);

    }

    if (to_set & (FUSE_SET_ATTR_ATIME | FUSE_SET_ATTR_MTIME)) {
        // TODO
    }


    auto buf = make_shared<struct stat>();
    auto err = get_attr(*buf, ino);

    if (err == 0) {
        fuse_reply_attr(req, buf.get(), 1.0);
    } else {
        fuse_reply_err(req, err);
    }
}

/**
	 * Create and open a file
	 *
	 * If the file does not exist, first create it with the specified
	 * mode, and then open it.
	 *
	 * Open flags (with the exception of O_NOCTTY) are available in
	 * fi->flags.
	 *
	 * Filesystem may store an arbitrary file handle (pointer, index,
	 * etc) in fi->fh, and use this in other all other file operations
	 * (read, write, flush, release, fsync).
	 *
	 * There are also some flags (direct_io, keep_cache) which the
	 * filesystem may set in fi, to change the way the file is opened.
	 * See fuse_file_info structure in <fuse_common.h> for more details.
	 *
	 * If this method is not implemented or under Linux kernel
	 * versions earlier than 2.6.15, the mknod() and open() methods
	 * will be called instead.
	 *
	 * If this request is answered with an error code of ENOSYS, the handler
	 * is treated as not implemented (i.e., for this and future requests the
	 * mknod() and open() handlers will be called instead).
	 *
	 * Valid replies:
	 *   fuse_reply_create
	 *   fuse_reply_err
	 *
	 * @param req request handle
	 * @param parent inode number of the parent directory
	 * @param name to create
	 * @param mode file type and mode with which to create the new file
	 * @param fi file information
	 */
void adafs_ll_create(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode, struct fuse_file_info *fi) {
    ADAFS_DATA->spdlogger()->debug("adafs_ll_create() enter: parent_inode {} name {} mode {:o}", parent, name, mode);

    auto fep = make_shared<fuse_entry_param>();

    // XXX check if file exists (how can we omit this? Let's just try to create it and see if it fails)

    // XXX check permissions (omittable)

    // XXX all this below stuff needs to be atomic. reroll if error happens
    auto err = create_node(req, *fep, parent, string(name), mode);

    // XXX create chunk space
    if (err == 0)
        fuse_reply_create(req, fep.get(), fi);
    else
        fuse_reply_err(req, err);
}


/**
 * Create file node
 *
 * Create a regular file, character device, block device, fifo or
 * socket node.
 *
 * Valid replies:
 *   fuse_reply_entry
 *   fuse_reply_err
 *
 * @param req request handle
 * @param parent inode number of the parent directory
 * @param name to create
 * @param mode file type and mode with which to create the new file
 * @param rdev the device number (only valid if created file is a device)
 */
void adafs_ll_mknod(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode, dev_t rdev) {
    ADAFS_DATA->spdlogger()->debug("adafs_ll_mknod() enter: parent_inode {} name {} mode {:o} dev {}", parent, name, mode, rdev);

    auto fep = make_shared<fuse_entry_param>();

    // XXX check if file exists (how can we omit this? Let's just try to create it and see if it fails)

    // XXX check permissions (omittable)

    // XXX all this below stuff needs to be atomic. reroll if error happens
    auto err = create_node(req, *fep, parent, string(name), mode);

    // XXX create chunk space


    // return new dentry
    if(err == 0) {
        fuse_reply_entry(req, fep.get());
    } else {
        fuse_reply_err(req, err);
    }
}

/**
 * Open a file
 *
 * Open flags are available in fi->flags.  Creation (O_CREAT,
 * O_EXCL, O_NOCTTY) and by default also truncation (O_TRUNC)
 * flags will be filtered out. If an application specifies
 * O_TRUNC, fuse first calls truncate() and then open().
 *
 * If filesystem is able to handle O_TRUNC directly, the
 * init() handler should set the `FUSE_CAP_ATOMIC_O_TRUNC` bit
 * in ``conn->want``.
 *
 * Filesystem may store an arbitrary file handle (pointer,
 * index, etc) in fi->fh, and use this in other all other file
 * operations (read, write, flush, release, fsync).
 *
 * Filesystem may also implement stateless file I/O and not store
 * anything in fi->fh.
 *
 * There are also some flags (direct_io, keep_cache) which the
 * filesystem may set in fi, to change the way the file is opened.
 * See fuse_file_info structure in <fuse_common.h> for more details.
 *
 * If this request is answered with an error code of ENOSYS
 * and FUSE_CAP_NO_OPEN_SUPPORT is set in
 * `fuse_conn_info.capable`, this is treated as success and
 * future calls to open will also succeed without being send
 * to the filesystem process.
 *
 * Valid replies:
 *   fuse_reply_open
 *   fuse_reply_err
 *
 * @param req request handle
 * @param ino the inode number
 * @param fi file information
 */
void adafs_ll_open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
    ADAFS_DATA->spdlogger()->debug("adafs_ll_open() enter: inode {}", ino);
    // TODO to be implemented
    // I think this is used for optimizing what'll happen with the file in the future through fi
    fuse_reply_open(req, fi);
//    fuse_reply_err(req, 0);
}

/**
 * Release an open file
 *
 * Release is called when there are no more references to an open
 * file: all file descriptors are closed and all memory mappings
 * are unmapped.
 *
 * For every open call there will be exactly one release call.
 *
 * The filesystem may reply with an error, but error values are
 * not returned to close() or munmap() which triggered the
 * release.
 *
 * fi->fh will contain the value set by the open method, or will
 * be undefined if the open method didn't set any value.
 * fi->flags will contain the same flags as for open.
 *
 * Valid replies:
 *   fuse_reply_err
 *
 * @param req request handle
 * @param ino the inode number
 * @param fi file information
 */
void adafs_ll_release(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
    ADAFS_DATA->spdlogger()->debug("adafs_ll_release() enter: inode {}", ino);
    // TODO to be implemented if required
    fuse_reply_err(req, 0);
}