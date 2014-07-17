#ifndef UTIL_FILE_H
#define UTIL_FILE_H

#include "util/exception.hh"
#include "util/string_piece.hh"

#include <cstddef>
#include <cstdio>
#include <string>

#include <stdint.h>

namespace util {

class scoped_fd {
  public:
    scoped_fd() : fd_(-1) {}

    explicit scoped_fd(int fd) : fd_(fd) {}

    ~scoped_fd();

    void reset(int to = -1) {
      scoped_fd other(fd_);
      fd_ = to;
    }

    int get() const { return fd_; }

    int operator*() const { return fd_; }

    int release() {
      int ret = fd_;
      fd_ = -1;
      return ret;
    }

  private:
    int fd_;

    scoped_fd(const scoped_fd &);
    scoped_fd &operator=(const scoped_fd &);
};

class scoped_FILE {
  public:
    explicit scoped_FILE(std::FILE *file = NULL) : file_(file) {}

    ~scoped_FILE();

    std::FILE *get() { return file_; }
    const std::FILE *get() const { return file_; }

    void reset(std::FILE *to = NULL) {
      scoped_FILE other(file_);
      file_ = to;
    }

    std::FILE *release() {
      std::FILE *ret = file_;
      file_ = NULL;
      return ret;
    }

  private:
    std::FILE *file_;
};

/* Thrown for any operation where the fd is known. */
class FDException : public ErrnoException {
  public:
    explicit FDException(int fd) throw();

    virtual ~FDException() throw();

    // This may no longer be valid if the exception was thrown past open.
    int FD() const { return fd_; }

    // Guess from NameFromFD.
    const std::string &NameGuess() const { return name_guess_; }

  private:
    int fd_;

    std::string name_guess_;
};

// End of file reached.
class EndOfFileException : public Exception {
  public:
    EndOfFileException() throw();
    ~EndOfFileException() throw();
};

// Open for read only.  
int OpenReadOrThrow(const char *name);
// Create file if it doesn't exist, truncate if it does.  Opened for write.   
int CreateOrThrow(const char *name);

// Return value for SizeFile when it can't size properly.  
const uint64_t kBadSize = (uint64_t)-1;
uint64_t SizeFile(int fd);
uint64_t SizeOrThrow(int fd);

void ResizeOrThrow(int fd, uint64_t to);

std::size_t PartialRead(int fd, void *to, std::size_t size);
void ReadOrThrow(int fd, void *to, std::size_t size);
std::size_t ReadOrEOF(int fd, void *to_void, std::size_t size);

void WriteOrThrow(int fd, const void *data_void, std::size_t size);
void WriteOrThrow(FILE *to, const void *data, std::size_t size);

/* These call pread/pwrite in a loop.  However, on Windows they call ReadFile/
 * WriteFile which changes the file pointer.  So it's safe to call ErsatzPRead
 * and ErsatzPWrite concurrently (or any combination thereof).  But it changes
 * the file pointer on windows, so it's not safe to call concurrently with
 * anything that uses the implicit file pointer e.g. the Read/Write functions
 * above.
 */
void ErsatzPRead(int fd, void *to, std::size_t size, uint64_t off);
void ErsatzPWrite(int fd, const void *data_void, std::size_t size, uint64_t off);

void FSyncOrThrow(int fd);

// Seeking
void SeekOrThrow(int fd, uint64_t off);
void AdvanceOrThrow(int fd, int64_t off);
void SeekEnd(int fd);

std::FILE *FDOpenOrThrow(scoped_fd &file);
std::FILE *FDOpenReadOrThrow(scoped_fd &file);

// Temporary files
// Append a / if base is a directory.
void NormalizeTempPrefix(std::string &base);
int MakeTemp(const StringPiece &prefix);
std::FILE *FMakeTemp(const StringPiece &prefix);

// dup an fd.
int DupOrThrow(int fd);

/* Attempt get file name from fd.  This won't always work (i.e. on Windows or
 * a pipe).  The file might have been renamed.  It's intended for diagnostics
 * and logging only.
 */
std::string NameFromFD(int fd);

} // namespace util

#endif // UTIL_FILE_H
