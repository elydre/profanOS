/*****************************************************************************\
|   === errno.c : 2024 ===                                                    |
|                                                                             |
|    Implementation of strerror function from libC                 .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <errno.h>

errno_t errno = 0;

char *strerror(int errnum) {
    switch (errnum) {
        case EOK:       return "Success";
        case EPERM:     return "Not super-user";
        case ENOENT:    return "No such file or directory";
        case ESRCH:     return "No such process";
        case EINTR:     return "Interrupted system call";
        case EIO:       return "I/O error";
        case ENXIO:     return "No such device or address";
        case E2BIG:     return "Arg list too long";
        case ENOEXEC:   return "Exec format error";
        case EBADF:     return "Bad file number";
        case ECHILD:    return "No children";
        case EAGAIN:    return "No more processes";
        case ENOMEM:    return "Not enough core";
        case EACCES:    return "Permission denied";
        case EFAULT:    return "Bad address";
        case ENOTBLK:   return "Block device required";
        case EBUSY:     return "Mount device busy";
        case EEXIST:    return "File exists";
        case EXDEV:     return "Cross-device link";
        case ENODEV:    return "No such device";
        case ENOTDIR:   return "Not a directory";
        case EISDIR:    return "Is a directory";
        case EINVAL:    return "Invalid argument";
        case ENFILE:    return "Too many open files in system";
        case EMFILE:    return "Too many open files";
        case ENOTTY:    return "Not a typewriter";
        case ETXTBSY:   return "Text file busy";
        case EFBIG:     return "File too large";
        case ENOSPC:    return "No space left on device";
        case ESPIPE:    return "Illegal seek";
        case EROFS:     return "Read only file system";
        case EMLINK:    return "Too many links";
        case EPIPE:     return "Broken pipe";
        case EDOM:      return "Math arg out of domain of func";
        case ERANGE:    return "Math result not representable";
        case ENOMSG:    return "No message of desired type";
        case EIDRM:     return "Identifier removed";
        case ECHRNG:    return "Channel number out of range";
        case EL2NSYNC:  return "Level 2 not synchronized";
        case EL3HLT:    return "Level 3 halted";
        case EL3RST:    return "Level 3 reset";
        case ELNRNG:    return "Link number out of range";
        case EUNATCH:   return "Protocol driver not attached";
        case ENOCSI:    return "No CSI structure available";
        case EL2HLT:    return "Level 2 halted";
        case EDEADLK:   return "Deadlock condition";
        case ENOLCK:    return "No record locks available";
        case EBADE:     return "Invalid exchange";
        case EBADR:     return "Invalid request descriptor";
        case EXFULL:    return "Exchange full";
        case ENOANO:    return "No anode";
        case EBADRQC:   return "Invalid request code";
        case EBADSLT:   return "Invalid slot";
        case EDEADLOCK: return "File locking deadlock error";
        case EBFONT:    return "Bad font file fmt";
        case ENOSTR:    return "Device not a stream";
        case ENODATA:   return "No data (for no delay io)";
        case ETIME:     return "Timer expired";
        case ENOSR:     return "Out of streams resources";
        case ENONET:    return "Machine is not on the network";
        case ENOPKG:    return "Package not installed";
        case EREMOTE:   return "The object is remote";
        case ENOLINK:   return "The link has been severed";
        case EADV:      return "Advertise error";
        case ESRMNT:    return "Srmount error";
        case ECOMM:     return "Communication error on send";
        case EPROTO:    return "Protocol error";
        case EMULTIHOP: return "Multihop attempted";
        case ELBIN:     return "Inode is remote (not really error)";
        case EDOTDOT:   return "Cross mount point (not really error)";
        case EBADMSG:   return "Trying to read unreadable message";
        case EFTYPE:    return "Inappropriate file type or format";
        case ENOTUNIQ:  return "Given log. name not unique";
        case EBADFD:    return "f.d. invalid for this operation";
        case EREMCHG:   return "Remote address changed";
        case ELIBACC:   return "Can't access a needed shared lib";
        case ELIBBAD:   return "Accessing a corrupted shared lib";
        case ELIBSCN:   return "lib section in a.out corrupted";
        case ELIBMAX:   return "Attempting to link in too many libs";
        case ELIBEXEC:  return "Attempting to exec a shared library";
        case ENOSYS:    return "Function not implemented";
        case ENMFILE:   return "No more files";
        case ENOTEMPTY:     return "Directory not empty";
        case ENAMETOOLONG:  return "File or path name too long";
        case ELOOP:         return "Too many symbolic links";
        case EOPNOTSUPP:    return "Operation not supported on transport endpoint";
        case EPFNOSUPPORT:  return "Protocol family not supported";
        case ECONNRESET:    return "Connection reset by peer";
        case ENOBUFS:       return "No buffer space available";
        case EAFNOSUPPORT:  return "Address family not supported by protocol family";
        case EPROTOTYPE:    return "Protocol wrong type for socket";
        case ENOTSOCK:      return "Socket operation on non-socket";
        case ENOPROTOOPT:   return "Protocol not available";
        case ESHUTDOWN:     return "Can't send after socket shutdown";
        case ECONNREFUSED:  return "Connection refused";
        case EADDRINUSE:    return "Address already in use";
        case ECONNABORTED:  return "Connection aborted";
        case ENETUNREACH:   return "Network is unreachable";
        case ENETDOWN:      return "Network interface is not configured";
        case ETIMEDOUT:     return "Connection timed out";
        case EHOSTDOWN:     return "Host is down";
        case EHOSTUNREACH:  return "Host is unreachable";
        case EINPROGRESS:   return "Connection already in progress";
        case EALREADY:      return "Socket already connected";
        case EDESTADDRREQ:  return "Destination address required";
        case EMSGSIZE:      return "Message too long";
        case EPROTONOSUPPORT:   return "Unknown protocol";
        case ESOCKTNOSUPPORT:   return "Socket type not supported";
        case EADDRNOTAVAIL:     return "Address not available";
        case ENETRESET:     return "Network dropped connection on reset";
        case EISCONN:       return "Socket is already connected";
        case ENOTCONN:      return "Socket is not connected";
        case ETOOMANYREFS:  return "Too many references: can't splice";
        case EPROCLIM:      return "Too many processes";
        case EUSERS:        return "Too many users";
        case EDQUOT:        return "Disc quota exceeded";
        case ESTALE:        return "Stale NFS file handle";
        case ENOTSUP:       return "Not supported";
        case ENOMEDIUM:     return "No medium (in tape drive)";
        case ENOSHARE:      return "No such host or network path";
        case ECASECLASH:    return "Filename exists with different case";
        case EILSEQ:        return "Illegal byte sequence";
        case EOVERFLOW:     return "Value too large for defined data type";
        default: return "Unknown error";
    }
    return "Unknown error";
}
