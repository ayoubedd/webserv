#pragma once

#include "libhttp/Request.hpp"
#include <fstream>
#include <string>
#include <vector>

namespace libhttp {

  struct MultipartEntity {
    MultipartEntity();
    ~MultipartEntity();

    // Part type.
    enum PartType {
      UNKNOWN,
      FILE,
      OTHER,
    };

    PartType            type;
    libhttp::HeadersMap headers;
    std::string         filePath;

    void clean();
  };

  struct MultipartFormData {
    MultipartFormData(void);

    // Reader states
    enum Status {
      READY,           // Ready to consume new multipart/form-data
      BEFORE_DEL,      // didn't match del yet
      READING_HEADERS, // Reading headers of a multipart/form-data part
      READING_BODY,    // Reading body of a multipart/form-data part
      DONE,            // Done reading and parsing the whole multipart/form-data
    };

    // Reader errors
    enum Error {
      OK,                               // No error accured
      ERROR_WRITTING_TO_FS,             // Error writting to file system
      ERROR_CREATING_FILE,              // Error writting to file system
      PART_MISSING_CONTENT_DISPOSITION, // Part missing Content-Disposition header
      MALFORMED_MUTLIPART,              // Malformed multipart/form-data request
      TMP_DIR_ACCESS_ERROR,             // Error accesssing tmp directory
      CANNOT_EXTRACT_BOUNRAY,           // Error extracting boundary from headers
      RERUN,                            // Telling the caller to re run the reader in place
    };

    typedef std::pair<Error, Status> ErrorStatePair;

    Status                       status;
    MultipartEntity              entity;
    std::vector<MultipartEntity> entities;
    std::string                  del;
    std::string                  afterBodyDel;
    std::string                  closeDel;
    std::string                  commonDel;
    std::fstream                 file;
    std::vector<char>::size_type searchedBytes;

    // Closes all open files for the current session
    // and removes from the file system.
    void cleanup(libhttp::MultipartFormData::Status newStatus);

    // Reads from the Request body
    // and continues from it where it was last time
    ErrorStatePair read(libhttp::Request &req, const std::string &uploadRoot);
  };

  std::string generateFileName(const std::string &prefix);
} // namespace libhttp
