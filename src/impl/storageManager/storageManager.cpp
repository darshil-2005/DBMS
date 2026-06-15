#include <./storageManager.h>
#include <filesystem>

using fs = std::filesystem;


// Page ID: 0 will now be meta data pages.

bool StorageManager::Bootstrap() {

  if (!fs.exists(DATA_DIR)) {
    fs.create_directories(DATA_DIR);
    std::cout << "[INIT] Initialized data directory: " << DATA_DIR << std::endl;
  };

  if (fs::exists(DB_PATH)) {
    fd_database = open(DB_PATH, O_RDWR | O_DIRECT, S_IRUSR | S_IWUSR);
  } else {
    fd_database = open(DB_PATH, O_RDWR | O_CREAT | O_DIRECT, S_IRUSR | S_IWUSR);
    uint16_t off_start = 2;
    pwrite(fd_database, &off_start, sizeof(off_start), 0);
  };

  fd_logs = open(LOG_PATH, O_WRONLY | O_CREAT | O_APPEND | O_DIRECT,
                 S_IRUSR | S_IWUSR);

  if (fd_database == -1 || fd_logs == -1) {
    if (fd_database != -1) {
      close(fd_database);
      fd_database = -1;
    }
    if (fd_logs != -1) {
      close(fd_logs);
      fd_logs = -1;
    }
    return false;
  };

  Byte buffer[PAGE_SIZE];
  Result<bool> read_result = ReadPage(0, buffer);
  if (read_result.arr != ErrType::None) {
    // handle error
  };
  
  new_page_offset_index = reinterpret_cast<uint16_t*>(buffer);
  return true;
};

Result<bool> StorageManager::ReadPage(PageID pid, Byte *buffer) {
  Offset off = pid * PAGE_SIZE;
  ssize_t byte_count = pread(fd_database, buffer, PAGE_SIZE, off);

  if (byte_count == -1) {
    return {.value : false, .err : DiskReadErr::SystemErr};
  };

  if (static_cast<size_t>(byte_count) != PAGE_SIZE) {
    return {.value : false, .err : DiskReadErr::FileCorruption};
  }

  return {.value = true, .err = DiskReadErr::None};
};

Result<bool> StorageManager::PageWrite(PageID pid, const Byte *buffer) {
  Offset off = pid * PAGE_SIZE;

  ssize_t byte_count = pwrite(fd_database, buffer, PAGE_SIZE, off);

  if (byte_count == -1) {
    return {.value = false, .err = DiskWriteErr::SystemError};
  };

  if (static_cast<size_t>(byte_count) != PAGE_SIZE) {
    return {.value = false, .err = DiskWriteErr::DiskFullOrTruncated};
  };

  return {.value = true, .err = DiskWriteErr::None};
};

StorageManager::~StorageManager() {
  std::cout << "[Shutdown] Destructor triggered. Releasing system resources..."
            << std::endl;

  if (fd_database != -1) {
    if (close(fd_database) == 0) {
      std::cout << "[Shutdown] fd_database (FD: " << fd_database
                << ") closed successfully." << std::endl;
    } else {
      std::cerr << "[Shutdown] Warning: Failed to close fd_database cleanly."
                << std::endl;
    }
    fd_database = -1;
  }

  if (fd_logs != -1) {
    if (close(fd_logs) == 0) {
      std::cout << "[Shutdown] fd_logs (FD: " << fd_logs
                << ") closed successfully." << std::endl;
    } else {
      std::cerr << "[Shutdown] Warning: Failed to close fd_logs cleanly."
                << std::endl;
    }
    fd_logs = -1;
  }
;

Result<PageID> AllocateNewPage() {

  Offset offset = new_page_offset_index * PAGE_SIZE;
  int alloc_result = posix_fallocate(fd_database, offset, PAGE_SIZE);

  if (alloc_result != 0) {
    // handle errors
  };

  new_page_offset_index++;
  fd_database = open(DB_PATH, O_RDWR | O_CREAT | O_DIRECT, S_IRUSR | S_IWUSR);
  pwrite(fd_database, &new_page_offset_index, sizeof(off_start), 0);
  return { .value = new_page_offset_index - 1, .err = ErrType::None };  
};


