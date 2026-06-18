#include "./utils/catch.hpp"
#include "../src/include/bufferpool/bufferpool.h"
#include "../src/include/storageManager/storageManager.h"
#include "../src/include/b-tree/b-tree.h"
#include <string>

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

TEST_CASE("gdsfbfds", "[vsfvbf]") {

  fs::remove_all(DATA_DIR);

  StorageManager sm;
  REQUIRE(sm.Bootstrap() == true);
  BufferPool bp(sm);
  BTree bt(bp, 0);

  std::string s = R"(Lorem ipsum dolor sit amet, consectetur adipiscing elit. Integer feugiat ultrices sem, sed tincidunt augue hendrerit at. Vestibulum ante ipsum primisin faucibus orci luctus et ultrices posuere cubilia curae; Donec vulputate
ornare eros, non porttitor nisi ullamcorper vitae. Sed volutpat, lectus quis
congue sollicitudin, ligula neque tristique nisl, quis efficitur turpis lorem
nec nisi. Suspendisse potenti. Curabitur faucibus magna sit amet metus
imperdiet, non volutpat lacus pulvinar. Mauris dignissim sapien at purus
tristique, eu consequat risus fermentum. Pellentesque habitant morbi tristique
senectus et netus et malesuada fames ac turpis egestas.
ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789
The quick brown fox jumps over the lazy dog.
Pack my box with five dozen liquor jugs.
Sphinx of black quartz, judge my vow.
How vexingly quick daft zebras jump.
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd
eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee
01234567890123456789012345678901234567890123456789012345678901234567890123456789
98765432109876543210987654321098765432109876543210987654321098765432109876543210
This is a long string intended for testing serialization, deserialization,
buffer writes, page layouts, tuple storage, variable length records, and
general data stream parsing functionality. The goal is to ensure that copying
data into and out of byte buffers works correctly even when the payload spans
hundreds or thousands of bytes and contains a mix of whitespace, punctuation,
letters, and numbers.)";

// std::string s = "!!DARshil!!";

std::cout << "String Length Before: " << s.size() << std::endl;

  Byte buffer1[968];
  Byte buffer2[256];
  Byte buffer3[360];
  Byte buffer4[2000];
  Byte buffer5[750];
  Byte buffer6[128];

  memset(buffer1, 'D', 968);
  memset(buffer2, 'E', 256);
  memset(buffer3, 'F', 360);
  memset(buffer4, 'G', 2000);
  memset(buffer5, 'H', 750);
  memset(buffer6, 'I', 128);
  memcpy(buffer4 + sizeof(Key), s.data(), s.size());

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

  bt.InsertTuple(buffer1, 968, b1);
  bt.InsertTuple(buffer2, 256, b2);
  bt.InsertTuple(buffer3, 360, b3);
  bt.InsertTuple(buffer4, 2 + s.size(), b4);
  bt.InsertTuple(buffer5, 750, b5);
  bt.InsertTuple(buffer6, 128, b6);

  PayloadStream sr = bt.Search(bt.root_page_id, 25);
  Byte response[2000];

  std::cout << sr.NextBytes(response, 2000) << std::endl;

  Key recovered_key;
  memcpy(&recovered_key, response, sizeof(Key));

  std::cout << "Recovered key: " << recovered_key << std::endl;

  std::string ret(reinterpret_cast<char*>(response + sizeof(Key)), s.size());

  std::cout << "RETRIEVED STRING LENGTH: " << ret.size() << std::endl;
  std::cout << "RETRIEVED STRING: " << ret << std::endl;
  std::cout << "T: " << sr.total_bytes << std::endl;
  std::cout << "T: " << sr.bytes_read << std::endl;

};

/*
TEST_CASE("gdsfbfrgqwerds", "[vsfvbf]") {

  StorageManager sm;
  REQUIRE(sm.Bootstrap() == true);
  BufferPool bp(sm);
  BTree bt(bp, 2);

  SearchResult sr = bt.Search(bt.root_page_id, 69);

  Byte response[100];
  memcpy(response, sr.ptr, 100);

  TupleHeader* th = reinterpret_cast<TupleHeader*>(response);
  std::cout << "Overflow: " << (th->overflow > 0 ? 1 : 0) << std::endl;
  std::cout << "Overflow Page ID: " << th->overflow_page << std::endl;
  std::cout << "Size: " << th->size << std::endl;

  uint16_t kk;
  memcpy(&kk, response + TUPLE_HEADER_SIZE, sizeof(uint16_t));
  std::cout << "Retrived Key: " << kk << std::endl;
  std::string ret(reinterpret_cast<char*>(response + TUPLE_HEADER_SIZE + sizeof(Key)), 14);
  std::cout << "Retrived String: " << ret << std::endl;
};


TEST_CASE("WriteChunkLeaf writes the data inside the page properly and returns the number of bytes written.", "[b-tree]") {
  
  Byte data[500];
  Key k = 5;
  memset(data, 'B', 500);
  memcpy(data, &k, sizeof(Key));

  Byte page[PAGE_SIZE];

  














}

*/
