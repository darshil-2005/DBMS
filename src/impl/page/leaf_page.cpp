#include "../../include/page/leaf_page.h"

// need to handle both leaf and overflow page
uint16_t LeafPage::CheckAvailableSpace(Byte* page) {
  
  LeafPageHeader* page_header = reinterpret_cast<LeafPageHeader*>(page);

  Byte* free_space_start = page;
  Byte* free_space_end = page + page_header->free_space_end_offset;
  free_space_start = free_space_start + LEAF_PAGE_HEADER_SIZE;
  free_space_start = free_space_start + (page_header->slot_array_size * SLOT_SIZE);

  uint16_t free_bytes = free_space_end - free_space_start + 1;
  return free_bytes;
};

SlotArrayElement* LeafPage::upper_bound(SlotArrayElement* start, SlotArrayElement* end, Byte* page, Key x) {

  int16_t n = end - start;
  int16_t l = 0;
  int16_t r = n - 1;
  int16_t ans = n;

  while (l <= r) {
    uint16_t mid = l + (r - l) / 2;
    SlotArrayElement* element = start + mid;
    Byte* tuple = page + element->offset;
    Key* key = reinterpret_cast<Key*>(tuple + TUPLE_HEADER_SIZE); 
    if (*key <= x) {
      l = mid + 1;
    } else {
      ans = mid;
      r = mid - 1;
    };
  };

  return start + ans;
};

Key LeafPage::HandleSplit(Byte* old_page, Byte* new_page, PageID new_pid) {

  Byte buffer[PAGE_SIZE];
  memcpy(buffer, old_page, PAGE_SIZE);

  uint16_t threshold = (PAGE_SIZE - LEAF_PAGE_HEADER_SIZE) / 2;
  uint16_t consumed_space = 0;

  LeafPageHeader* page_header = reinterpret_cast<LeafPageHeader*>(buffer);
  SlotArrayElement* slot_array_start = reinterpret_cast<SlotArrayElement*>(buffer + LEAF_PAGE_HEADER_SIZE);
  SlotArrayElement* slot_array_new_page_start = reinterpret_cast<SlotArrayElement*>(buffer + LEAF_PAGE_HEADER_SIZE);

  for (int i=0; i < page_header->slot_array_size; i++) {
    SlotArrayElement* current = slot_array_start + i;
    consumed_space = consumed_space + current->length;
    if (consumed_space > threshold) {
      slot_array_new_page_start = current + 1;      
      break;
    };
  };

  LeafPage::MakePage(old_page, slot_array_start, 
      (uint16_t)(slot_array_new_page_start - slot_array_start),
      buffer, page_header->page_id, page_header->left_pid, new_pid);

  LeafPage::MakePage(new_page, slot_array_new_page_start, 
      (uint16_t)((slot_array_start + page_header->slot_array_size) - slot_array_new_page_start),
      buffer, new_pid, page_header->page_id, page_header->right_pid);

  Key boundary_key = LeafPage::GetKeyFromSlotElement(buffer, slot_array_new_page_start);
  return boundary_key;
};

bool LeafPage::MakePage(Byte* page, SlotArrayElement* slot_array_start, uint16_t slot_array_size, Byte* buffer, PageID pid, PageID left_pid, PageID right_pid) {

  PageOffset free_space_end_offset = PAGE_SIZE - 1;
  LeafPageHeader* page_header = reinterpret_cast<LeafPageHeader*>(page);

  page_header->slot_array_size = slot_array_size;
  page_header->page_type = PageType::LeafPage;
  page_header->page_id = pid;
  page_header->left_pid = left_pid;
  page_header->right_pid = right_pid;

  if (slot_array_size == 0) {
    page_header->free_space_end_offset = PAGE_SIZE - 1;
    return true;
  };

  SlotArrayElement* slot_array = reinterpret_cast<SlotArrayElement*>(page + LEAF_PAGE_HEADER_SIZE);

  for (int i=0; i<slot_array_size; i++) {
    PageOffset offset = slot_array_start[i].offset;
    TupleLength length = slot_array_start[i].length;

    memcpy(page + free_space_end_offset - length + 1, buffer + offset, length); 
    slot_array[i].offset = free_space_end_offset - length + 1;
    slot_array[i].length = length;
    free_space_end_offset = slot_array[i].offset;
  };

  page_header->free_space_end_offset = free_space_end_offset;
  
  return true;
};

SlotArrayElement* LeafPage::lower_bound(SlotArrayElement* start, SlotArrayElement* end, Byte* page, Key x) {

  int16_t n = end - start;
  int16_t l = 0;
  int16_t r = n - 1;
  int16_t ans = n;

  while (l <= r) {
    uint16_t mid = l + (r - l) / 2;
    SlotArrayElement* element = start + mid;
    Byte* tuple = page + element->offset;
    Key* key = reinterpret_cast<Key*>(tuple + TUPLE_HEADER_SIZE); 
    if (*key < x) {
      l = mid + 1;
    } else {
      ans = mid;
      r = mid - 1;
    };
  };

  return start + ans;
};

Key LeafPage::GetKeyFromSlotElement(Byte* page, SlotArrayElement* element) {
  uint16_t key;
  Byte* key_address = page + element->offset + TUPLE_HEADER_SIZE;
  memcpy(&key, key_address, sizeof(uint16_t));
  return key;
};

uint32_t LeafPage::GetTupleSizeFromSlotElement(Byte* page, SlotArrayElement* element) {
  TupleHeader* tuple_header = reinterpret_cast<TupleHeader*>(page + element->offset);
  return tuple_header->size;
};

SearchResult LeafPage::Search(Byte* page, Key key) {
  
  LeafPageHeader* page_header = reinterpret_cast<LeafPageHeader*>(page);
  SlotArrayElement* slot_array_start = reinterpret_cast<SlotArrayElement*>(page + LEAF_PAGE_HEADER_SIZE);
  SlotArrayElement* slot_array_end = reinterpret_cast<SlotArrayElement*>(slot_array_start + page_header->slot_array_size);
  SlotArrayElement* search_result = LeafPage::lower_bound(slot_array_start, slot_array_end, page, key);

  if (search_result == slot_array_end) {
    return { .found = false, .tuple_offset = 0, .tuple_end_offset = 0, .total_tuple_size = 0, .overflow = false, .overflow_page_id = 0 };
  };

  Key returned_key = LeafPage::GetKeyFromSlotElement(page, search_result);
  TupleHeader* tuple_header = reinterpret_cast<TupleHeader*>(page + search_result->offset);
  size_t tuple_size = tuple_header->size;
  bool overflow = (tuple_header->overflow > 0) ? true : false;
  PageID overflow_page_id = tuple_header->overflow_page;
  

  if (returned_key == key) {
    return { 
      .found = true,
      .tuple_offset = search_result->offset,
      .tuple_end_offset = (Offset)(search_result->offset + search_result->length),
      .total_tuple_size = tuple_size,
      .overflow = overflow, 
      .overflow_page_id = overflow_page_id };
  };

  return { .found = false, .tuple_offset = 0, .tuple_end_offset = 0, .total_tuple_size = 0, .overflow = false, .overflow_page_id = 0 };
};

PayloadStream::PayloadStream() {
  buffer_pool = nullptr;
  curr_pid = 0;
  curr_page_offset = 0;
  curr_page_end = 0;
  total_bytes = 0;
  bytes_read = 0;
  overflow = false;
  overflow_page_id = 0;
};

PayloadStream::PayloadStream(BufferPool *bf, PageID leaf_pid, Offset tuple_offset, Offset tuple_end_offset, size_t total_tuple_size, bool overflow, PageID overflow_page_id) {
  buffer_pool = bf;
  curr_pid = leaf_pid;
  curr_page_offset = tuple_offset + TUPLE_HEADER_SIZE;
  curr_page_end = tuple_end_offset;
  total_bytes = total_tuple_size;
  bytes_read = 0;
  this->overflow = overflow;
  this->overflow_page_id = overflow_page_id;
};

Offset PayloadStream::ReadPage(Byte* page, Byte* buffer, size_t n, Offset start_offset, Offset max_offset) {

  size_t possible_reads = max_offset - start_offset + 1;
  if (n <= possible_reads) {
    memcpy(buffer, page + start_offset, n);
    return start_offset + n;
  } else {
    memcpy(buffer, page + start_offset, possible_reads);
    return start_offset + possible_reads;
  };
};

size_t PayloadStream::NextBytes(Byte* buffer, size_t n) {
  
  size_t total_possible_read_size = total_bytes - bytes_read;
  size_t bytes_to_read = std::min(n, total_possible_read_size);

  if (total_possible_read_size == 0) return 0;

  Result<Byte*> curr_page_fetch = buffer_pool->RequestPage(curr_pid);
  Byte* curr_page = curr_page_fetch.value;

  size_t total_read_this_call = 0;

  while (bytes_to_read > 0) {

    // Returns the offset after the byte it read last.
    Offset result = PayloadStream::ReadPage(curr_page, buffer + total_read_this_call, bytes_to_read, curr_page_offset, curr_page_end - 1);

    size_t bytes_read_from_page = result - curr_page_offset; 
    bytes_read += bytes_read_from_page; 
    bytes_to_read = bytes_to_read - bytes_read_from_page;
    total_read_this_call += bytes_read_from_page;

    if (result == curr_page_end) {
      if (overflow) {
        buffer_pool->ReleasePage(curr_pid, false);
        curr_pid = overflow_page_id;
        curr_page_offset = OVERFLOW_PAGE_HEADER_SIZE;
        curr_page_end = PAGE_SIZE;
        curr_page_fetch = buffer_pool->RequestPage(curr_pid);
        curr_page = curr_page_fetch.value;
        OverflowPageHeader* curr_page_header = reinterpret_cast<OverflowPageHeader*>(curr_page); 
        overflow = curr_page_header->overflow > 0 ? true : false;
        overflow_page_id = curr_page_header->overflow_page;
      } else {
        break;
      };
    } else {
      curr_page_offset = result;
      break;
    }
  };
  
  buffer_pool->ReleasePage(curr_pid, false);
  return total_read_this_call;
};
