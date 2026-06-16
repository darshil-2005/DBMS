#include "./utils/catch.hpp"
#include "../src/include/bufferpool/bufferpool.h"
#include "../src/include/storageManager/storageManager.h"
#include "../src/include/b-tree/b-tree.h"

void DumpPage(Byte* page) {
  LeafPageHeader* page_header = reinterpret_cast<LeafPageHeader*>(page);

  std::cout << "=========================================" << std::endl;
  std::cout << "Page Type: " << static_cast<int>(page_header->page_type) << std::endl;
  std::cout << "Page ID: " << page_header->page_id << std::endl;
  std::cout << "Free Space End Offset: " << page_header->free_space_end_offset << std::endl;
  std::cout << "Slot Array Size: " << page_header->slot_array_size << std::endl;
  std::cout << "Left Sibling: " << page_header->left_pid << std::endl;
  std::cout << "Right Sibling: " << page_header->right_pid << std::endl;
  std::cout << "=========================================" << std::endl;

  SlotArrayElement* slot_array = reinterpret_cast<SlotArrayElement*>(page + LEAF_PAGE_HEADER_SIZE);
  for (int i=0; i<page_header->slot_array_size; i++) {
    OverflowInfo* overflow_info = reinterpret_cast<OverflowInfo*>(page + slot_array[i].offset);
    std::cout << "=========================================" << std::endl;
    std::cout << "Tuple Offset: " << slot_array[i].offset << std::endl;
    std::cout << "Tuple Length: " << slot_array[i].length << std::endl;
    std::cout << "Tuple Overflow: " << static_cast<int>(overflow_info->overflow) << std::endl;
    std::cout << "Overflow Page: " << overflow_info->overflow_page << std::endl;
    std::cout << "Key: " << *reinterpret_cast<uint16_t*>(page + slot_array[i].offset + TUPLE_HEADER_SIZE) << std::endl;
    std::cout << "=========================================" << std::endl;
  };
};

/*
TEST_CASE("WriteChunkLeaf write properly to the leaf", "[b-tree]") {

  fs::remove_all(DATA_DIR);

  StorageManager sm;
  REQUIRE(sm.Bootstrap() == true);
  BufferPool bp(sm);
  BTree bt(bp);
  
  Byte page[4096];
  SlotArrayElement *slots = reinterpret_cast<SlotArrayElement *>(page + LEAF_PAGE_HEADER_SIZE);
  uint8_t page_type = 3;
  uint16_t page_id = 69;
  uint16_t free_space_end_offset = 4095;
  uint16_t slot_array_size = 0;
  PageID prev_pid = 66;
  PageID next_pid = 66;
  memcpy(page, &page_type, 1);
  memcpy(page + 1, &page_id, 2);
  memcpy(page + 3, &free_space_end_offset, 2);
  memcpy(page + 5, &slot_array_size, 2);
  memcpy(page + 7, &prev_pid, 2);
  memcpy(page + 9, &next_pid, 2);

  Byte buffer1[968];
  Byte buffer2[256];
  Byte buffer3[360];
  Byte buffer4[800];
  Byte buffer5[750];
  Byte buffer6[128];

  memset(buffer1, 'D', 968);
  memset(buffer2, 'E', 256);
  memset(buffer3, 'F', 360);
  memset(buffer4, 'G', 800);
  memset(buffer5, 'H', 750);
  memset(buffer6, 'I', 128);

  Key b1 = 10;
  Key b2 = 20;
  Key b3 = 69;
  Key b4 = 25;
  Key b5 = 45;
  Key b6 = 2;

  memcpy(buffer1, &b1, sizeof(Key));
  memcpy(buffer2, &b2, sizeof(Key));
  memcpy(buffer3, &b3, sizeof(Key));
  memcpy(buffer4, &b4, sizeof(Key));
  memcpy(buffer5, &b5, sizeof(Key));
  memcpy(buffer6, &b6, sizeof(Key));

  bt.WriteChunkLeaf(page, buffer1, 968, b1);
  bt.WriteChunkLeaf(page, buffer2, 256, b2);
  bt.WriteChunkLeaf(page, buffer3, 360, b3);
  bt.WriteChunkLeaf(page, buffer4, 800, b4);
  bt.WriteChunkLeaf(page, buffer5, 750, b5);
  bt.WriteChunkLeaf(page, buffer6, 128, b6);

  Result<NewPage> new_page = bp.AllocateNewPage();
  REQUIRE(new_page.err == ErrType::None);

  Key k = LeafPage::HandleSplit(page, new_page.value.ptr, new_page.value.pid);

  DumpPage(page);
  DumpPage(new_page.value.ptr);
};
*/

/*
TEST_CASE("gdsfbfds", "[vsfvbf]") {

  fs::remove_all(DATA_DIR);

  StorageManager sm;
  REQUIRE(sm.Bootstrap() == true);
  BufferPool bp(sm);
  BTree bt(bp, 0);

  for (int i=0; i<10; i++) {
    Byte buffer[500];
    Key key = 69 + i;
    memcpy(buffer, &key, sizeof(Key));
    bt.InsertTuple(buffer, 600 + 5 * (i+1), key);
  };
  
  SearchResult sr = bt.Search(bt.root_page_id, 69 + 3);
  // 120
  // std::cout << sr.size << std::endl;
  // std::cout << bt.root_page_id << std::endl;
};
*/

TEST_CASE("gdsfbfrgqwerds", "[vsfvbf]") {

  StorageManager sm;
  REQUIRE(sm.Bootstrap() == true);
  BufferPool bp(sm);
  BTree bt(bp, 5);

  SearchResult sr = bt.Search(bt.root_page_id, 69 + 3);
  // 120
  std::cout << sr.size << std::endl;
};
