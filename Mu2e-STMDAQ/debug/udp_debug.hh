#ifndef UDP_DEBUG_HH
#define UDP_DEBUG_HH

#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <cstring>
#include <iostream>

namespace udp_debug {

// Pretty-print a single msghdr + its iovecs
inline void dump_msghdr(const msghdr& hdr, unsigned idx)
{
    std::cerr << "---- msghdr[" << idx << "] ----\n";

    std::cerr << "  msg_name       = " << hdr.msg_name
              << "  msg_namelen    = " << hdr.msg_namelen << "\n";

    std::cerr << "  msg_iov        = " << hdr.msg_iov
              << "  msg_iovlen     = " << hdr.msg_iovlen << "\n";

    for (size_t j = 0; j < hdr.msg_iovlen; ++j) {
        const iovec& v = hdr.msg_iov[j];
        std::cerr << "    iov[" << j << "].iov_base = " << v.iov_base
                  << "  iov[" << j << "].iov_len = " << v.iov_len << "\n";
    }

    std::cerr << "  msg_control    = " << hdr.msg_control
              << "  msg_controllen = " << hdr.msg_controllen << "\n";

    std::cerr << "  msg_flags      = " << hdr.msg_flags << "\n";
}


// Main debugging wrapper for recvmmsg
inline int debug_recvmmsg(int sockfd,
                          struct mmsghdr* msgvec,
                          unsigned int vlen,
                          int flags,
                          struct timespec* timeout)
{
    errno = 0;
    int n = recvmmsg(sockfd, msgvec, vlen, flags, timeout);

    if (n != -1 || errno != EFAULT) {
        // Success or some error other than EFAULT → no detailed debugging needed
        return n;
    }

    // Report the initial failure
    char ebuf[256];
    strerror_r(errno, ebuf, sizeof(ebuf));
    std::cerr << "\n*** recvmmsg FAILED with EFAULT (Bad address) ***\n";
    std::cerr << "Initial recvmmsg failed: " << ebuf << "\n\n";

    //
    // 1. Test timeout pointer
    //
    if (timeout != nullptr) {
        errno = 0;
        int n2 = recvmmsg(sockfd, msgvec, vlen, flags, nullptr);
        if (n2 != -1 || errno != EFAULT) {
            std::cerr << "EFAULT disappears when timeout=NULL.\n";
            std::cerr << "=> LIKELY CAUSE: invalid or corrupted `timespec* timeout` pointer.\n\n";
            return (n2 == -1 ? -1 : n2);
        }
        std::cerr << "EFAULT persists after removing timeout → timeout OK.\n\n";
    }

    //
    // 2. Test each msgvec entry individually
    //
    std::cerr << "Testing each mmsghdr entry individually...\n";

    for (unsigned i = 0; i < vlen; ++i) {
        struct mmsghdr tmp{};
        tmp.msg_hdr = msgvec[i].msg_hdr;
        tmp.msg_len = 0;

        errno = 0;
        int n2 = recvmmsg(sockfd, &tmp, 1, flags, nullptr);

        if (n2 == -1 && errno == EFAULT) {
            std::cerr << "\n>>>> EFAULT reproduced with msgvec[" << i << "] only <<<<\n";
            std::cerr << "This entry most likely contains the invalid pointer or size.\n\n";

            dump_msghdr(tmp.msg_hdr, i);
            std::cerr << "\n*** End of diagnosis ***\n";
            return -1;
        }
    }

    //
    // 3. If no individual entry caused EFAULT, something else is corrupted
    //
    std::cerr << "\nCould NOT isolate EFAULT to a single entry.\n";
    std::cerr << "Possible causes:\n";
    std::cerr << "  - msgvec pointer itself invalid\n";
    std::cerr << "  - memory corruption\n";
    std::cerr << "  - overwritten mmsghdr array\n";
    std::cerr << "  - stack corruption or misuse of structs\n";
    std::cerr << "*** End of diagnosis ***\n";

    return -1;
}

} // namespace recvmmsg_debug

#endif // UDP_DEBUG_HPP
